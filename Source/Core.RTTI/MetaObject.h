#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObjectName.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaClass;
FWD_REFPTR(MetaObject);
class MetaLoadContext;
class MetaUnloadContext;
//----------------------------------------------------------------------------
class MetaObject : public RefCountable {
public:
    enum Flags : size_t {
        None        = 0,
        Loaded      = 1<<0,
        Unloaded    = 1<<1,
        Exported    = 1<<2,
    };

public:
    MetaObject();
    virtual ~MetaObject();

    MetaObject(const MetaObject&) = delete;
    MetaObject& operator =(const MetaObject&) = delete;

    const MetaObjectName& RTTI_Name() const { return _name; }

    bool RTTI_IsLoaded() const { return Loaded == (_state & Loaded); }
    bool RTTI_IsUnloaded() const { return Unloaded == (_state & Unloaded); }
    bool RTTI_IsExported() const { return Exported == (_state & Exported); }

    void RTTI_Export(const MetaObjectName& name);
    void RTTI_Unexport();

    virtual void RTTI_Load(MetaLoadContext *context);
    virtual void RTTI_Unload(MetaUnloadContext *context);

    void RTTI_CallLoadIFN(MetaLoadContext *context);
    void RTTI_CallUnloadIFN(MetaUnloadContext *context);

    class MetaClass : public DefaultMetaClass<MetaObject> {
    public:
        typedef MetaObject object_type;
        typedef void parent_type;

        MetaClass();
        virtual ~MetaClass();

        static void Create();
        static void Destroy();

        static bool HasInstance();
        static const MetaClass *Instance();
    };

    virtual const RTTI::MetaClass *RTTI_MetaClass() const {
        return MetaObject::MetaClass::Instance();
    }

private:
    MetaObjectName _name;
    Flags _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
