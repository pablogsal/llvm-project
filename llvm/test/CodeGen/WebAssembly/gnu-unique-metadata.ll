; RUN: llc -mtriple=wasm32-unknown-unknown -filetype=obj < %s -o /dev/null

; The metadata is ELF-specific. Non-ELF targets should ignore it instead of
; forwarding an ELF-only symbol attribute to their streamer.

@x = weak_odr global i32 1, !gnu_unique !0

!0 = !{}
