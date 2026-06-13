#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd -- "$SCRIPT_DIR/../../.." && pwd)

CLANG=${CLANG:-"$REPO_ROOT/build/bin/clang"}
READELF=${READELF:-"$REPO_ROOT/build/bin/llvm-readelf"}
GXX=${GXX:-g++}
TARGET=${TARGET:-x86_64-unknown-linux-gnu}
CXXSTD=${CXXSTD:--std=c++20}

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

failures=0

require_tool() {
  local tool=$1
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "missing tool: $tool" >&2
    exit 2
  fi
}

require_file() {
  local file=$1
  if [[ ! -x "$file" ]]; then
    echo "missing executable: $file" >&2
    exit 2
  fi
}

require_tool "$GXX"
require_file "$CLANG"
require_file "$READELF"

compile_matrix() {
  local src=$1
  local stem=$2
  "$GXX" "$CXXSTD" -c "$src" -o "$TMPDIR/$stem-gcc-gnu.o"
  "$GXX" "$CXXSTD" -fno-gnu-unique -c "$src" -o "$TMPDIR/$stem-gcc-nognu.o"
  "$CLANG" -target "$TARGET" "$CXXSTD" -fgnu-unique -c "$src" \
    -o "$TMPDIR/$stem-clang-gnu.o"
  "$CLANG" -target "$TARGET" "$CXXSTD" -fno-gnu-unique -c "$src" \
    -o "$TMPDIR/$stem-clang-nognu.o"
}

lookup_symbol() {
  local obj=$1
  local sym=$2
  "$READELF" --symbols "$obj" | awk -v sym="$sym" '
    $8 == sym {
      print $4 "/" $5 "/" $6
      found = 1
    }
    END {
      if (!found)
        print "MISSING"
    }'
}

print_osabi() {
  local label=$1
  local obj=$2
  printf '%-18s ' "$label:"
  "$READELF" --file-header "$obj" | awk '/OS\/ABI:/ { sub(/^ +/, ""); print }'
}

compare_symbols() {
  local stem=$1
  local title=$2
  shift 2

  echo
  echo "$title"
  printf '| Case | Symbol | GCC default | Clang -fgnu-unique | Match | GCC -fno | Clang -fno | Match |\n'
  printf '|---|---|---:|---:|:---:|---:|---:|:---:|\n'

  while (($#)); do
    local desc=$1
    local sym=$2
    shift 2

    local gcc_gnu clang_gnu gcc_nognu clang_nognu match_gnu match_nognu
    gcc_gnu=$(lookup_symbol "$TMPDIR/$stem-gcc-gnu.o" "$sym")
    clang_gnu=$(lookup_symbol "$TMPDIR/$stem-clang-gnu.o" "$sym")
    gcc_nognu=$(lookup_symbol "$TMPDIR/$stem-gcc-nognu.o" "$sym")
    clang_nognu=$(lookup_symbol "$TMPDIR/$stem-clang-nognu.o" "$sym")

    match_gnu=NO
    if [[ "$gcc_gnu" == "$clang_gnu" ]]; then
      match_gnu=YES
    else
      ((failures++))
    fi

    match_nognu=NO
    if [[ "$gcc_nognu" == "$clang_nognu" ]]; then
      match_nognu=YES
    else
      ((failures++))
    fi

    printf '| %s | `%s` | `%s` | `%s` | %s | `%s` | `%s` | %s |\n' \
      "$desc" "$sym" "$gcc_gnu" "$clang_gnu" "$match_gnu" \
      "$gcc_nognu" "$clang_nognu" "$match_nognu"
  done
}

compile_vtv_i386_if_available() {
  local src=$1
  local stem=vtv-i386

  if ! "$GXX" "$CXXSTD" -m32 -c "$src" -o "$TMPDIR/$stem-gcc-gnu.o" \
      >/dev/null 2>&1; then
    echo
    echo "i386 libvtv-shaped demo skipped: $GXX -m32 failed"
    return
  fi

  "$GXX" "$CXXSTD" -m32 -fno-gnu-unique -c "$src" \
    -o "$TMPDIR/$stem-gcc-nognu.o"
  "$CLANG" -target i386-unknown-linux-gnu "$CXXSTD" -fgnu-unique -c "$src" \
    -o "$TMPDIR/$stem-clang-gnu.o"
  "$CLANG" -target i386-unknown-linux-gnu "$CXXSTD" -fno-gnu-unique -c "$src" \
    -o "$TMPDIR/$stem-clang-nognu.o"

  compare_symbols "$stem" "libvtv-shaped hidden COMDAT map symbols (i386)" \
    "hidden vtable map static" '_ZN3VTVI11EnvironmentE12__vtable_mapE' \
    "hidden vtable map static" '_ZN3VTVI15EnvironmentImplE12__vtable_mapE'
}

echo "Compiler versions"
"$GXX" --version | head -1
"$CLANG" --version | head -1

compile_matrix "$SCRIPT_DIR/cases.cpp" cases
compile_matrix "$SCRIPT_DIR/vtv-map.cpp" vtv

echo
echo "OSABI"
print_osabi "GCC default" "$TMPDIR/cases-gcc-gnu.o"
print_osabi "Clang enabled" "$TMPDIR/cases-clang-gnu.o"
print_osabi "GCC -fno" "$TMPDIR/cases-gcc-nognu.o"
print_osabi "Clang -fno" "$TMPDIR/cases-clang-nognu.o"

compare_symbols cases "C++ vague-linkage cases" \
  "template static data member" '_ZN6HolderIiE5valueE' \
  "template static const data member" '_ZN6HolderIiE8constantE' \
  "template static TLS data member" '_ZN6HolderIiE3tlsE' \
  "dynamic template static data member" '_ZN13DynamicHolderIiE5valueE' \
  "guard for dynamic template static" '_ZGVN13DynamicHolderIiE5valueE' \
  "inline function local static" '_ZZ12local_staticvE1x' \
  "dynamic inline local static" '_ZZ14dynamic_staticvE1y' \
  "guard for dynamic inline local static" '_ZGVZ14dynamic_staticvE1y' \
  "vtable negative case" '_ZTV9HasVTable' \
  "typeinfo negative case" '_ZTI9HasVTable' \
  "typeinfo name negative case" '_ZTS9HasVTable'

compare_symbols vtv "libvtv-shaped hidden COMDAT map symbols (x86_64)" \
  "hidden vtable map static" '_ZN3VTVI11EnvironmentE12__vtable_mapE' \
  "hidden vtable map static" '_ZN3VTVI15EnvironmentImplE12__vtable_mapE'

compile_vtv_i386_if_available "$SCRIPT_DIR/vtv-map.cpp"

echo
if ((failures)); then
  echo "FAIL: $failures symbol comparison mismatches"
  exit 1
fi

echo "PASS: patched Clang matches GCC symbol Type/Bind/Visibility for all demo cases"
