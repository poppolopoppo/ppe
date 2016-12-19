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

public:
    FMetaObject();
    virtual ~FMetaObject();

    FMetaObject(const FMetaObject&) = delete;
    FMetaObject& operator =(const FMetaObject&) = delete;

    const FName& RTTI_Name() const { return _name; }

    bool RTTI_IsLoaded() const { return Loaded == (_state & Loaded); }
    bool RTTI_IsUnloaded() const { return Unloaded == (_state & Unloaded); }
    bool RTTI_IsExported() const { return Exported == (_state & Exported); }

    void RTTI_Export(const FName& name);
    void RTTI_Unexport();

    virtual void RTTI_Load(FMetaLoadContext *context);
    virtual void RTTI_Unload(FMetaUnloadContext *context);

    void RTTI_CallLoadIFN(FMetaLoadContext *context);
    void RTTI_CallUnloadIFN(FMetaUnloadContext *context);

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const;
#endif

    class FMetaClass: public TDefaultMetaClass<FMetaObject> {
    public:
        typedef FMetaObject object_type;
        typedef void parent_type;

        FMetaClass();
        virtual ~FMetaClass();

        static void Create();
        static void Destroy();

        static bool HasInstance();
        static const FMetaClass*Instance();
    };

    virtual const RTTI::FMetaClass *RTTI_MetaClass() const {
        return FMetaObject::FMetaClass::Instance();
    }

    template <typename T> bool RTTI_InheritsFrom() const { return RTTI_MetaClass()->InheritsFrom<T>(); }
    template <typename T> bool RTTI_IsAssignableFrom() const { return RTTI_MetaClass()->IsAssignableFrom<T>(); }

private:
    FName _name;
    mutable EFlags _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
