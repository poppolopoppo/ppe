#pragma once

// Safe Bool Idiom
// http://www.artima.com/cppsource/safebool.html

#include <type_traits>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSafeBoolBase {
protected:
    typedef void (FSafeBoolBase::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}

    FSafeBoolBase() {}
    FSafeBoolBase(const FSafeBoolBase& ) {}
    FSafeBoolBase& operator=(const FSafeBoolBase& ) { return *this; }
    ~FSafeBoolBase() = default;
};
//----------------------------------------------------------------------------
template <typename T = void>
class TSafeBoolImpl : public FSafeBoolBase {
    static_assert(!std::has_virtual_destructor<T>::value, "T should not be virtual");
protected:
    ~TSafeBoolImpl() = default;

public:
    operator bool_type () const {
        return static_cast<const T*>(this)->boolean_test()
            ? &FSafeBoolBase::this_type_does_not_support_comparisons
            : nullptr;
    }
};
//----------------------------------------------------------------------------
template <>
class TSafeBoolImpl<void> : public FSafeBoolBase {
protected:
    virtual ~TSafeBoolImpl() = default;
    virtual bool boolean_test() const=0;

public:
    operator bool_type () const {
        return boolean_test()
            ? &FSafeBoolBase::this_type_does_not_support_comparisons
            : nullptr;
    }
};
//----------------------------------------------------------------------------
template <typename T, typename U>
void operator ==(const TSafeBoolImpl<T>& lhs,const TSafeBoolImpl<U>& rhs) {
    lhs.this_type_does_not_support_comparisons();
    return false;
}
//----------------------------------------------------------------------------
template <typename T,typename U>
void operator !=(const TSafeBoolImpl<T>& lhs,const TSafeBoolImpl<U>& rhs) {
    lhs.this_type_does_not_support_comparisons();
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct FSafeBool {
    typedef typename std::conditional<
        std::has_virtual_destructor<T>::value,
        TSafeBoolImpl<void>,
        TSafeBoolImpl<T>
    >   type;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
