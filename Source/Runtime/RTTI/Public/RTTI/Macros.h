#pragma once

#include "RTTI.h"

#include "RTTI_fwd.h"

#include "MetaClassHelpers.h"
#include "MetaEnumHelpers.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_HEADER(_Name, _Parent) \
public: \
    typedef _Parent RTTI_parent_type; \
    \
    virtual const ::PPE::RTTI::FMetaClass *RTTI_Class() const override { \
        return RTTI_FMetaClass::Get(); \
    } \
    \
    class RTTI_FMetaClass : public ::PPE::RTTI::TInScopeMetaClass<_Name> { \
        friend class ::PPE::RTTI::TInScopeMetaClass<_Name>; \
        typedef ::PPE::RTTI::TInScopeMetaClass<_Name> metaclass_type; \
        \
    public: \
        typedef _Name object_type; \
        typedef _Parent parent_type; \
        \
        using metaclass_type::Get; \
        \
        static ::PPE::RTTI::FMetaNamespace& Namespace(); \
        \
    private: \
        RTTI_FMetaClass(::PPE::RTTI::FClassId id, const ::PPE::RTTI::FMetaNamespace* metaNamespace); \
    }
//----------------------------------------------------------------------------
#define RTTI_ENUM_HEADER(_Api, _Name) \
    class _Api CONCAT(RTTI_, _Name) : public ::PPE::RTTI::TInScopeMetaEnum<_Name> { \
        friend class ::PPE::RTTI::TInScopeMetaEnum<_Name>; \
        typedef ::PPE::RTTI::TInScopeMetaEnum<_Name> metaenum_type; \
        \
    public: \
        using metaenum_type::Get; \
        static ::PPE::RTTI::FMetaNamespace& Namespace(); \
        \
    private: \
        explicit CONCAT(RTTI_, _Name)(const PPE::RTTI::FMetaNamespace* metaNamespace); \
    }; \
    _Api const CONCAT(RTTI_, _Name)* RTTI_Enum(_Name) NOEXCEPT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------