#pragma once

#include "RTTI_fwd.h"

#include "MetaClassHelpers.h"
#include "MetaEnumHelpers.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_STRUCT_DECL(_Api, _Name) \
    CONSTEXPR ::PPE::RTTI::PTypeInfos TypeInfos(::PPE::RTTI::TType< _Name > t) { \
        return ::PPE::RTTI::StructInfos(t); \
    } \
    _Api ::PPE::RTTI::PTypeTraits Traits(::PPE::RTTI::TType< _Name >) NOEXCEPT
//----------------------------------------------------------------------------
#define RTTI_CLASS_HEADER(_Api, _Name, _Parent) \
public: \
    using RTTI_parent_type = _Parent; \
    \
    virtual const ::PPE::RTTI::FMetaClass *RTTI_Class() const NOEXCEPT override { \
        return RTTI_FMetaClass::Get(); \
    } \
    \
    class _Api RTTI_FMetaClass : public ::PPE::RTTI::TInScopeMetaClass<RTTI_FMetaClass, _Name> { \
        friend class ::PPE::RTTI::TInScopeMetaClass<RTTI_FMetaClass, _Name>; \
        using metaclass_type = ::PPE::RTTI::TInScopeMetaClass<RTTI_FMetaClass, _Name>; \
        static const RTTI_FMetaClassHandle& metaclass_handle() NOEXCEPT; \
        \
    public: \
        using object_type = _Name; \
        using parent_type = _Parent; \
        \
        using metaclass_type::Get; \
        \
        static ::PPE::RTTI::FMetaModule& Module() NOEXCEPT; \
        \
    private: \
        RTTI_FMetaClass(::PPE::RTTI::FClassId id, const ::PPE::RTTI::FMetaModule* module) NOEXCEPT; \
    }
//----------------------------------------------------------------------------
#define RTTI_ENUM_HEADER(_Api, _Name) \
    class _Api CONCAT(RTTI_, _Name) : public ::PPE::RTTI::TInScopeMetaEnum<CONCAT(RTTI_, _Name), _Name> { \
        friend class ::PPE::RTTI::TInScopeMetaEnum<CONCAT(RTTI_, _Name), _Name>; \
        using metaenum_type = ::PPE::RTTI::TInScopeMetaEnum<CONCAT(RTTI_, _Name), _Name>; \
        static const RTTI_FMetaEnumHandle& metaenum_handle() NOEXCEPT; \
        \
    public: \
        using metaenum_type::Get; \
        static ::PPE::RTTI::FMetaModule& Module() NOEXCEPT; \
        \
    private: \
        explicit CONCAT(RTTI_, _Name)(const PPE::RTTI::FMetaModule* module) NOEXCEPT; \
    }; \
    _Api const ::PPE::RTTI::FMetaEnum* RTTI_Enum(_Name) NOEXCEPT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
