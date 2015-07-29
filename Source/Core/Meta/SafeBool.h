#pragma once

// Safe Bool Idiom
// http://www.artima.com/cppsource/safebool.html

#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SafeBoolBase {
protected:
    typedef void (SafeBoolBase::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}

    SafeBoolBase() {}
    SafeBoolBase(const SafeBoolBase& ) {}
    SafeBoolBase& operator=(const SafeBoolBase& ) { return *this; }
    ~SafeBoolBase() {}
};
//----------------------------------------------------------------------------
template <typename T = void>
class SafeBoolImpl : public SafeBoolBase {
    static_assert(!std::has_virtual_destructor<T>::value, "T should not be virtual");
protected:
    ~SafeBoolImpl() {}

public:
    operator bool_type () const {
        return static_cast<const T*>(this)->boolean_test()
            ? &SafeBoolBase::this_type_does_not_support_comparisons
            : nullptr;
    }
};
//----------------------------------------------------------------------------
template <>
class SafeBoolImpl<void> : public SafeBoolBase {
protected:
    virtual ~SafeBoolImpl() {}
    virtual bool boolean_test() const=0;

public:
    operator bool_type () const {
        return boolean_test()
            ? &SafeBoolBase::this_type_does_not_support_comparisons
            : nullptr;
    }
};
//----------------------------------------------------------------------------
template <typename T, typename U>
void operator ==(const SafeBoolImpl<T>& lhs,const SafeBoolImpl<U>& rhs) {
    lhs.this_type_does_not_support_comparisons();
    return false;
}
//----------------------------------------------------------------------------
template <typename T,typename U>
void operator !=(const SafeBoolImpl<T>& lhs,const SafeBoolImpl<U>& rhs) {
    lhs.this_type_does_not_support_comparisons();
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct SafeBool {
    typedef typename std::conditional<
        std::has_virtual_destructor<T>::value,
        SafeBoolImpl<void>,
        SafeBoolImpl<T>
    >   type;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
