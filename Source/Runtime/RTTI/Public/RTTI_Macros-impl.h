#pragma once

#include "RTTI.h"

#include "RTTI_Macros.h"

#include "Atom.h"
#include "MetaClass.h"
#include "MetaNamespace.h"
#include "MetaObject.h"
#include "MetaFunction.h"
#include "MetaProperty.h"
#include "NativeTypes.h"

#include "IO/StringView.h"

#include <type_traits>

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_BEGIN(_Namespace, _Name, _Attributes) \
    PPE::RTTI::FMetaNamespace& _Name::RTTI_FMetaClass::Namespace() { \
        return RTTI_NAMESPACE(_Namespace); \
    } \
    \
    _Name::RTTI_FMetaClass::RTTI_FMetaClass(PPE::RTTI::FClassId id, const PPE::RTTI::FMetaNamespace* metaNamespace) \
        : metaclass_type(id, PPE::RTTI::FName(STRINGIZE(_Name)), (_Attributes), metaNamespace) { \
//----------------------------------------------------------------------------
#define RTTI_CLASS_END() }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Internal helper
#define _RTTI_FUNCTION_IMPL(_Name, _Flags, _Func, ...) \
    RegisterFunction(PPE::RTTI::TMakeFunction<decltype(_Func)>::Make<_Func>(PPE::RTTI::FName(_Name), (_Flags), { PP_FOREACH_ARGS(STRINGIZE, __VA_ARGS__) }))
//----------------------------------------------------------------------------
// Add a public function
#define RTTI_FUNCTION(_Name, ...) \
    _RTTI_FUNCTION_IMPL(STRINGIZE(_Name), PPE::RTTI::EFunctionFlags::Public, &object_type::_Name, __VA_ARGS__);
//----------------------------------------------------------------------------
// Add a private function
#define RTTI_FUNCTION_PRIVATE(_Name, ...) \
    _RTTI_FUNCTION_IMPL(STRINGIZE(_Name), PPE::RTTI::EFunctionFlags::Private, &object_type::_Name, __VA_ARGS__);
//----------------------------------------------------------------------------
// Add a deprecated function
#define RTTI_FUNCTION_DEPRECATED(_Name, ...) \
    _RTTI_FUNCTION_IMPL(STRINGIZE(_Name), PPE::RTTI::EFunctionFlags::Private + PPE::RTTI::EFunctionFlags::Deprecated, &object_type::_Name, __VA_ARGS__);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Internal helper
#define _RTTI_PROPERTY_IMPL(_Name, _Flags, ...) \
    RegisterProperty(PPE::RTTI::MakeProperty(PPE::RTTI::FName(_Name), _Flags, __VA_ARGS__))
//----------------------------------------------------------------------------
// Add a public property "Alias" from a field "_someName"
#define RTTI_PROPERTY_FIELD_ALIAS_FLAGS(_Name, _Alias, _Flags) \
    _RTTI_PROPERTY_IMPL(STRINGIZE(_Alias), (_Flags), &object_type::_Name);
#define RTTI_PROPERTY_FIELD_ALIAS(_Name, _Alias) RTTI_PROPERTY_FIELD_ALIAS_FLAGS(_Name, _Alias, PPE::RTTI::EPropertyFlags::Public)
#define RTTI_PROPERTY_DEPRECATED_ALIAS(_Name, _Alias) RTTI_PROPERTY_FIELD_ALIAS_FLAGS(_Name, _Alias, PPE::RTTI::EPropertyFlags::Public + PPE::RTTI::EPropertyFlags::Deprecated)
//----------------------------------------------------------------------------
// Add a private property "SomeName" from a private field "_someName"
#define _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, _Flags) do { \
        char propName[] = STRINGIZE(_Name); \
        STATIC_ASSERT(sizeof(propName) > 2 * sizeof(char)); \
        Assert_NoAssume(propName[0] == '_'); \
        Assert_NoAssume(IsAlpha(propName[1])); \
        InplaceToUpper(propName[1]); \
        const FStringView capitalizedWithoutUnderscore(&propName[1], lengthof(propName) - 2); \
        _RTTI_PROPERTY_IMPL(capitalizedWithoutUnderscore, (_Flags), &object_type::_Name); \
    } while (0)
#define RTTI_PROPERTY_PRIVATE_FIELD(_Name) _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, PPE::RTTI::EPropertyFlags::Private);
#define RTTI_PROPERTY_PRIVATE_DEPRECATED(_Name) _RTTI_PROPERTY_PRIVATE_FIELD_IMPL(_Name, PPE::RTTI::EPropertyFlags::Private + PPE::RTTI::EPropertyFlags::Deprecated);
//----------------------------------------------------------------------------
// Add a public property "SomeName" from a public field "SomeName"
#define RTTI_PROPERTY_PUBLIC_FIELD(_Name) RTTI_PROPERTY_FIELD_ALIAS(_Name, _Name);
#define RTTI_PROPERTY_DEPRECATED_FIELD(_Name) RTTI_PROPERTY_DEPRECATED_ALIAS(_Name, _Name);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
