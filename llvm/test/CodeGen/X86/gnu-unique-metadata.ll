; RUN: llc -mtriple=x86_64-unknown-linux-gnu -filetype=obj < %s | llvm-readelf --file-header --symbols - | FileCheck %s
; RUN: llc -mtriple=i386-unknown-linux-gnu -filetype=obj < %s | llvm-readelf --file-header --symbols - | FileCheck %s
; RUN: llc -mtriple=x86_64-unknown-linux-gnu -emulated-tls -filetype=obj < %s | llvm-readelf --symbols - | FileCheck %s --check-prefix=EMUTLS

$unique_global = comdat any
$unique_const = comdat any
$unique_tls = comdat any
$weak_global = comdat any

; GCC's libvtv tests contain checked-in assembly with these hidden weak COMDAT
; symbols in .vtable_map_vars emitted as @gnu_unique_object, for both 32-bit
; and 64-bit ELF.
$"_ZN4_VTVI11EnvironmentE12__vtable_mapE" = comdat any
$"_ZN4_VTVI15EnvironmentImplE12__vtable_mapE" = comdat any

@unique_global = weak_odr global i32 1, comdat, align 4, !gnu_unique !0
@unique_const = weak_odr constant i32 2, comdat, align 4, !gnu_unique !0
@unique_tls = weak_odr thread_local global i32 3, comdat, align 4, !gnu_unique !0
@weak_global = weak_odr global i32 4, comdat, align 4
@internal_unique = internal global i32 5, align 4, !gnu_unique !0
@global_unique = global i32 6, align 4, !gnu_unique !0
@weak_no_comdat = weak_odr global i32 7, align 4, !gnu_unique !0
@_ZN4_VTVI11EnvironmentE12__vtable_mapE = weak_odr hidden global ptr null, section ".vtable_map_vars", comdat, align 4, !gnu_unique !0
@_ZN4_VTVI15EnvironmentImplE12__vtable_mapE = weak_odr hidden global ptr null, section ".vtable_map_vars", comdat, align 4, !gnu_unique !0

!0 = !{}

; CHECK: OS/ABI: UNIX - GNU
; CHECK-DAG: OBJECT  UNIQUE DEFAULT {{.*}} unique_global
; CHECK-DAG: OBJECT  UNIQUE DEFAULT {{.*}} unique_const
; CHECK-DAG: TLS     UNIQUE DEFAULT {{.*}} unique_tls
; CHECK-DAG: OBJECT  WEAK   DEFAULT {{.*}} weak_global
; CHECK-DAG: OBJECT  LOCAL  DEFAULT {{.*}} internal_unique
; CHECK-DAG: OBJECT  GLOBAL DEFAULT {{.*}} global_unique
; CHECK-DAG: OBJECT  WEAK   DEFAULT {{.*}} weak_no_comdat
; CHECK-DAG: OBJECT  UNIQUE HIDDEN  {{.*}} _ZN4_VTVI11EnvironmentE12__vtable_mapE
; CHECK-DAG: OBJECT  UNIQUE HIDDEN  {{.*}} _ZN4_VTVI15EnvironmentImplE12__vtable_mapE

; EMUTLS-DAG: OBJECT  UNIQUE DEFAULT {{.*}} __emutls_v.unique_tls
; EMUTLS-DAG: OBJECT  UNIQUE DEFAULT {{.*}} __emutls_t.unique_tls
