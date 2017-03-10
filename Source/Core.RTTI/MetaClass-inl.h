#pragma once

#include "Core.RTTI/MetaClass.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Visitor/* = void(*)(const FMetaClass* metaClass, const FMetaFunction* func) */>
void ForEachFunction(const FMetaClass* metaClass, const _Visitor& visitor) {
    Assert(metaClass);
    for (const FMetaFunction* func : metaClass->AllFunctions()) {
        Assert(func);
        visitor(metaClass, func);
    }
}
//----------------------------------------------------------------------------
template <typename _Pred/* = bool(*)(const FMetaClass* metaClass, const FMetaFunction* func) */>
const FMetaFunction* FindFunction(const FMetaClass* metaClass, const _Pred& pred) {
    Assert(metaClass);
    for (const FMetaFunction* func : metaClass->AllFunctions()) {
        Assert(func);
        if (pred(metaClass, func))
            return func;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
template <typename _Visitor/* = void (*)(const FMetaClass* metaClass, const FMetaProperty* prop) */>
void ForEachProperty(const FMetaClass* metaClass, const _Visitor& visitor) {
    Assert(metaClass);
    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        Assert(prop);
        visitor(metaClass, prop);
    }
}
//----------------------------------------------------------------------------
template <typename _Pred/* = bool (*)(const FMetaClass* metaClass, const FMetaProperty* prop) */>
const FMetaProperty* FindProperty(const FMetaClass* metaClass, const _Pred& pred) {
    Assert(metaClass);
    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        Assert(prop);
        if (pred(metaClass, prop))
            return prop;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
