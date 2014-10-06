#pragma once

#include "Core.h"

#include "MetaClass.h"
#include "MetaObject.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RTTI_CLASS_HEADER(_Name, _Parent) \
public: \
    virtual const Core::RTTI::MetaClass *RTTI_MetaClass() const override { \
        return _Name::MetaClass::Instance(); \
    } \
    \
    class MetaClass : public Core::RTTI::MetaClass { \
    public: \
        typedef _Name object_type; \
        typedef _Parent parent_type; \
        \
        MetaClass(); \
        virtual ~MetaClass(); \
        \
        virtual Core::RTTI::MetaObject *CreateInstance() const override; \
        \
        static void Create(); \
        static void Destroy(); \
        \
        static bool HasInstance(); \
        static const MetaClass *Instance(); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
