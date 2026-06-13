// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fgnu-unique -emit-llvm -o - %s | FileCheck %s --check-prefix=GNU
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fno-gnu-unique -emit-llvm -o - %s | FileCheck %s --check-prefix=NOGNU

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

int use() {
  HasVTable h;
  return Holder<int>::value + Holder<int>::constant + Holder<int>::tls +
         DynamicHolder<int>::value + local_static() + dynamic_static();
}

// GNU-DAG: @_ZN6HolderIiE5valueE = weak_odr {{.*}}global i32 1, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZN6HolderIiE8constantE = weak_odr {{.*}}constant i32 2, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZN6HolderIiE3tlsE = weak_odr {{.*}}thread_local global i32 3, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZN13DynamicHolderIiE5valueE = weak_odr {{.*}}global i32 0, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGVN13DynamicHolderIiE5valueE = weak_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @_ZZ12local_staticvE1x = linkonce_odr {{.*}}global i32 4, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZZ14dynamic_staticvE1y = linkonce_odr {{.*}}global i32 0, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGVZ14dynamic_staticvE1y = linkonce_odr {{.*}}global i64 0, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZTV9HasVTable = linkonce_odr {{.*}}constant {{.*}}, comdat, align 8{{$}}

// NOGNU-DAG: @_ZN6HolderIiE5valueE = weak_odr {{.*}}global i32 1, comdat, align 4{{$}}
// NOGNU-DAG: @_ZN6HolderIiE8constantE = weak_odr {{.*}}constant i32 2, comdat, align 4{{$}}
// NOGNU-DAG: @_ZN6HolderIiE3tlsE = weak_odr {{.*}}thread_local global i32 3, comdat, align 4{{$}}
// NOGNU-DAG: @_ZN13DynamicHolderIiE5valueE = weak_odr {{.*}}global i32 0, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGVN13DynamicHolderIiE5valueE = weak_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @_ZZ12local_staticvE1x = linkonce_odr {{.*}}global i32 4, comdat, align 4{{$}}
// NOGNU-DAG: @_ZZ14dynamic_staticvE1y = linkonce_odr {{.*}}global i32 0, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGVZ14dynamic_staticvE1y = linkonce_odr {{.*}}global i64 0, comdat, align 8{{$}}
