#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"
#include "Core.RTTI/MetaClass.h"

#include "Core/IO/TextWriter_fwd.h"
#include "Core/Memory/RefPtr.h"

#if !(defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
#   define WITH_RTTI_VERIFY_PREDICATES
#endif

#ifdef WITH_RTTI_VERIFY_PREDICATES
#   define RTTI_VerifyPredicate(...) AssertRelease(COMMA_PROTECT(__VA_ARGS__))
#endif

namespace Core {
namespace RTTI {
class FAtom;
class FMetaClass;
class FMetaNamespace;
FWD_REFPTR(MetaTransaction);
class FLoadContext;
class FUnloadContext;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EObjectFlags : u32 {
    None        = 0,
    Loaded      = 1<<0,
    Unloaded    = 1<<1,
    Exported    = 1<<2,
    TopObject   = 1<<3,
#ifdef WITH_RTTI_VERIFY_PREDICATES
    Verifying   = 1<<4,
#endif
};
ENUM_FLAGS(EObjectFlags);
//----------------------------------------------------------------------------
class ILoadContext {
public:
    virtual ~ILoadContext() {}
    virtual void OnLoadObject(FMetaObject& object) = 0;
};
//----------------------------------------------------------------------------
class IUnloadContext {
public:
    virtual ~IUnloadContext() {}
    virtual void OnUnloadObject(FMetaObject& object) = 0;
};
//----------------------------------------------------------------------------
FWD_REFPTR(MetaObject);
class CORE_RTTI_API FMetaObject : public FRefCountable {
public:
    FMetaObject();
    virtual ~FMetaObject();

    const FName& RTTI_Name() const  { return _name; }
    EObjectFlags RTTI_Flags() const { return _flags; }

    bool RTTI_IsLoaded() const      { return (_flags ^ EObjectFlags::Loaded     ); }
    bool RTTI_IsUnloaded() const    { return (_flags ^ EObjectFlags::Unloaded   ); }
    bool RTTI_IsExported() const    { return (_flags ^ EObjectFlags::Exported   ); }
    bool RTTI_IsTopObject() const   { return (_flags ^ EObjectFlags::TopObject  ); }

    const FMetaTransaction* RTTI_Outer() const { return _outer.get(); }
    void RTTI_SetOuter(const FMetaTransaction* outer, const FMetaTransaction* prevOuterForDbg = nullptr);

    bool RTTI_IsA(const FMetaClass& metaClass) const;
    bool RTTI_CastTo(const FMetaClass& metaClass) const;
    bool RTTI_InheritsFrom(const FMetaClass& metaClass) const;
    bool RTTI_IsAssignableFrom(const FMetaClass& metaClass) const;

    void RTTI_ResetToDefaultValue();

    void RTTI_Export(const FName& name);
    void RTTI_Unexport();

    virtual void RTTI_Load(ILoadContext& context);
    virtual void RTTI_Unload(IUnloadContext& context);

    bool RTTI_CallLoadIFN(ILoadContext& context);
    bool RTTI_CallUnloadIFN(IUnloadContext& context);

    void RTTI_MarkAsTopObject(); // should only be called by FMetaTransaction
    void RTTI_UnmarkAsTopObject(); // should only be called by FMetaTransaction

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const;
#endif

public: // Meta class
    class RTTI_FMetaClass : public TInScopeMetaClass<FMetaObject> {
        friend class TInScopeMetaClass<FMetaObject>;
        typedef TInScopeMetaClass<FMetaObject> metaclass_type;
    public:
        typedef FMetaObject object_type;
        typedef void parent_type;
        using metaclass_type::Instance;
        static FMetaNamespace& Namespace();
    private:
        RTTI_FMetaClass(FClassId id, const FMetaNamespace* metaNamespace);
    };

    virtual const RTTI::FMetaClass* RTTI_Class() const {
        return RTTI_FMetaClass::Instance();
    };

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::EObjectFlags flags);
CORE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EObjectFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
