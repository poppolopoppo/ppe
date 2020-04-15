#pragma once

#include "RTTI_fwd.h"

#include "MetaClass.h"
#include "RTTI/Exceptions.h"
#include "RTTI/Typedefs.h"

#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryDomain.h"
#include "Memory/RefPtr.h"

#if !(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING)
#   define WITH_RTTI_VERIFY_PREDICATES
#endif

#ifdef WITH_RTTI_VERIFY_PREDICATES
#   define RTTI_VerifyPredicate(...) AssertRelease(COMMA_PROTECT(__VA_ARGS__))
#endif

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EObjectFlags : u32 {
    None        = 0,
    Loaded      = 1<<0,
    Unloaded    = 1<<1,
    Exported    = 1<<2,
    TopObject   = 1<<3,
    Transient   = 1<<4,
    Frozen      = 1<<5,
#ifdef WITH_RTTI_VERIFY_PREDICATES
    Verifying   = 1<<6,
#endif

    All         = UINT32_MAX
};
ENUM_FLAGS(EObjectFlags);
//----------------------------------------------------------------------------
class ILoadContext {
public:
    virtual ~ILoadContext() = default;
    virtual void OnLoadObject(FMetaObject& object) = 0;
};
//----------------------------------------------------------------------------
class IUnloadContext {
public:
    virtual ~IUnloadContext() = default;
    virtual void OnUnloadObject(FMetaObject& object) = 0;
};
//----------------------------------------------------------------------------
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
template <typename T>
using IsMetaObject = std::is_base_of<FMetaObject, Meta::TDecay<T> >;
template <typename T>
using TEnableIfMetaObject = Meta::TEnableIf< IsMetaObject<T>::value, T>;
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaObject : public FRefCountable {
public:
    FMetaObject();
    virtual ~FMetaObject();

    FMetaObject(const FMetaObject&) = delete;
    FMetaObject& operator =(const FMetaObject&) = delete;

    const FName& RTTI_Name() const  { return _name; }
    EObjectFlags RTTI_Flags() const { return _flags; }
    PTypeTraits RTTI_Traits() const { return RTTI_Class()->MakeTraits(); }

    bool RTTI_IsLoaded() const      { return (_flags ^ EObjectFlags::Loaded     ); }
    bool RTTI_IsUnloaded() const    { return (_flags ^ EObjectFlags::Unloaded   ); }
    bool RTTI_IsExported() const    { return (_flags ^ EObjectFlags::Exported   ); }
    bool RTTI_IsFrozen() const      { return (_flags ^ EObjectFlags::Frozen     ); }
    bool RTTI_IsTopObject() const   { return (_flags ^ EObjectFlags::TopObject  ); }
    bool RTTI_IsTransient() const   { return (_flags ^ EObjectFlags::Transient  ); }

    const FMetaTransaction* RTTI_Outer() const { return _outer.get(); }
    void RTTI_SetOuter(const FMetaTransaction* outer, const FMetaTransaction* prevOuterForDbg = nullptr);

    FPathName RTTI_PathName() const;

    bool RTTI_IsA(const FMetaClass& metaClass) const;
    bool RTTI_CastTo(const FMetaClass& metaClass) const;
    bool RTTI_InheritsFrom(const FMetaClass& metaClass) const;
    bool RTTI_IsAssignableFrom(const FMetaClass& metaClass) const;

    void RTTI_ResetToDefaultValue();

    void RTTI_Freeze(); // frozen object can't be modified
    void RTTI_Unfreeze();

    void RTTI_Export(const FName& name);
    void RTTI_Unexport();

    bool RTTI_CallLoadIFN(ILoadContext& context);
    bool RTTI_CallUnloadIFN(IUnloadContext& context);

    void RTTI_MarkAsTopObject(); // should only be called by FMetaTransaction
    void RTTI_UnmarkAsTopObject(); // should only be called by FMetaTransaction

    virtual void RTTI_Load(ILoadContext& context);
    virtual void RTTI_Unload(IUnloadContext& context);

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const PPE_THROW();
#endif

public: // Meta class
    class PPE_RTTI_API RTTI_FMetaClass : public FMetaClass {
    public:
        typedef FMetaObject object_type;
        typedef void parent_type;

        virtual const FMetaClass* Parent() const override final;
        virtual bool CreateInstance(PMetaObject& dst, bool resetToDefaultValue = true) const override final;
        virtual PTypeTraits MakeTraits() const override final;

        static const FMetaClass* Get();
        static FMetaModule& Module();

    private:
        static const FMetaClassHandle GMetaClassHandle;
        static FMetaClass* CreateMetaClass_(FClassId id, const FMetaModule* module);

        RTTI_FMetaClass(FClassId id, const FMetaModule* module);
    };

    virtual const RTTI::FMetaClass* RTTI_Class() const {
        return RTTI_FMetaClass::Get();
    }

private:
    FName _name;
#ifdef WITH_RTTI_VERIFY_PREDICATES
    mutable EObjectFlags _flags;
#else
    EObjectFlags _flags;
#endif
    SCMetaTransaction _outer;
};
//----------------------------------------------------------------------------
// provide memory tracking for each RTTI meta-class
#if USE_PPE_MEMORYDOMAINS
template <typename T, typename... _Args>
TRefPtr< TEnableIfMetaObject<FMetaObject> > NewRtti(const FMetaClass& metaClass, _Args&&... args) {
    return NewRef<T>(metaClass.TrackingData(), std::forward<_Args>(args)...);
}
#else
template <typename T, typename... _Args>
TRefPtr< TEnableIfMetaObject<FMetaObject> > NewRtti(_Args&&... args) {
    return NewRef<T>(std::forward<_Args>(args)...);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T* Cast(FMetaObject* p) {
    Assert(p);
    const FMetaClass* const metaClass = MetaClass<T>();
    Assert(metaClass);
    return (p->RTTI_CastTo(*metaClass) ? checked_cast<T*>(p) : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
const T* Cast(const FMetaObject* p) {
    Assert(p);
    const FMetaClass* const metaClass = MetaClass<T>();
    Assert(metaClass);
    return (p->RTTI_CastTo(*metaClass) ? checked_cast<const T*>(p) : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
T* CastChecked(FMetaObject* p) {
#if USE_PPE_ASSERT_RELEASE
    T* const result = Cast<T>(p);
    AssertRelease(result);
    return result;
#else
    return static_cast<T*>(p);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
const T* CastChecked(const FMetaObject* p) {
#if USE_PPE_ASSERT_RELEASE
    const T* const result = Cast<T>(p);
    AssertRelease(result);
    return result;
#else
    return static_cast<const T*>(p);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
auto Cast(const PMetaObject& p) { return Cast<T>(p.get()); }
template <typename T>
auto CastChecked(const PMetaObject& p) { return Cast<T>(p.get()); }
//----------------------------------------------------------------------------
template <typename T>
auto Cast(const PCMetaObject& p) { return Cast<T>(p.get()); }
template <typename T>
auto CastChecked(const PCMetaObject& p) { return Cast<T>(p.get()); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::EObjectFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EObjectFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
