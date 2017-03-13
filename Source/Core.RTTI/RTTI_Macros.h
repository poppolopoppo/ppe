#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTI_fwd.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_HEADER(_Name, _Parent) \
public: \
    typedef _Parent RTTI_parent_type; \
    \
    virtual const Core::RTTI::FMetaClass *RTTI_MetaClass() const override { \
        return _Name::FMetaClass::Instance(); \
    } \
    \
    class FMetaClass : public Core::RTTI::TInScopeMetaClass<_Name> { \
        friend class Core::RTTI::TInScopeMetaClass<_Name>; \
        typedef Core::RTTI::TInScopeMetaClass<_Name> metaclass_type; \
        \
    public: \
        typedef _Name object_type; \
        typedef _Parent parent_type; \
        \
        using metaclass_type::HasInstance; \
        using metaclass_type::Instance; \
        using metaclass_type::Handle; \
        \
        static Core::RTTI::FMetaNamespace& Namespace(); \
        \
    private: \
        FMetaClass(Core::RTTI::FMetaClassGuid guid, const Core::RTTI::FMetaNamespace* metaNamespace); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
