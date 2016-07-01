#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTIMacros.h"

#include "Core.RTTI/MetaTypePromote.h"
#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaAtomDatabase.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaClassDatabase.h"
#include "Core.RTTI/MetaClassSingleton.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaType.h"

#include "Core.RTTI/RTTI_extern.h"

#include <type_traits>

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define _RTTI_CLASS_SINGLETON(_Name) \
    void _Name::MetaClass::Create() { \
        Core::RTTI::MetaClassSingleton< _Name >::Create(); \
    } \
    void _Name::MetaClass::Destroy() { \
        Core::RTTI::MetaClassSingleton< _Name >::Destroy(); \
    } \
    bool _Name::MetaClass::HasInstance() { \
        return Core::RTTI::MetaClassSingleton< _Name >::HasInstance(); \
    } \
    const _Name::MetaClass *_Name::MetaClass::Instance() { \
        return &Core::RTTI::MetaClassSingleton< _Name >::Instance(); \
    }
//----------------------------------------------------------------------------
#define _RTTI_CLASS_DESTRUCTOR(_Name) \
    _Name::MetaClass::~MetaClass() {}
//----------------------------------------------------------------------------
#define _RTTI_CLASS_AUTOREGISTER(_Tag, _Name) \
    static const Core::RTTI::MetaClassDecl<RTTI_TAG(_Tag)> CONCAT(gRTTI_AutoRegister_, _Name)( \
        &_Name::MetaClass::Create, \
        &_Name::MetaClass::Destroy \
        );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_BEGIN(_Tag, _Name, _Attributes) \
    _RTTI_CLASS_SINGLETON(_Name) \
    _RTTI_CLASS_AUTOREGISTER(_Tag, _Name) \
    _RTTI_CLASS_DESTRUCTOR(_Name) \
    _Name::MetaClass::MetaClass() \
        :   Core::RTTI::DefaultMetaClass<_Name>( \
                STRINGIZE(_Name), \
                Core::RTTI::MetaClass::_Attributes ) {
//----------------------------------------------------------------------------
#define RTTI_CLASS_END() }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Internal helper
#define _RTTI_PROPERTY_IMPL(_Name, _Flags, _Args) { \
        Core::RTTI::UCMetaProperty prop(Core::RTTI::MakeProperty(_Name, _Flags, _Args )); \
        RegisterProperty(std::move(prop)); \
    }
//----------------------------------------------------------------------------
// Add a public property "Alias" from a private field "_someName"
#define RTTI_PROPERTY_FIELD_ALIAS(_Name, _Alias) \
    _RTTI_PROPERTY_IMPL(STRINGIZE(_Alias), Core::RTTI::MetaProperty::Public, &object_type::_Name )
//----------------------------------------------------------------------------
// Add a private property "SomeName" from a private field "_someName"
#define RTTI_PROPERTY_PRIVATE_FIELD(_Name) { \
        char propName[] = STRINGIZE(_Name); \
        STATIC_ASSERT(sizeof(propName) > 1); \
        InplaceToUpper(propName[1]); \
        _RTTI_PROPERTY_IMPL(&propName[1], Core::RTTI::MetaProperty::Private, &object_type::_Name ) \
    }
//----------------------------------------------------------------------------
// Add a public property "SomeName" from 3 delegates or 3 std::function : a getter, a mover & a setter
#define RTTI_PROPERTY_GETMOVESET_FLAGS(_Name, _Flags, _Get, _Move, _Set) \
    _RTTI_PROPERTY_IMPL(STRINGIZE(_Name), _Flags, COMMA_PROTECT(std::move(_Get), std::move(_Move), std::move(_Set)) )
//----------------------------------------------------------------------------
// Add a property "SomeName" from 3 delegates or 3 std::function : a getter, a mover & a setter
#define RTTI_PROPERTY_GETMOVESET(_Name, _Get, _Move, _Set) \
    RTTI_PROPERTY_GETMOVESET_FLAGS(_Name, Core::RTTI::MetaProperty::Public, _Get, _Move, _Set )
//----------------------------------------------------------------------------
// Add a property "SomeName" from 3 member functions : GetSomeName(), MoveSomeName() & SetSomeName()
#define RTTI_PROPERTY_MEMBER(_Name) \
    RTTI_PROPERTY_GETMOVESET(_Name, \
        &object_type::CONCAT(Get,  _Name), \
        &object_type::CONCAT(Move, _Name), \
        &object_type::CONCAT(Set,  _Name)  )
//----------------------------------------------------------------------------
// Add a deprecated property "SomeName" of type T, these are write-only : you can't read from them
#define RTTI_PROPERTY_DEPRECATED(_Type, _Name) { \
        Core::RTTI::UCMetaProperty prop = Core::RTTI::MakeDeprecatedProperty<_Type, object_type>( \
            STRINGIZE(_Name), Core::RTTI::MetaProperty::Private ); \
        RegisterProperty(std::move(prop)); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
