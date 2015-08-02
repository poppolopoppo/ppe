#pragma once

#include "Core/Core.h"
#include "Core/Meta/Singleton.h"

#include "Core.RTTI/RTTIMacros.h"

#include "Core.RTTI/Type/MetaTypePromote.h"

#include "Core.RTTI/Atom/MetaAtom.h"
#include "Core.RTTI/Atom/MetaAtomDatabase.h"

#include "Core.RTTI/Class/MetaClass.h"
#include "Core.RTTI/Class/MetaClassDatabase.h"
#include "Core.RTTI/Class/MetaClassName.h"

#include "Core.RTTI/Object/MetaObject.h"
#include "Core.RTTI/Object/MetaObjectName.h"

#include "Core.RTTI/Property/MetaProperty.h"
#include "Core.RTTI/Property/MetaPropertyName.h"

#include <type_traits>

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_SINGLETON(_Name) \
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
#define RTTI_CLASS_CREATE_INSTANCE(_Name) \
    Core::RTTI::MetaObject *_Name::MetaClass::CreateInstance() const { \
        return new _Name(); \
    }
//----------------------------------------------------------------------------
#define RTTI_CLASS_DESTRUCTOR(_Name) \
    _Name::MetaClass::~MetaClass() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_BEGIN(_Name, _Attributes) \
    RTTI_CLASS_SINGLETON(_Name) \
    RTTI_CLASS_CREATE_INSTANCE(_Name) \
    RTTI_CLASS_DESTRUCTOR(_Name) \
    _Name::MetaClass::MetaClass() \
    :   Core::RTTI::MetaClass(  STRINGIZE(_Name), \
                                Core::RTTI::MetaClass::_Attributes, \
                                Core::RTTI::GetMetaClass<parent_type>()) {
//----------------------------------------------------------------------------
#define RTTI_CLASS_END() }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_PROPERTY_FIELD_ALIAS_IMPL(_Name, _Alias, _Flags) { \
        const Core::RTTI::MetaProperty *prop = MakeProperty(\
            STRINGIZE(_Alias), \
            _Flags, \
            &object_type::_Name )  \
        ; \
        _properties.Insert_AssertUnique(prop->Name(), prop); \
    }
//----------------------------------------------------------------------------
#define RTTI_PROPERTY_FIELD_ALIAS(_Name, _Alias) \
    RTTI_PROPERTY_FIELD_ALIAS_IMPL(_Name, _Alias, Core::RTTI::MetaProperty::Private)
//----------------------------------------------------------------------------
#define RTTI_PROPERTY_FIELD(_Name) \
    RTTI_PROPERTY_FIELD_ALIAS_IMPL(_Name, _Name, Core::RTTI::MetaProperty::Private)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_PROPERTY_FUNCTION(_Name, _Get, _Move, _Set) { \
        const Core::RTTI::MetaProperty *prop = MakeProperty( \
            STRINGIZE(_Name), \
            Core::RTTI::MetaProperty::Public, \
            std::move(_Get), std::move(_Move), std::move(_Set) ) \
        ; \
        _properties.Insert_AssertUnique(prop->Name(), prop); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MetaClassSingleton : Meta::Singleton<typename T::MetaClass, typename T::MetaClass> {
    typedef Meta::Singleton<typename T::MetaClass, typename T::MetaClass> parent_type;
public:
    using parent_type::HasInstance;
    using parent_type::Instance;

    static void Create() {
        parent_type::Create();
        parent_type::Instance().Register(MetaClassDatabase::Instance());
    }

    static void Destroy() {
        parent_type::Instance().Unregister(MetaClassDatabase::Instance());
        parent_type::Destroy();
    }
};
//----------------------------------------------------------------------------
template <typename T>
static const RTTI::MetaClass *GetMetaClass(typename std::enable_if< std::is_base_of<RTTI::MetaObject, T>::value >::type* = 0) {
    return &MetaClassSingleton<T>::Instance();
}
//----------------------------------------------------------------------------
template <typename T>
static const RTTI::MetaClass *GetMetaClass(typename std::enable_if< !std::is_base_of<RTTI::MetaObject, T>::value >::type* = 0) {
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
