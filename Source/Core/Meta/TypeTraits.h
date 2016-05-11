#pragma once

#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct noinit_tag {}; // special tag used on constructors to skip object default initialization
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Very interesting, but did not work everywhere :'(
// http://stackoverflow.com/questions/10711952/how-to-detect-existence-of-a-class-using-sfinae
//----------------------------------------------------------------------------
/*! The template `has_destructor<T>` exports a
boolean constant `value that is true iff `T` has
a public destructor.

N.B. A compile error will occur if T has non-public destructor.
*/
template< typename T>
struct has_destructor
{
    /* Has destructor :) */
    template <typename A>
    static std::true_type test(decltype(std::declval<A>().~A()) *) {
        return std::true_type();
    }

    /* Has no destructor :( */
    template<typename A>
    static std::false_type test(...) {
        return std::false_type();
    }

    /* This will be either `std::true_type` or `std::false_type` */
    typedef decltype(test<T>(0)) type;

    static constexpr bool value = type::value; /* Which is it? */
};
//----------------------------------------------------------------------------
template <typename T>
using IsDefined = has_destructor<T>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define INSTANTIATE_CLASS_TYPEDEF(_NAME, ...) \
    class _NAME : public __VA_ARGS__ { \
        typedef __VA_ARGS__ parent_type; \
        STATIC_ASSERT(Core::Meta::IsDefined<parent_type>::value); \
    public: \
        using parent_type::parent_type; \
        using parent_type::operator=; \
        \
        _NAME() = default; \
        \
        template <typename _Arg0, typename... _Args> \
        _NAME(_Arg0&& arg0, _Args&&... args) \
            : parent_type(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...) {} \
        \
        template <typename _Arg0> \
        _NAME& operator=(_Arg0&& arg0) { \
            parent_type::operator =(std::forward<_Arg0>(arg0)); \
            return *this; \
        } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using Greater = std::greater<T>;
//----------------------------------------------------------------------------
template <typename T>
using GreaterEqual = std::greater_equal<T>;
//----------------------------------------------------------------------------
template <typename T>
using Less = std::less<T>;
//----------------------------------------------------------------------------
template <typename T>
using LessEqual = std::less_equal<T>;
//----------------------------------------------------------------------------
template <typename T>
using EqualTo = std::equal_to<T>;
//----------------------------------------------------------------------------
template <typename T>
using RemoveConst = typename std::remove_const<T>::type;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
