#pragma once

#include "RTTI.h"

#include "RTTI/Macros.h"

#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/TypeInfos.h"

#include "MetaClass.h"
#include "MetaFunction.h"
#include "MetaFunctionHelpers.h"
#include "MetaModule.h"
#include "MetaObject.h"
#include "MetaProperty.h"

#include "IO/StringView.h"
#include "IO/StaticString.h"

#include <type_traits>

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Internal
//----------------------------------------------------------------------------
namespace PPE::RTTI::details {
template <typename _Char, size_t _Capacity>
CONSTEXPR auto NativeClassName_ToRTTI_(const _Char(&name)[_Capacity]) {
    return MakeStringView(name).ShiftFront(/* F */);
}
template <typename _Char, size_t _Capacity>
CONSTEXPR auto PrivateFieldName_ToPublic_(const _Char (&name)[_Capacity]) {
    TBasicStaticString<_Char, _Capacity> result;
    result.Assign(MakeStringView(name).ShiftFront(/* _ */));
    result[0] = ToUpper(result[0]);
    return result;
}
}
//----------------------------------------------------------------------------
// Facet
//----------------------------------------------------------------------------
// internal helpers
#define _RTTI_FACET_APPEND_VA_ARGS(_Meta, ...) ((_Meta).Facets().Append(__VA_ARGS__))
//----------------------------------------------------------------------------
// Struct
//----------------------------------------------------------------------------
#define RTTI_STRUCT_DEF(_Api, _Name) \
    _Api ::PPE::RTTI::PTypeTraits RTTI_Traits(::PPE::RTTI::TTypeTag< _Name > tag) NOEXCEPT { \
        return ::PPE::RTTI::StructTraits(tag); \
    }
//----------------------------------------------------------------------------
// Class
//----------------------------------------------------------------------------
#define _RTTI_COMBINE_CLASSFLAGS_IMPL(_Attribute) ::PPE::RTTI::EClassFlags::_Attribute |
#define RTTI_CLASS_BEGIN(_Module, _Name, ...) \
    static const _Name::RTTI_FMetaClass::RTTI_FMetaClassHandle CONCAT(GRTTI_MetaClass_, _Name); \
    auto _Name::RTTI_FMetaClass::metaclass_handle() NOEXCEPT -> const RTTI_FMetaClassHandle& { \
        return CONCAT(GRTTI_MetaClass_, _Name); \
    } \
    \
    ::PPE::RTTI::FMetaModule& _Name::RTTI_FMetaClass::Module() NOEXCEPT { \
        return RTTI_MODULE(_Module); \
    } \
    \
    _Name::RTTI_FMetaClass::RTTI_FMetaClass(::PPE::RTTI::FClassId id, const ::PPE::RTTI::FMetaModule* metaModule) NOEXCEPT \
    :   metaclass_type(id, ::PPE::RTTI::FName(::PPE::RTTI::details::NativeClassName_ToRTTI_(STRINGIZE(_Name))), \
            (PP_FOREACH(_RTTI_COMBINE_CLASSFLAGS_IMPL, __VA_ARGS__) ::PPE::RTTI::EClassFlags::None), \
            metaModule ) {
//----------------------------------------------------------------------------
#define RTTI_CLASS_FACET(...) Facets().Add(__VA_ARGS__);
//----------------------------------------------------------------------------
#define RTTI_CLASS_END() }
//----------------------------------------------------------------------------
// Function
//----------------------------------------------------------------------------
// Internal helper
#define _RTTI_FUNCTION_IMPL(_Name, _Flags, _Func, ...) \
    RegisterFunction(::PPE::RTTI::MakeFunction<_Func>(::PPE::RTTI::FName(_Name), { PP_FOREACH_ARGS(STRINGIZE, __VA_ARGS__) }, (_Flags) ))
//----------------------------------------------------------------------------
// Add a public function
#define RTTI_FUNCTION(_Name, _Args) \
    _RTTI_FUNCTION_IMPL(STRINGIZE(_Name), ::PPE::RTTI::EFunctionFlags::Public, &object_type::_Name, EXPAND_VA _Args);
//----------------------------------------------------------------------------
// Add a public function with facets
#define RTTI_FUNCTION_FACET(_Name, _Args, ...) \
    _RTTI_FACET_APPEND_VA_ARGS(_RTTI_FUNCTION_IMPL(STRINGIZE(_Name), ::PPE::RTTI::EFunctionFlags::Public, &object_type::_Name, EXPAND_VA _Args),## __VA_ARGS__);
//----------------------------------------------------------------------------
// Add a private function
#define RTTI_FUNCTION_PRIVATE(_Name, _Args) \
    _RTTI_FUNCTION_IMPL(STRINGIZE(_Name), ::PPE::RTTI::EFunctionFlags::Private, &object_type::_Name, EXPAND_VA _Args);
//----------------------------------------------------------------------------
// Add a deprecated function
#define RTTI_FUNCTION_DEPRECATED(_Name, _Args) \
    _RTTI_FUNCTION_IMPL(STRINGIZE(_Name), ::PPE::RTTI::EFunctionFlags::Private + ::PPE::RTTI::EFunctionFlags::Deprecated, &object_type::_Name, EXPAND_VA _Args);
//----------------------------------------------------------------------------
// Add a public function with customization delegate
#define RTTI_FUNCTION_CUSTOM(_Name, _Args, ...) \
    (__VA_ARGS__)(_RTTI_FUNCTION_IMPL(STRINGIZE(_Name), ::PPE::RTTI::EFunctionFlags::Public, &object_type::_Name, EXPAND_VA _Args));
//----------------------------------------------------------------------------
// Property
//----------------------------------------------------------------------------
// Internal helper
#define _RTTI_PROPERTY_IMPL(_Name, _Flags, ...) \
    RegisterProperty(::PPE::RTTI::MakeProperty(::PPE::RTTI::FName(_Name), _Flags, ## __VA_ARGS__))
//----------------------------------------------------------------------------
// Add a public property "Alias" from a field "Any_name_"
#define RTTI_PROPERTY_FIELD_ALIAS_FLAGS(_Name, _Alias, _Flags) \
    _RTTI_PROPERTY_IMPL(STRINGIZE(_Alias), (_Flags), &object_type::_Name);
#define RTTI_PROPERTY_FIELD_ALIAS(_Name, _Alias) RTTI_PROPERTY_FIELD_ALIAS_FLAGS(_Name, _Alias, ::PPE::RTTI::EPropertyFlags::Public)
#define RTTI_PROPERTY_DEPRECATED_ALIAS(_Name, _Alias) RTTI_PROPERTY_FIELD_ALIAS_FLAGS(_Name, _Alias, ::PPE::RTTI::EPropertyFlags::Public + ::PPE::RTTI::EPropertyFlags::Deprecated)
#define RTTI_PROPERTY_READONLY_ALIAS(_Name, _Alias) RTTI_PROPERTY_FIELD_ALIAS_FLAGS(_Name, _Alias, ::PPE::RTTI::EPropertyFlags::Public + ::PPE::RTTI::EPropertyFlags::ReadOnly)
#define RTTI_PROPERTY_TRANSIENT_ALIAS(_Name, _Alias) RTTI_PROPERTY_FIELD_ALIAS_FLAGS(_Name, _Alias, ::PPE::RTTI::EPropertyFlags::Public + ::PPE::RTTI::EPropertyFlags::Transient)
//----------------------------------------------------------------------------
// Add a private property "SomeName" from a private field "_someName"
#define _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, _Flags) \
    _RTTI_PROPERTY_IMPL(::PPE::RTTI::details::PrivateFieldName_ToPublic_(STRINGIZE(_Name)), (_Flags), &object_type::_Name)
#define RTTI_PROPERTY_PRIVATE_FIELD(_Name) _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, ::PPE::RTTI::EPropertyFlags::Private);
#define RTTI_PROPERTY_PRIVATE_FACET(_Name, ...) _RTTI_FACET_APPEND_VA_ARGS(_RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, ::PPE::RTTI::EPropertyFlags::Private),## __VA_ARGS__);
#define RTTI_PROPERTY_PRIVATE_DEPRECATED(_Name) _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, ::PPE::RTTI::EPropertyFlags::Private + ::PPE::RTTI::EPropertyFlags::Deprecated);
#define RTTI_PROPERTY_PRIVATE_READONLY(_Name) _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, ::PPE::RTTI::EPropertyFlags::Private + ::PPE::RTTI::EPropertyFlags::ReadOnly);
#define RTTI_PROPERTY_PRIVATE_TRANSIENT(_Name) _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, ::PPE::RTTI::EPropertyFlags::Private + ::PPE::RTTI::EPropertyFlags::Transient);
//----------------------------------------------------------------------------
// Add a public property "SomeName" from a public field "SomeName"
#define RTTI_PROPERTY_PUBLIC_FIELD(_Name) RTTI_PROPERTY_FIELD_ALIAS(_Name, _Name);
#define RTTI_PROPERTY_PUBLIC_FACET(_Name, ...) _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(RTTI_PROPERTY_FIELD_ALIAS(_Name, _Name),## __VA_ARGS__);
#define RTTI_PROPERTY_DEPRECATED_FIELD(_Name) RTTI_PROPERTY_DEPRECATED_ALIAS(_Name, _Name);
#define RTTI_PROPERTY_READONLY_FIELD(_Name) RTTI_PROPERTY_READONLY_ALIAS(_Name, _Name);
#define RTTI_PROPERTY_TRANSIENT_FIELD(_Name) RTTI_PROPERTY_TRANSIENT_ALIAS(_Name, _Name);
//----------------------------------------------------------------------------
// Enum
//----------------------------------------------------------------------------
#define RTTI_ENUM_BEGIN_EX(_Module, _Name, _Attributes) \
    static const CONCAT(RTTI_, _Name)::RTTI_FMetaEnumHandle CONCAT(GRTTI_MetaEnum_, _Name); \
    auto CONCAT(RTTI_, _Name)::metaenum_handle() NOEXCEPT -> const RTTI_FMetaEnumHandle& { \
        return CONCAT(GRTTI_MetaEnum_, _Name); \
    } \
    \
    const ::PPE::RTTI::FMetaEnum* RTTI_Enum(_Name) NOEXCEPT { \
        return CONCAT(RTTI_, _Name)::Get(); \
    } \
    \
    ::PPE::RTTI::FMetaModule& CONCAT(RTTI_, _Name)::Module() NOEXCEPT { \
        return RTTI_MODULE(_Module); \
    } \
    \
    CONCAT(RTTI_, _Name)::CONCAT(RTTI_, _Name)(const ::PPE::RTTI::FMetaModule* metaNamespace) NOEXCEPT \
    :   metaenum_type(PPE::RTTI::FName(STRINGIZE(_Name)), (_Attributes), metaNamespace) { \
        using enum_type = _Name;
//----------------------------------------------------------------------------
#define RTTI_ENUM_BEGIN(_Module, _Name) \
    RTTI_ENUM_BEGIN_EX(_Module, _Name, ::PPE::RTTI::EEnumFlags::None)
//----------------------------------------------------------------------------
#define RTTI_ENUM_FLAGS_BEGIN(_Module, _Name) \
    RTTI_ENUM_BEGIN_EX(_Module, _Name, ::PPE::RTTI::EEnumFlags::Flags)
//----------------------------------------------------------------------------
#define RTTI_ENUM_FACET(...) Facets().Add(__VA_ARGS__);
//----------------------------------------------------------------------------
#define RTTI_ENUM_VALUE_EX(_Name, _Value) \
    RegisterValue(::PPE::RTTI::FMetaEnumValue{ _Name, i64(_Value) });
//----------------------------------------------------------------------------
#define RTTI_ENUM_VALUE(_Name) \
    RTTI_ENUM_VALUE_EX(::PPE::RTTI::FName(STRINGIZE(_Name)), enum_type::_Name)
//----------------------------------------------------------------------------
#define RTTI_ENUM_END() }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
