#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
FWD_REFPTR(EffectDescriptor);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IEffectPasses : public RefCountable {
public:
    virtual ~IEffectPasses() {}

    virtual size_t FillEffectPasses(const EffectDescriptor **pOutPasses, const size_t capacity) const = 0;

    template <size_t _Dim>
    size_t FillEffectPasses(const EffectDescriptor *(&pOutPasses)[_Dim]) const {
        return FillEffectPasses(pOutPasses, _Dim);
    }
};
//----------------------------------------------------------------------------
typedef Core::RefPtr<IEffectPasses> PEffectPasses;
typedef Core::RefPtr<const IEffectPasses> PCEffectPasses;
typedef Core::SafePtr<IEffectPasses> SEffectPasses;
typedef Core::SafePtr<const IEffectPasses> SCEffectPasses;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
