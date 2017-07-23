#pragma once

#include "Core/Meta/BitField.h"
#include "Core/Meta/Cast.h"

#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// use a macro instead of inheritance to keep TPointerWFlags<> as a pod
#define POINTERWFLAGS_BASE_DEF() \
    typedef TBit<uintptr_t>::TFirst<2>::type field_Flag01; \
    typedef TBit<uintptr_t>::TFirst<1>::type field_Flag0; \
    typedef TBit<uintptr_t>::TAfter<field_Flag0>::TField<1>::type field_Flag1; \
    \
    uintptr_t Flag01() const { return field_Flag01::Get(_pFlags); } \
    void SetFlag01(uintptr_t value) { field_Flag01::InplaceSet(_pFlags, value); } \
    \
    bool Flag0() const { return field_Flag0::Get(_pFlags); } \
    void SetFlag0(bool value) { field_Flag0::InplaceSet(_pFlags, value); } \
    \
    bool Flag1() const { return field_Flag1::Get(_pFlags); } \
    void SetFlag1(bool value) { field_Flag1::InplaceSet(_pFlags, value); } \
    \
    void *RawPointer() const { \
        return reinterpret_cast<void *>(_pFlags & uintptr_t(field_Flag01::NotMask)); \
    } \
    \
    void SetRawPointer(const void *ptr) { \
        Assert((uintptr_t(ptr) & field_Flag01::Mask) == 0); \
        _pFlags = (_pFlags & field_Flag01::Mask) | uintptr_t(ptr); \
    } \
    \
    friend inline bool operator ==(const TPointerWFlags& lhs, const TPointerWFlags& rhs) { \
        return (lhs._pFlags == rhs._pFlags); \
    } \
    \
    friend inline bool operator !=(const TPointerWFlags& lhs, const TPointerWFlags& rhs) { \
        return (lhs._pFlags != rhs._pFlags); \
    } \
    \
    uintptr_t _pFlags
//----------------------------------------------------------------------------
template <typename T>
struct TPointerWFlags {
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

    void Reset(T* p, uintptr_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        SetRawPointer(p);
    }

    T* operator ->() const { return Get(); }
    T& operator *() const { return *Get(); }
};
//----------------------------------------------------------------------------
template <typename T>
struct TPointerWFlags<T *> {
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

    void Reset(T* p, uintptr_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        SetRawPointer(p);
    }

    T* operator ->() const { return Get(); }
    T& operator *() const { return *Get(); }
};
//----------------------------------------------------------------------------
template <>
struct TPointerWFlags<void> {
    POINTERWFLAGS_BASE_DEF();

    FORCE_INLINE void *Get() const { return RawPointer(); }
    FORCE_INLINE void Set(void *ptr) { SetRawPointer(ptr); }

    void Reset(void *p, bool flag0 = false, bool flag1 = false) {
        _pFlags = 0;
        field_Flag0::InplaceSet(_pFlags, flag0);
        field_Flag1::InplaceSet(_pFlags, flag1);
        SetRawPointer(p);
    }

    void Reset(void *p, uintptr_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        SetRawPointer(p);
    }
};
//----------------------------------------------------------------------------
#undef POINTERWFLAGS_BASE_DEF
//----------------------------------------------------------------------------
static_assert(Meta::TIsPod< TPointerWFlags<int> >::value, "TPointerWFlags<int> must be a POD type" );
static_assert(Meta::TIsPod< TPointerWFlags<void> >::value, "TPointerWFlags<void> must be a POD type");
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
