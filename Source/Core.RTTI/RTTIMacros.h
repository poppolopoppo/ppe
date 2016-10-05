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
    virtual const Core::RTTI::FMetaClass *RTTI_MetaClass() const override { \
        return _Name::FMetaClass::Instance(); \
    } \
    \
    class FMetaClass : public Core::RTTI::TDefaultMetaClass<_Name> { \
    public: \
        typedef _Name object_type; \
        typedef _Parent parent_type; \
        \
        FMetaClass(); \
        virtual ~FMetaClass(); \
        \
        static void Create(); \
        static void Destroy(); \
        \
        static bool HasInstance(); \
        static const FMetaClass *Instance(); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
