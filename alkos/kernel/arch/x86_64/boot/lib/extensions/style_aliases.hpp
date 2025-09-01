#ifndef ALKOS_BOOT_LIB_EXTENSIONS_EXPECTED_HPP_
#define ALKOS_BOOT_LIB_EXTENSIONS_EXPECTED_HPP_

#include <extensions/expected.hpp>
#include <extensions/tuple.hpp>

template <class T, class E>
using Expected = std::expected<T, E>;

template <class E>
using Unexpected = std::unexpected<E>;

template <class... Types>
using Tuple = std::tuple<Types...>;

#endif  // ALKOS_BOOT_LIB_EXTENSIONS_EXPECTED_HPP_
