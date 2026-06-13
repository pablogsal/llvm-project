struct Environment {};
struct EnvironmentImpl {};

template <class T> struct VTV {
  __attribute__((visibility("hidden"), section(".vtable_map_vars")))
  static void *__vtable_map;
};

template <class T>
__attribute__((visibility("hidden"), section(".vtable_map_vars")))
void *VTV<T>::__vtable_map;

template struct VTV<Environment>;
template struct VTV<EnvironmentImpl>;

void *use_vtv() { return VTV<Environment>::__vtable_map; }
