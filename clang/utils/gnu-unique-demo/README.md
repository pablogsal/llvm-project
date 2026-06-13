# GNU Unique Demo

This directory contains a small smoke/demo harness for the downstream
`-fgnu-unique` Clang patch.

Run it from the LLVM source tree after building Clang and llvm-readelf:

```sh
clang/utils/gnu-unique-demo/compare-gcc-clang.sh
```

By default it uses:

```sh
CLANG=build/bin/clang
READELF=build/bin/llvm-readelf
GXX=g++
TARGET=x86_64-unknown-linux-gnu
```

Override those in the environment if needed:

```sh
CLANG=/path/to/clang READELF=/path/to/llvm-readelf GXX=/path/to/g++ \
  clang/utils/gnu-unique-demo/compare-gcc-clang.sh
```

The script compiles each testcase four ways:

* GCC default
* GCC `-fno-gnu-unique`
* patched Clang default
* patched Clang `-fno-gnu-unique`

It compares the ELF symbol `Type/Bind/Visibility` triples for:

* template static data members
* template static const data members
* template static TLS data members
* dynamically initialized template static data members
* guard variables for dynamically initialized template static data members
* inline-function local statics
* dynamically initialized inline-function local statics
* guard variables for dynamically initialized inline-function local statics
* negative cases that must stay weak: vtables, typeinfo, and typeinfo names
* libvtv-shaped hidden GNU-unique symbols in `.vtable_map_vars`

The script also tries an i386 object-only libvtv-shaped comparison, matching
the 32-bit and 64-bit checked-in GCC libvtv assembly coverage. If the local GCC
does not support `-m32`, the i386 demo is skipped.
