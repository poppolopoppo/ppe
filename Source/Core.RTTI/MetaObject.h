#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaType.h"

#include "Core/Memory/RefPtr.h"

#if !(defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
#   define WITH_RTTI_VERIFY_PREDICATES
#endif

#ifdef WITH_RTTI_VERIFY_PREDICATES
#   define RTTI_VerifyPredicate(_COND) AssertRelease(_COND)
#endif

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaClass;
class FMetaNamespace;
FWD_REFPTR(MetaObject);
class FMetaLoadContext;
class FMetaUnloadContext;
//----------------------------------------------------------------------------
class FMetaObject : public FRefCountable {
public:
    enum EFlags : size_t {
        None        = 0,
        Loaded      = 1<<0,
        Unloaded    = 1<<1,
        Exported    = 1<<2,
#ifdef WITH_RTTI_VERIFY_PREDICATES
        Verifying   = 1<<3,
#endif
    };
    ENUM_FLAGS_FRIEND(EFlags);

public:
    FMetaObject();
    virtual ~FMetaObject();

    FMetaObject(const FMetaObject&) = delete;
    FMetaObject& operator =(const FMetaObject&) = delete;

    const FName& RTTI_Name() const { return _name; }

    bool RTTI_IsLoaded() const   { return (_state ^ Loaded  ); }
    bool RTTI_IsUnloaded() const { return (_state ^ Unloaded); }
    bool RTTI_IsExported() const { return (_state ^ Exported); }

    void RTTI_Export(const FName& name);
    void RTTI_Unexport();

    virtual void RTTI_Load(FMetaLoadContext* context);
    virtual void RTTI_Unload(FMetaUnloadContext* context);

    void RTTI_CallLoadIFN(FMetaLoadContext* context);
    void RTTI_CallUnloadIFN(FMetaUnloadContext* context);

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const;
#endif

    class FMetaClass: public TInScopeMetaClass<FMetaObject> {
        friend class TInScopeMetaClass<FMetaObject>;
        typedef TInScopeMetaClass<FMetaObject> metaclass_type;
    public:
        typedef FMetaObject object_type;
        typedef void parent_type;
        using metaclass_type::HasInstance;
        using metaclass_type::Instance;
        using metaclass_type::Handle;
        static FMetaNamespace& Namespace();
    private:
        FMetaClass(FMetaClassGuid guid, const FMetaNamespace* metaNamespace);
    };

    virtual const RTTI::FMetaClass* RTTI_MetaClass() const {
        return FMetaObject::FMetaClass::Instance();
    }

    template <typename T> bool RTTI_InheritsFrom() const { return RTTI_MetaClass()->InheritsFrom<T>(); }
    template <typename T> bool RTTI_IsAssignableFrom() const { return RTTI_MetaClass()->IsAssignableFrom<T>(); }

    template <typename T> T* RTTI_Cast() { return (RTTI_MetaClass()->CastTo<T>() ? checked_cast<T*>(this) : nullptr); }
    template <typename T> const T* RTTI_Cast() const { return (RTTI_MetaClass()->CastTo<T>() ? checked_cast<const T*>(this) : nullptr); }

private:
    FName _name;
    mutable EFlags _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
