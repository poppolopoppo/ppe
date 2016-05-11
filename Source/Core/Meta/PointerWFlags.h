#pragma once

#include "Core/Meta/BitField.h"
#include "Core/Meta/Cast.h"

#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// use a macro instead of inheritance to keep PointerWFlags<> as a pod
#define POINTERWFLAGS_BASE_DEF() \
    typedef Bit<size_t>::First<2>::type field_Flag01; \
    typedef Bit<size_t>::First<1>::type field_Flag0; \
    typedef Bit<size_t>::After<field_Flag0>::Field<1>::type field_Flag1; \
    \
    size_t Flag01() const { return field_Flag01::Get(_pFlags); } \
    void SetFlag01(size_t value) { field_Flag01::InplaceSet(_pFlags, value); } \
    \
    bool Flag0() const { return field_Flag0::Get(_pFlags); } \
    void SetFlag0(bool value) { field_Flag0::InplaceSet(_pFlags, value); } \
    \
    bool Flag1() const { return field_Flag1::Get(_pFlags); } \
    void SetFlag1(bool value) { field_Flag1::InplaceSet(_pFlags, value); } \
    \
    void *RawPointer() const { \
        return reinterpret_cast<void *>(_pFlags & size_t(field_Flag01::NotMask)); \
    } \
    \
    void SetRawPointer(const void *ptr) { \
        Assert((size_t(ptr) & field_Flag01::Mask) == 0); \
        _pFlags = (_pFlags & field_Flag01::Mask) | size_t(ptr); \
    } \
    \
    friend inline bool operator ==(const PointerWFlags& lhs, const PointerWFlags& rhs) { \
        return (lhs._pFlags == rhs._pFlags); \
    } \
    \
    friend inline bool operator !=(const PointerWFlags& lhs, const PointerWFlags& rhs) { \
        return (lhs._pFlags != rhs._pFlags); \
    } \
    \
    size_t  _pFlags
//----------------------------------------------------------------------------
template <typename T>
struct PointerWFlags {
    POINTERWFLAGS_BASE_DEF();

    FORCE_INLINE T *Get() const { return reinterpret_cast<T*>(RawPointer()); }
    FORCE_INLINE void Set(T *ptr) { SetRawPointer(ptr); }

    T& Reference() const {
        T* ptr = Get();
        Assert(ptr);
        return *ptr;
    }

    void Reset(T* p, bool flag0 = false, bool flag1 = false) {
        _pFlags = 0;
        field_Flag0::InplaceSet(_pFlags, flag0);
        field_Flag1::InplaceSet(_pFlags, flag1);
        SetRawPointer(p);
    }

    void Reset(T* p, size_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        SetRawPointer(p);
    }

    T* operator ->() const { return Get(); }
    T& operator *() const { return *Get(); }
};
//----------------------------------------------------------------------------
template <typename T>
struct PointerWFlags<T *> {
    POINTERWFLAGS_BASE_DEF();

    FORCE_INLINE T *Get() const { return reinterpret_cast<T*>(RawPointer()); }
    FORCE_INLINE void Set(T *ptr) { SetRawPointer(ptr); }

    T& Reference() const {
        T* ptr = Get();
        Assert(ptr);
        return *ptr;
    }

    void Reset(T* p, bool flag0 = false, bool flag1 = false) {
        _pFlags = 0;
        field_Flag0::InplaceSet(_pFlags, flag0);
        field_Flag1::InplaceSet(_pFlags, flag1);
        SetRawPointer(p);
    }

    void Reset(T* p, size_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        SetRawPointer(p);
    }

    T* operator ->() const { return Get(); }
    T& operator *() const { return *Get(); }
};
//----------------------------------------------------------------------------
template <>
struct PointerWFlags<void> {
    POINTERWFLAGS_BASE_DEF();

    FORCE_INLINE void *Get() const { return RawPointer(); }
    FORCE_INLINE void Set(void *ptr) { SetRawPointer(ptr); }

    void Reset(void *p, bool flag0 = false, bool flag1 = false) {
        _pFlags = 0;
        field_Flag0::InplaceSet(_pFlags, flag0);
        field_Flag1::InplaceSet(_pFlags, flag1);
        SetRawPointer(p);
    }

    void Reset(void *p, size_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        SetRawPointer(p);
    }
};
//----------------------------------------------------------------------------
#undef POINTERWFLAGS_BASE_DEF
//----------------------------------------------------------------------------
static_assert(std::is_pod< PointerWFlags<int> >::value, "PointerWFlags<int> must be a POD type" );
static_assert(std::is_pod< PointerWFlags<void> >::value, "PointerWFlags<void> must be a POD type");
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
