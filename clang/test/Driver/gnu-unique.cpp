// RUN: %clang -### -target x86_64-unknown-linux-gnu -fgnu-unique -c %s 2>&1 | FileCheck %s --check-prefix=ENABLE
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fno-gnu-unique -c %s 2>&1 | FileCheck %s --check-prefix=DISABLE
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fgnu-unique -fno-gnu-unique -c %s 2>&1 | FileCheck %s --check-prefix=DISABLE
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fno-gnu-unique -fgnu-unique -c %s 2>&1 | FileCheck %s --check-prefix=ENABLE

// GCC's libphobos shared tests pass -fno-gnu-unique because they rely on
// repeated dlopen/dlclose. Make sure the disabling spelling is accepted and
// the last spelling wins.

// ENABLE: "-cc1"
// ENABLE-SAME: "-fgnu-unique"

// DISABLE-NOT: argument unused
// DISABLE: "-cc1"
// DISABLE-NOT: "-fgnu-unique"
