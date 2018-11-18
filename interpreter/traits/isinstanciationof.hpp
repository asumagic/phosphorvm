#pragma once

#include <type_traits>

//! Inherits from std::true_type when TT is a template instanciation of T.
template<class T, template<class> class TT>
struct is_instantiation_of : std::false_type {};

template<class T, template<class> class TT>
struct is_instantiation_of<TT<T>, TT> : std::true_type {};
