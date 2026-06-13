// REQUIRES: x86-registered-target

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -emit-obj -o - %s | llvm-readelf --file-header --symbols - | FileCheck %s --check-prefix=GNU
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fgnu-unique -emit-obj -o - %s | llvm-readelf --file-header --symbols - | FileCheck %s --check-prefix=GNU
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fno-gnu-unique -emit-obj -o - %s | llvm-readelf --file-header --symbols - | FileCheck %s --check-prefix=NOGNU --implicit-check-not=UNIQUE

template <class T> struct Holder {
  static int value;
  static const int constant;
  static thread_local int tls;
};

template <class T> int Holder<T>::value = 1;
template <class T> const int Holder<T>::constant = 2;
template <class T> thread_local int Holder<T>::tls = 3;

template struct Holder<int>;

int side();

template <class T> struct DynamicHolder {
  static int value;
};

template <class T> int DynamicHolder<T>::value = side();

template struct DynamicHolder<int>;

inline int &local_static() {
  static int x = 4;
  return x;
}

inline int dynamic_static() {
  static int y = side();
  return y;
}

struct HasVTable {
  virtual void f() {}
};

__attribute__((selectany)) int selectany_global = 5;
__attribute__((weak)) int weak_plain = 6;

int use() {
  HasVTable h;
  return Holder<int>::value + Holder<int>::constant + Holder<int>::tls +
         DynamicHolder<int>::value + local_static() + dynamic_static() +
         selectany_global + weak_plain;
}

// GNU: OS/ABI: UNIX - GNU
// GNU-DAG: OBJECT  UNIQUE DEFAULT {{.*}} _ZN6HolderIiE5valueE
// GNU-DAG: OBJECT  UNIQUE DEFAULT {{.*}} _ZN6HolderIiE8constantE
// GNU-DAG: TLS     UNIQUE DEFAULT {{.*}} _ZN6HolderIiE3tlsE
// GNU-DAG: OBJECT  UNIQUE DEFAULT {{.*}} _ZN13DynamicHolderIiE5valueE
// GNU-DAG: OBJECT  UNIQUE DEFAULT {{.*}} _ZGVN13DynamicHolderIiE5valueE
// GNU-DAG: OBJECT  UNIQUE DEFAULT {{.*}} _ZZ12local_staticvE1x
// GNU-DAG: OBJECT  UNIQUE DEFAULT {{.*}} _ZZ14dynamic_staticvE1y
// GNU-DAG: OBJECT  UNIQUE DEFAULT {{.*}} _ZGVZ14dynamic_staticvE1y
// GNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZTV9HasVTable
// GNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZTI9HasVTable
// GNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZTS9HasVTable
// GNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} selectany_global
// GNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} weak_plain

// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZN6HolderIiE5valueE
// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZN6HolderIiE8constantE
// NOGNU-DAG: TLS     WEAK   DEFAULT {{.*}} _ZN6HolderIiE3tlsE
// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZN13DynamicHolderIiE5valueE
// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZGVN13DynamicHolderIiE5valueE
// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZZ12local_staticvE1x
// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZZ14dynamic_staticvE1y
// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} _ZGVZ14dynamic_staticvE1y
// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} selectany_global
// NOGNU-DAG: OBJECT  WEAK   DEFAULT {{.*}} weak_plain
