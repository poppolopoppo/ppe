#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTI_fwd.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"

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
        static void Create(); \
        static void Destroy(); \
        \
        static bool HasInstance(); \
        static const MetaClass *Instance(); \
        \
    protected: \
        virtual Core::RTTI::MetaObject *VirtualCreateInstance() const override; \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
