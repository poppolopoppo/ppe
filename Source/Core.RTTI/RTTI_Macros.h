#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTI_fwd.h"
#include "Core.RTTI/MetaClass.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_HEADER(_Name, _Parent) \
public: \
    typedef _Parent RTTI_parent_type; \
    \
    virtual const Core::RTTI::FMetaClass *RTTI_Class() const override { \
        return RTTI_FMetaClass::Get(); \
    } \
    \
    class RTTI_FMetaClass : public Core::RTTI::TInScopeMetaClass<_Name> { \
        friend class Core::RTTI::TInScopeMetaClass<_Name>; \
        typedef Core::RTTI::TInScopeMetaClass<_Name> metaclass_type; \
        \
    public: \
        typedef _Name object_type; \
        typedef _Parent parent_type; \
        \
        using metaclass_type::Get; \
        \
        static Core::RTTI::FMetaNamespace& Namespace(); \
        \
    private: \
        RTTI_FMetaClass(Core::RTTI::FClassId id, const Core::RTTI::FMetaNamespace* metaNamespace); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
