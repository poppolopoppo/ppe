#pragma once

#include "Core.RTTI/MetaClass.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Visitor/* = void(*)(const FMetaClass* metaClass, const FMetaFunction* func) */>
void ForEachFunction(const FMetaClass* metaClass, const _Visitor& visitor) {
    while (nullptr != metaClass) {
        for (const UCMetaFunction& func : metaClass->Functions()) {
            Assert(func);
            visitor(metaClass, func.get());
        }
        metaClass = metaClass->Parent();
    }
}
//----------------------------------------------------------------------------
template <typename _Pred/* = bool(*)(const FMetaClass* metaClass, const FMetaFunction* func) */>
const FMetaFunction* FindFunction(const FMetaClass* metaClass, const _Pred& pred) {
    while (nullptr != metaClass) {
        for (const UCMetaFunction& func : metaClass->Functions()) {
            Assert(func);
            if (pred(metaClass, func.get()))
                return func.get();
        }
        metaClass = metaClass->Parent();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
template <typename _Visitor/* = void (*)(const FMetaClass* metaClass, const FMetaProperty* prop) */>
void ForEachProperty(const FMetaClass* metaClass, const _Visitor& visitor) {
    while (nullptr != metaClass) {
        for (const UCMetaProperty& prop : metaClass->Properties()) {
            Assert(prop);
            visitor(metaClass, prop.get());
        }
        metaClass = metaClass->Parent();
    }
}
//----------------------------------------------------------------------------
template <typename _Pred/* = bool (*)(const FMetaClass* metaClass, const FMetaProperty* prop) */>
const FMetaProperty* FindProperty(const FMetaClass* metaClass, const _Pred& pred) {
    while (nullptr != metaClass) {
        for (const UCMetaProperty& prop : metaClass->Properties()) {
            Assert(prop);
            if (pred(metaClass, prop.get()))
                return prop.get();
        }
        metaClass = metaClass->Parent();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
