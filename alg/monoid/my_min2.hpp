#pragma once

template <typename T, typename E = int>
struct My_Monoid_Min2 {
  struct Data {
    T min1, min2;
    E id1, id2;
    Data(E id, T x){
        id1 = id;
        min1 = x;
        id2 = -1;
        min2 = infty<T>;        
    }
    Data(E i1, T m1, E i2, T m2){
        id1 = i1;
        min1 = m1;
        id2 = i2;
        min2 = m2;        
    }
    Data(){
        min1 = infty<T>;
        id1 = -1;
        min2 = infty<T>;
        id2 = -1;
    }
    bool add_element(E id, T x) {
      if (min1 > x) {
        min2 = min1, id2 = id1, min1 = x, id1 = id;
        return 1;
      }
      elif (min2 > x) {
        min2 = x, id2 = id;
        return 1;
      }
      return 0;
    }
  };
  using value_type = Data;
  using X = value_type;

  static X op(X x, X y) {
    // SHOW(x.min1, x.id1, x.min2, x.id2);
    // SHOW(y.min1, y.id1, y.min2, y.id2);
    x.add_element(y.id1, y.min1);
    x.add_element(y.id2, y.min2);
    // SHOW(x.min1, x.id1, x.min2, x.id2);
    return x;
  }
  static constexpr X unit() { return {-1, infty<T>, -1, infty<T>}; }
  static constexpr bool commute = true;
};