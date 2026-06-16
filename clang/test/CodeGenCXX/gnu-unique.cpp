// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fgnu-unique -emit-llvm -o - %s | FileCheck %s --check-prefix=GNU
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fno-gnu-unique -emit-llvm -o - %s | FileCheck %s --check-prefix=NOGNU --implicit-check-not=!gnu_unique

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

constexpr bool ref_temp_is_constant_evaluated() {
  return __builtin_is_constant_evaluated();
}

namespace std {
template <class T> struct initializer_list {
  const T *ptr;
  decltype(sizeof(0)) len;
  constexpr initializer_list(const T *p, decltype(sizeof(0)) n)
      : ptr(p), len(n) {}
  constexpr const T *begin() const { return ptr; }
};
} // namespace std

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

struct RefTemp {
  int value;
};

constexpr RefTemp make_ref_temp() { return {18}; }
constexpr const RefTemp make_const_ref_temp() { return {19}; }

struct RefTempDtor {
  int value;
  constexpr RefTempDtor(int value) : value(value) {}
  constexpr ~RefTempDtor() {}
};

struct RefTempMutable {
  mutable int value;
};

inline const RefTemp &ref_temp_object = RefTemp{7};
inline const int &ref_temp_dynamic_scalar = side();
inline const int (&ref_temp_dynamic_array)[2] = {side(), side()};
inline thread_local const RefTemp &ref_temp_tls = RefTemp{8};

template <class T> inline const RefTemp &ref_temp_var_template = RefTemp{9};

struct RefTempHolder {
  static inline const RefTemp &member = RefTemp{10};
};

inline const RefTemp *ref_temp_local() {
  static const RefTemp &local = RefTemp{11};
  return &local;
}

inline std::initializer_list<int> ref_temp_dynamic_list = {side(), side()};

inline const int &ref_temp_const_scalar = 42;
inline const int (&ref_temp_const_array)[2] = {1, 2};
inline const RefTemp (&ref_temp_const_class_array)[2] = {RefTemp{12},
                                                         RefTemp{13}};
inline std::initializer_list<int> ref_temp_const_list = {1, 2, 3};
inline const volatile int &&ref_temp_volatile_scalar = 14;
inline const volatile int (&&ref_temp_volatile_array)[2] = {15, 16};
inline const RefTemp &ref_temp_explicit_const_object =
    static_cast<const RefTemp>(RefTemp{17});
inline const RefTemp &ref_temp_nonconst_call = make_ref_temp();
inline const RefTemp &ref_temp_const_call = make_const_ref_temp();
inline const RefTempDtor (&ref_temp_constexpr_dtor_array)[2] = {
    RefTempDtor{20}, RefTempDtor{21}};
inline std::initializer_list<RefTempDtor> ref_temp_constexpr_dtor_list = {
    RefTempDtor{22}, RefTempDtor{23}};
inline const volatile RefTemp &&ref_temp_volatile_object = RefTemp{24};
inline const RefTempMutable &ref_temp_mutable_object = RefTempMutable{25};
inline const int &ref_temp_comma_scalar = (side(), 26);
inline const RefTemp &ref_temp_comma_const_object =
    (side(), static_cast<const RefTemp>(RefTemp{27}));
inline const int &ref_temp_ice_scalar =
    ref_temp_is_constant_evaluated() ? side() : 28;
inline const RefTemp &ref_temp_ice_const_object =
    static_cast<const RefTemp>(ref_temp_is_constant_evaluated()
                                   ? RefTemp{side()}
                                   : RefTemp{29});

extern "C" const volatile void *use_ref_temps(int n) {
  switch (n) {
  case 0:
    return &ref_temp_object;
  case 1:
    return &ref_temp_dynamic_scalar;
  case 2:
    return &ref_temp_dynamic_array;
  case 3:
    return &ref_temp_tls;
  case 4:
    return &ref_temp_var_template<int>;
  case 5:
    return &RefTempHolder::member;
  case 6:
    return ref_temp_local();
  case 7:
    return ref_temp_dynamic_list.begin();
  case 8:
    return &ref_temp_const_scalar;
  case 9:
    return &ref_temp_const_array;
  case 10:
    return &ref_temp_const_class_array;
  case 11:
    return &ref_temp_volatile_scalar;
  case 12:
    return &ref_temp_volatile_array;
  case 13:
    return &ref_temp_explicit_const_object;
  case 14:
    return &ref_temp_nonconst_call;
  case 15:
    return &ref_temp_const_call;
  case 16:
    return &ref_temp_constexpr_dtor_array;
  case 17:
    return ref_temp_constexpr_dtor_list.begin();
  case 18:
    return &ref_temp_volatile_object;
  case 19:
    return &ref_temp_mutable_object;
  case 20:
    return &ref_temp_comma_scalar;
  case 21:
    return &ref_temp_comma_const_object;
  case 22:
    return &ref_temp_ice_scalar;
  case 23:
    return &ref_temp_ice_const_object;
  default:
    return ref_temp_const_list.begin();
  }
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

// GNU-DAG: @_ZN6HolderIiE5valueE = weak_odr {{.*}}global i32 1, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZN6HolderIiE8constantE = weak_odr {{.*}}constant i32 2, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZN6HolderIiE3tlsE = weak_odr {{.*}}thread_local global i32 3, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZN13DynamicHolderIiE5valueE = weak_odr {{.*}}global i32 0, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGVN13DynamicHolderIiE5valueE = weak_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @_ZZ12local_staticvE1x = linkonce_odr {{.*}}global i32 4, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZZ14dynamic_staticvE1y = linkonce_odr {{.*}}global i32 0, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGVZ14dynamic_staticvE1y = linkonce_odr {{.*}}global i64 0, comdat, align 8, !gnu_unique
// GNU-DAG: @ref_temp_tls = linkonce_odr {{.*}}thread_local global {{.*}} null, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV12ref_temp_tls = linkonce_odr {{.*}}thread_local global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @_Z21ref_temp_var_templateIiE = linkonce_odr {{.*}}constant {{.*}} @_ZGR21ref_temp_var_templateIiE_, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZZ14ref_temp_localvE5local = linkonce_odr {{.*}}constant {{.*}} @_ZGRZ14ref_temp_localvE5local_, comdat, align 8, !gnu_unique
// GNU-DAG: @ref_temp_dynamic_scalar = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV23ref_temp_dynamic_scalar = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @ref_temp_dynamic_array = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV22ref_temp_dynamic_array = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @ref_temp_dynamic_list = linkonce_odr {{.*}}global %"struct.std::initializer_list" zeroinitializer, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV21ref_temp_dynamic_list = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @ref_temp_const_list = linkonce_odr {{.*}}global %"struct.std::initializer_list" { {{.*}} @_ZGR19ref_temp_const_list_, i64 3 }, comdat, align 8, !gnu_unique
// GNU-DAG: @ref_temp_constexpr_dtor_array = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV29ref_temp_constexpr_dtor_array = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @ref_temp_constexpr_dtor_list = linkonce_odr {{.*}}global %"struct.std::initializer_list.0" zeroinitializer, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV28ref_temp_constexpr_dtor_list = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @ref_temp_comma_scalar = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV21ref_temp_comma_scalar = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @ref_temp_comma_const_object = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV27ref_temp_comma_const_object = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @ref_temp_ice_scalar = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV19ref_temp_ice_scalar = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @ref_temp_ice_const_object = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8, !gnu_unique
// GNU-DAG: @_ZGV25ref_temp_ice_const_object = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8, !gnu_unique
// GNU-DAG: @_ZGR15ref_temp_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 7 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR23ref_temp_dynamic_scalar_ = linkonce_odr {{.*}}global i32 0, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR22ref_temp_dynamic_array_ = linkonce_odr {{.*}}global [2 x i32] zeroinitializer, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR21ref_temp_dynamic_list_ = linkonce_odr {{.*}}global [2 x i32] zeroinitializer, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR12ref_temp_tls_ = linkonce_odr {{.*}}thread_local {{global|constant}} %struct.RefTemp { i32 8 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR21ref_temp_var_templateIiE_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 9 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGRN13RefTempHolder6memberE_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 10 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGRZ14ref_temp_localvE5local_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 11 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR21ref_temp_const_scalar_ = linkonce_odr {{.*}}{{global|constant}} i32 42, comdat, align 4{{$}}
// GNU-DAG: @_ZGR20ref_temp_const_array_ = linkonce_odr {{.*}}{{global|constant}} [2 x i32] [i32 1, i32 2], comdat, align 4{{$}}
// GNU-DAG: @_ZGR26ref_temp_const_class_array_ = linkonce_odr {{.*}}{{global|constant}} [2 x %struct.RefTemp] [%struct.RefTemp { i32 12 }, %struct.RefTemp { i32 13 }], comdat, align 4{{$}}
// GNU-DAG: @_ZGR19ref_temp_const_list_ = linkonce_odr {{.*}}{{global|constant}} [3 x i32] [i32 1, i32 2, i32 3], comdat, align 4{{$}}
// GNU-DAG: @_ZGR24ref_temp_volatile_scalar_ = linkonce_odr {{.*}}{{global|constant}} i32 14, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR23ref_temp_volatile_array_ = linkonce_odr {{.*}}{{global|constant}} [2 x i32] [i32 15, i32 16], comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR30ref_temp_explicit_const_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 17 }, comdat, align 4{{$}}
// GNU-DAG: @_ZGR22ref_temp_nonconst_call_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 18 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR19ref_temp_const_call_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 19 }, comdat, align 4{{$}}
// GNU-DAG: @_ZGR29ref_temp_constexpr_dtor_array_ = linkonce_odr {{.*}}global [2 x %struct.RefTempDtor] zeroinitializer, comdat, align 4{{$}}
// GNU-DAG: @_ZGR28ref_temp_constexpr_dtor_list_ = linkonce_odr {{.*}}global [2 x %struct.RefTempDtor] zeroinitializer, comdat, align 4{{$}}
// GNU-DAG: @_ZGR24ref_temp_volatile_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 24 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR23ref_temp_mutable_object_ = linkonce_odr {{.*}}global %struct.RefTempMutable { i32 25 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR21ref_temp_comma_scalar_ = linkonce_odr {{.*}}{{global|constant}} i32 26, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR27ref_temp_comma_const_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 27 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR19ref_temp_ice_scalar_ = linkonce_odr {{.*}}{{global|constant}} i32 28, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZGR25ref_temp_ice_const_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 29 }, comdat, align 4, !gnu_unique
// GNU-DAG: @_ZTV9HasVTable = linkonce_odr {{.*}}constant {{.*}}, comdat, align 8{{$}}
// GNU-DAG: @selectany_global = weak_odr {{.*}}global i32 5, comdat, align 4{{$}}
// GNU-DAG: @weak_plain = weak global i32 6, align 4{{$}}

// NOGNU-DAG: @_ZN6HolderIiE5valueE = weak_odr {{.*}}global i32 1, comdat, align 4{{$}}
// NOGNU-DAG: @_ZN6HolderIiE8constantE = weak_odr {{.*}}constant i32 2, comdat, align 4{{$}}
// NOGNU-DAG: @_ZN6HolderIiE3tlsE = weak_odr {{.*}}thread_local global i32 3, comdat, align 4{{$}}
// NOGNU-DAG: @_ZN13DynamicHolderIiE5valueE = weak_odr {{.*}}global i32 0, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGVN13DynamicHolderIiE5valueE = weak_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @_ZZ12local_staticvE1x = linkonce_odr {{.*}}global i32 4, comdat, align 4{{$}}
// NOGNU-DAG: @_ZZ14dynamic_staticvE1y = linkonce_odr {{.*}}global i32 0, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGVZ14dynamic_staticvE1y = linkonce_odr {{.*}}global i64 0, comdat, align 8{{$}}
// NOGNU-DAG: @ref_temp_tls = linkonce_odr {{.*}}thread_local global {{.*}} null, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV12ref_temp_tls = linkonce_odr {{.*}}thread_local global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @_Z21ref_temp_var_templateIiE = linkonce_odr {{.*}}constant {{.*}} @_ZGR21ref_temp_var_templateIiE_, comdat, align 8{{$}}
// NOGNU-DAG: @_ZZ14ref_temp_localvE5local = linkonce_odr {{.*}}constant {{.*}} @_ZGRZ14ref_temp_localvE5local_, comdat, align 8{{$}}
// NOGNU-DAG: @ref_temp_dynamic_scalar = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV23ref_temp_dynamic_scalar = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @ref_temp_dynamic_array = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV22ref_temp_dynamic_array = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @ref_temp_dynamic_list = linkonce_odr {{.*}}global %"struct.std::initializer_list" zeroinitializer, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV21ref_temp_dynamic_list = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @ref_temp_const_list = linkonce_odr {{.*}}global %"struct.std::initializer_list" { {{.*}} @_ZGR19ref_temp_const_list_, i64 3 }, comdat, align 8{{$}}
// NOGNU-DAG: @ref_temp_constexpr_dtor_array = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV29ref_temp_constexpr_dtor_array = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @ref_temp_constexpr_dtor_list = linkonce_odr {{.*}}global %"struct.std::initializer_list.0" zeroinitializer, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV28ref_temp_constexpr_dtor_list = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @ref_temp_comma_scalar = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV21ref_temp_comma_scalar = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @ref_temp_comma_const_object = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV27ref_temp_comma_const_object = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @ref_temp_ice_scalar = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV19ref_temp_ice_scalar = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @ref_temp_ice_const_object = linkonce_odr {{.*}}global {{.*}} null, comdat, align 8{{$}}
// NOGNU-DAG: @_ZGV25ref_temp_ice_const_object = linkonce_odr {{.*}}global i64 0, comdat{{.*}}, align 8{{$}}
// NOGNU-DAG: @_ZGR15ref_temp_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 7 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR23ref_temp_dynamic_scalar_ = linkonce_odr {{.*}}global i32 0, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR22ref_temp_dynamic_array_ = linkonce_odr {{.*}}global [2 x i32] zeroinitializer, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR21ref_temp_dynamic_list_ = linkonce_odr {{.*}}global [2 x i32] zeroinitializer, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR12ref_temp_tls_ = linkonce_odr {{.*}}thread_local {{global|constant}} %struct.RefTemp { i32 8 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR21ref_temp_var_templateIiE_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 9 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGRN13RefTempHolder6memberE_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 10 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGRZ14ref_temp_localvE5local_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 11 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR21ref_temp_const_scalar_ = linkonce_odr {{.*}}{{global|constant}} i32 42, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR20ref_temp_const_array_ = linkonce_odr {{.*}}{{global|constant}} [2 x i32] [i32 1, i32 2], comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR26ref_temp_const_class_array_ = linkonce_odr {{.*}}{{global|constant}} [2 x %struct.RefTemp] [%struct.RefTemp { i32 12 }, %struct.RefTemp { i32 13 }], comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR19ref_temp_const_list_ = linkonce_odr {{.*}}{{global|constant}} [3 x i32] [i32 1, i32 2, i32 3], comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR24ref_temp_volatile_scalar_ = linkonce_odr {{.*}}{{global|constant}} i32 14, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR23ref_temp_volatile_array_ = linkonce_odr {{.*}}{{global|constant}} [2 x i32] [i32 15, i32 16], comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR30ref_temp_explicit_const_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 17 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR22ref_temp_nonconst_call_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 18 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR19ref_temp_const_call_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 19 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR29ref_temp_constexpr_dtor_array_ = linkonce_odr {{.*}}global [2 x %struct.RefTempDtor] zeroinitializer, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR28ref_temp_constexpr_dtor_list_ = linkonce_odr {{.*}}global [2 x %struct.RefTempDtor] zeroinitializer, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR24ref_temp_volatile_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 24 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR23ref_temp_mutable_object_ = linkonce_odr {{.*}}global %struct.RefTempMutable { i32 25 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR21ref_temp_comma_scalar_ = linkonce_odr {{.*}}{{global|constant}} i32 26, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR27ref_temp_comma_const_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 27 }, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR19ref_temp_ice_scalar_ = linkonce_odr {{.*}}{{global|constant}} i32 28, comdat, align 4{{$}}
// NOGNU-DAG: @_ZGR25ref_temp_ice_const_object_ = linkonce_odr {{.*}}{{global|constant}} %struct.RefTemp { i32 29 }, comdat, align 4{{$}}
// NOGNU-DAG: @selectany_global = weak_odr {{.*}}global i32 5, comdat, align 4{{$}}
// NOGNU-DAG: @weak_plain = weak global i32 6, align 4{{$}}
