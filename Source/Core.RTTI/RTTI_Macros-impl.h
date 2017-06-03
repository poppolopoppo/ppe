#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTI_Macros.h"

#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypePromote.h"
#include "Core.RTTI/MetaTypeTraits.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaNamespace.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaFunction.h"
#include "Core.RTTI/MetaProperty.h"

#include "Core.RTTI/RTTI_extern.h"

#include <type_traits>

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_BEGIN(_Namespace, _Name, _Attributes) \
    Core::RTTI::FMetaNamespace& _Name::FMetaClass::Namespace() { \
        return RTTI_NAMESPACE(_Namespace); \
    } \
    \
    _Name::FMetaClass::FMetaClass(Core::RTTI::FMetaClassGuid guid, const Core::RTTI::FMetaNamespace* metaNamespace) \
        : metaclass_type(guid, (_Attributes), Core::RTTI::FName(STRINGIZE(_Name)), metaNamespace) { \
//----------------------------------------------------------------------------
#define RTTI_CLASS_END() }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Internal helper
#define _RTTI_FUNCTION_IMPL(_Name, _Flags, _Args) \
     RegisterFunction(MakeUnique<const Core::RTTI::FMetaFunction>(Core::RTTI::MakeFunction(Core::RTTI::FName(_Name), _Flags, _Args)));
//----------------------------------------------------------------------------
#define RTTI_FUNCTION(_Name) \
    _RTTI_FUNCTION_IMPL(STRINGIZE(_Name), Core::RTTI::FMetaFunction::Public, &object_type::_Name )
//----------------------------------------------------------------------------
#define RTTI_FUNCTION_PRIVATE(_Name) \
    _RTTI_FUNCTION_IMPL(STRINGIZE(_Name), Core::RTTI::FMetaFunction::Private, &object_type::_Name )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Internal helper
#define _RTTI_PROPERTY_IMPL(_Name, _Flags, _Args) \
    RegisterProperty(MakeUnique<const Core::RTTI::FMetaProperty>(Core::RTTI::MakeProperty(Core::RTTI::FName(_Name), _Flags, _Args )));
//----------------------------------------------------------------------------
// Add a public property "Alias" from a private field "_someName"
#define RTTI_PROPERTY_FIELD_ALIAS(_Name, _Alias) \
    _RTTI_PROPERTY_IMPL(STRINGIZE(_Alias), Core::RTTI::FMetaProperty::Public, &object_type::_Name )
//----------------------------------------------------------------------------
// Add a private property "SomeName" from a private field "_someName"
#define RTTI_PROPERTY_PRIVATE_FIELD(_Name) do { \
        char propName[] = STRINGIZE(_Name); \
        STATIC_ASSERT(sizeof(propName) > 1); \
        InplaceToUpper(propName[1]); \
        _RTTI_PROPERTY_IMPL(&propName[1], Core::RTTI::FMetaProperty::Private, &object_type::_Name ) \
    } while (0);
//----------------------------------------------------------------------------
// Add a public property "SomeName" from a public field "SomeName"
#define RTTI_PROPERTY_PUBLIC_FIELD(_Name) \
        _RTTI_PROPERTY_IMPL(STRINGIZE(_Name), Core::RTTI::FMetaProperty::Public, &object_type::_Name)
//----------------------------------------------------------------------------
// Add a public property "SomeName" from 3 delegates or 3 std::function : a getter, a mover & a setter
#define RTTI_PROPERTY_GETSET_FLAGS(_Name, _Flags, _Get, _Set) \
    _RTTI_PROPERTY_IMPL(STRINGIZE(_Name), _Flags, COMMA_PROTECT(std::move(_Get), std::move(_Set)) )
//----------------------------------------------------------------------------
// Add a property "SomeName" from 3 delegates or 3 std::function : a getter, a mover & a setter
#define RTTI_PROPERTY_GETSET(_Name, _Get, _Set) \
    RTTI_PROPERTY_GETSET_FLAGS(_Name, Core::RTTI::FMetaProperty::Public, _Get, _Set )
//----------------------------------------------------------------------------
// Add a property "SomeName" from 2 member functions : SomeName() & SetSomeName()
#define RTTI_PROPERTY_MEMBER(_Name) \
    RTTI_PROPERTY_GETSET(_Name, \
        &object_type::_Name, \
        &object_type::CONCAT(Set,  _Name)  )
//----------------------------------------------------------------------------
// Add a deprecated property "SomeName" of type T, these are write-only : you can't read from them
#define RTTI_PROPERTY_DEPRECATED(_Type, _Name) \
    RegisterProperty(MakeUnique<const Core::RTTI::FMetaProperty>( \
        Core::RTTI::MakeDeprecatedProperty<_Type, object_type>( \
            Core::RTTI::FName(STRINGIZE(_Name)), \
            Core::RTTI::FMetaProperty::Private  \
        )));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
