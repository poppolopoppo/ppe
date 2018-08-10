#pragma once

#include "RTTI.h"

#include "RTTI_fwd.h"
#include "MetaClass.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_HEADER(_Name, _Parent) \
public: \
    typedef _Parent RTTI_parent_type; \
    \
    virtual const PPE::RTTI::FMetaClass *RTTI_Class() const override { \
        return RTTI_FMetaClass::Get(); \
    } \
    \
    class RTTI_FMetaClass : public PPE::RTTI::TInScopeMetaClass<_Name> { \
        friend class PPE::RTTI::TInScopeMetaClass<_Name>; \
        typedef PPE::RTTI::TInScopeMetaClass<_Name> metaclass_type; \
        \
    public: \
        typedef _Name object_type; \
        typedef _Parent parent_type; \
        \
        using metaclass_type::Get; \
        \
        static PPE::RTTI::FMetaNamespace& Namespace(); \
        \
    private: \
        RTTI_FMetaClass(PPE::RTTI::FClassId id, const PPE::RTTI::FMetaNamespace* metaNamespace); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
