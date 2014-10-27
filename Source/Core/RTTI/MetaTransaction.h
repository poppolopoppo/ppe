#pragma once

#include "Core/Core.h"

#include "Core/Container/Tuple.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"

#include "Core/RTTI/Class/MetaClassName.h"
#include "Core/RTTI/Object/MetaObjectName.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
class MetaClass;
//----------------------------------------------------------------------------
class MetaTransaction : public RefCountable {
public:
    MetaTransaction();
    ~MetaTransaction();

    MetaTransaction(const MetaTransaction&) = delete;
    MetaTransaction& operator =(const MetaTransaction&) = delete;

    void Export(const MetaClassName& name, const MetaClass *metaclass, bool allowOverride);
    void Remove(const MetaClassName& name, const MetaClass *metaclass);

    void Export(const MetaObjectName& name, MetaAtom *atom, bool allowOverride);
    void Remove(const MetaObjectName& name, MetaAtom *atom);

    void Clear();

private:
    struct MetaClassBinding {
        MetaClassName   Name;
        const MetaClass *Old;
        const MetaClass *New;
    };

    struct MetaAtomBinding {
        MetaObjectName  Name;
        PMetaAtom       Old;
        PMetaAtom       New;
    };

    VECTOR(RTTI, MetaClassBinding) _classes;
    VECTOR(RTTI, MetaAtomBinding) _atoms;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
