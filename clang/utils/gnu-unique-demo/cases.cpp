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
