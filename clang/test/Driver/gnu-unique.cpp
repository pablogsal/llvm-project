// RUN: %clang -### -target x86_64-unknown-linux-gnu -c %s 2>&1 | FileCheck %s --check-prefix=DEFAULT
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fgnu-unique -c %s 2>&1 | FileCheck %s --check-prefix=ENABLE
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fno-gnu-unique -c %s 2>&1 | FileCheck %s --check-prefix=DISABLE
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fgnu-unique -fno-gnu-unique -c %s 2>&1 | FileCheck %s --check-prefix=DISABLE
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fno-gnu-unique -fgnu-unique -c %s 2>&1 | FileCheck %s --check-prefix=ENABLE

// GCC's libphobos shared tests pass -fno-gnu-unique because they rely on
// repeated dlopen/dlclose. Make sure the disabling spelling is accepted and
// the last spelling wins.

// DEFAULT: "-cc1"
// DEFAULT-NOT: "-fgnu-unique"
// DEFAULT-NOT: "-fno-gnu-unique"

// ENABLE: "-cc1"
// ENABLE-NOT: "-fno-gnu-unique"
// ENABLE-SAME: "-fgnu-unique"
// ENABLE-NOT: "-fno-gnu-unique"

// DISABLE-NOT: argument unused
// DISABLE: "-cc1"
// DISABLE-NOT: "-fgnu-unique"
// DISABLE-SAME: "-fno-gnu-unique"
// DISABLE-NOT: "-fgnu-unique"
