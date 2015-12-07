#pragma once

#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Very interesting, but did not work everywhere :'(
// http://stackoverflow.com/questions/10711952/how-to-detect-existence-of-a-class-using-sfinae
//----------------------------------------------------------------------------
template <typename T>
typename std::add_rvalue_reference<T>::type declval();
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
    static std::true_type test(decltype(declval<A>().~A()) *) {
        return std::true_type();
    }

    /* Has no destructor :( */
    template<typename A>
    static std::false_type test(...) {
        return std::false_type();
    }

    /* This will be either `std::true_type` or `std::false_type` */
    typedef decltype(test<T>(0)) type;

    static const bool value = type::value; /* Which is it? */
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
        using parent_type::operator =; \
        \
        _NAME() = default; \
        \
        _NAME(parent_type&& rvalue) : parent_type(std::move(rvalue)) {} \
        _NAME& operator =(parent_type&& rvalue) { parent_type::operator =(std::move(rvalue)); return *this; } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
