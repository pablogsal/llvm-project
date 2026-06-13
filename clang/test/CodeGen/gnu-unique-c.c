// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fgnu-unique -emit-llvm -o - %s | FileCheck %s

// GNU unique is a C++ vague-linkage rule. C declarations can still produce
// weak or weak_odr COMDAT globals, but those must not be tagged as GNU unique.

__attribute__((weak)) int c_weak = 1;
__attribute__((selectany)) int c_selectany = 2;

int use(void) { return c_weak + c_selectany; }

// CHECK-DAG: @c_weak = weak global i32 1, align 4{{$}}
// CHECK-DAG: @c_selectany = weak_odr global i32 2, comdat, align 4{{$}}
// CHECK-NOT: !gnu_unique
