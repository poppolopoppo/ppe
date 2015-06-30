#pragma once

#include "Core/Meta/BitField.h"
#include "Core/Meta/Cast.h"

#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PointerWFlagsTraits {
    typedef Bit<size_t>::First<2>::type                     field_Flag01;
    typedef Bit<size_t>::First<1>::type                     field_Flag0;
    typedef Bit<size_t>::After<field_Flag0>::Field<1>::type field_Flag1;
    typedef Bit<size_t>::After<field_Flag1>::Remain::type   field_Pointer;
};
//----------------------------------------------------------------------------
template <typename T>
struct PointerWFlags {
    typedef typename PointerWFlagsTraits::field_Flag01      field_Flag01;
    typedef typename PointerWFlagsTraits::field_Flag0       field_Flag0;
    typedef typename PointerWFlagsTraits::field_Flag1       field_Flag1;
    typedef typename PointerWFlagsTraits::field_Pointer     field_Pointer;

    size_t  _pFlags;

    size_t Flag01() const { return field_Flag01::Get(_pFlags); }
    void SetFlag01(size_t value) { field_Flag01::InplaceSet(_pFlags, value); }

    bool Flag0() const { return field_Flag0::Get(_pFlags); }
    void SetFlag0(bool value) { field_Flag0::InplaceSet(_pFlags, value); }

    bool Flag1() const { return field_Flag1::Get(_pFlags); }
    void SetFlag1(bool value) { field_Flag1::InplaceSet(_pFlags, value); }

    T *Get() const {
        return reinterpret_cast<T*>(field_Pointer::Get(_pFlags));
    }

    void Set(T *ptr) {
        field_Pointer::InplaceSet(_pFlags, ptr);
    }

    T& Reference() const {
        T* ptr = Get();
        Assert(ptr);
        return *ptr;
    }

    void Reset(T* p, bool flag0 = false, bool flag1 = false) {
        _pFlags = 0;
        field_Flag0::InplaceSet(_pFlags, flag0);
        field_Flag1::InplaceSet(_pFlags, flag1);
        field_Pointer::InplaceSet(_pFlags, p);
    }

    void Reset(T* p, size_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        field_Pointer::InplaceSet(_pFlags, p);
    }

    T* operator ->() const { return Get(); }
    T& operator *() const { return *Get(); }
};

//----------------------------------------------------------------------------
template <typename T>
struct PointerWFlags<T *> {
    typedef typename PointerWFlagsTraits::field_Flag01      field_Flag01;
    typedef typename PointerWFlagsTraits::field_Flag0       field_Flag0;
    typedef typename PointerWFlagsTraits::field_Flag1       field_Flag1;
    typedef typename PointerWFlagsTraits::field_Pointer     field_Pointer;

    size_t  _pFlags;

    size_t Flag01() const { return field_Flag01::Get(_pFlags); }
    void SetFlag01(size_t value) { field_Flag01::InplaceSet(_pFlags, value); }

    bool Flag0() const { return field_Flag0::Get(_pFlags); }
    void SetFlag0(bool value) { field_Flag0::InplaceSet(_pFlags, value); }

    bool Flag1() const { return field_Flag1::Get(_pFlags); }
    void SetFlag1(bool value) { field_Flag1::InplaceSet(_pFlags, value); }

    T *Get() const {
        return reinterpret_cast<T*>(field_Pointer::Get(_pFlags));
    }

    void Set(T *ptr) {
        field_Pointer::InplaceSet(_pFlags, ptr);
    }

    T& Reference() const {
        T* ptr = Get();
        Assert(ptr);
        return *ptr;
    }

    void Reset(T* p, bool flag0 = false, bool flag1 = false) {
        _pFlags = 0;
        field_Flag0::InplaceSet(_pFlags, flag0);
        field_Flag1::InplaceSet(_pFlags, flag1);
        field_Pointer::InplaceSet(_pFlags, p);
    }

    void Reset(T* p, size_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        field_Pointer::InplaceSet(_pFlags, p);
    }

    T* operator ->() const { return Get(); }
    T& operator *() const { return *Get(); }
};
//----------------------------------------------------------------------------
template <>
struct PointerWFlags<void> {
    typedef PointerWFlagsTraits::field_Flag01 field_Flag01;
    typedef PointerWFlagsTraits::field_Flag0 field_Flag0;
    typedef PointerWFlagsTraits::field_Flag1 field_Flag1;
    typedef PointerWFlagsTraits::field_Pointer field_Pointer;

    size_t  _pFlags;

    size_t Flag01() const { return field_Flag01::Get(_pFlags); }
    void SetFlag01(size_t value) { field_Flag01::InplaceSet(_pFlags, value); }

    bool Flag0() const { return field_Flag0::Get(_pFlags); }
    void SetFlag0(bool value) { field_Flag0::InplaceSet(_pFlags, value); }

    bool Flag1() const { return field_Flag1::Get(_pFlags); }
    void SetFlag1(bool value) { field_Flag1::InplaceSet(_pFlags, value); }

    void *Get() const {
        return reinterpret_cast<void *>(field_Pointer::Get(_pFlags));
    }
    
    void Set(void *ptr) {
        field_Pointer::InplaceSet(_pFlags, ptr);
    }

    void Reset(void *p, bool flag0 = false, bool flag1 = false) {
        _pFlags = 0;
        field_Flag0::InplaceSet(_pFlags, flag0);
        field_Flag1::InplaceSet(_pFlags, flag1);
        field_Pointer::InplaceSet(_pFlags, p);
    }

    void Reset(void *p, size_t flag01) {
        _pFlags = 0;
        field_Flag01::InplaceSet(_pFlags, flag01);
        field_Pointer::InplaceSet(_pFlags, p);
    }
};
//----------------------------------------------------------------------------
static_assert(std::is_pod< PointerWFlags<int> >::value,
                "PointerWFlags<int> must be a POD type" );
static_assert(std::is_pod< PointerWFlags<void> >::value,
                "PointerWFlags<void> must be a POD type");
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
