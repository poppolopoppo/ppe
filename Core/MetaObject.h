#pragma once

#include "Core.h"
#include "RefPtr.h"

#include "MetaObjectName.h"

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
    };

    MetaObject();
    virtual ~MetaObject();

    MetaObject(const MetaObject&) = delete;
    MetaObject& operator =(const MetaObject&) = delete;

    const MetaObjectName& RTTI_Name() const { return _name; }

    void RTTI_Export(const MetaObjectName& name);
    void RTTI_Unexport();

    virtual void RTTI_Load(MetaLoadContext *context);
    virtual void RTTI_Unload(MetaUnloadContext *context);

    void RTTI_CallLoadIFN(MetaLoadContext *context);
    void RTTI_CallUnloadIFN(MetaUnloadContext *context);

    virtual const RTTI::MetaClass *RTTI_MetaClass() const = 0;

private:
    MetaObjectName _name;
    Flags _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
