#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
FWD_REFPTR(EffectDescriptor);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IEffectPasses : public FRefCountable {
public:
    virtual ~IEffectPasses() {}

    virtual size_t FillEffectPasses(const FEffectDescriptor **pOutPasses, const size_t capacity) const = 0;

    template <size_t _Dim>
    size_t FillEffectPasses(const FEffectDescriptor *(&pOutPasses)[_Dim]) const {
        return FillEffectPasses(pOutPasses, _Dim);
    }
};
//----------------------------------------------------------------------------
typedef PPE::TRefPtr<IEffectPasses> PEffectPasses;
typedef PPE::TRefPtr<const IEffectPasses> PCEffectPasses;
typedef PPE::TSafePtr<IEffectPasses> SEffectPasses;
typedef PPE::TSafePtr<const IEffectPasses> SCEffectPasses;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
