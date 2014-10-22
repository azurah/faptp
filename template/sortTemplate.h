#ifndef SORTTEMPLATE_H
#define	SORTTEMPLATE_H

#include <functional>

#include "../src/Disciplina.h"

template<typename T, typename M, template<typename> class C = std::less>
struct member_comparer : std::binary_function<T, T, bool> {

  explicit member_comparer(M T::*p) : p_(p) {
  }

  bool operator ()(T const& lhs, T const& rhs) const {
    return C<M>()(lhs.*p_, rhs.*p_);
  }

private:
  M T::*p_;
};

template<typename T, typename M>
member_comparer<T, M> make_member_comparer(M T::*p) {
  return member_comparer<T, M>(p);
}

template<template<typename> class C, typename T, typename M>
member_comparer<T, M, C> make_member_comparer2(M T::*p) {
  return member_comparer<T, M, C>(p);
}

template<typename T, typename M, template<typename> class C = std::less>
struct method_comparer : std::binary_function<T, T, bool> {

  explicit method_comparer(M(T::*p)() const) : p_(p) {
  }

  bool operator ()(T const& lhs, T const& rhs) const {
    return C<M>()((lhs.*p_)(), (rhs.*p_)());
  }

private:
  M(T::*p_)() const;
};

template<typename T, typename M>
method_comparer<T, M> make_method_comparer(M(T::*p)() const) {
  return method_comparer<T, M>(p);
}

template<template<typename> class C, typename T, typename M>
method_comparer<T, M, C> make_method_comparer2(M(T::*p)() const) {
  return method_comparer<T, M, C>(p);
}

struct DisciplinaCargaHorariaDesc {

  bool operator() (const Disciplina *a, const Disciplina *b) {
    return a->getCargaHoraria() > b->getCargaHoraria();
  }
};

#endif	/* SORTTEMPLATE_H */

