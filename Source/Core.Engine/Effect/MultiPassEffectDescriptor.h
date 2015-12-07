#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/IEffectPasses.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
FWD_REFPTR(EffectDescriptor);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MultiPassEffectDescriptor);
class MultiPassEffectDescriptor : public IEffectPasses {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxPassCount, 8);

    MultiPassEffectDescriptor();
    virtual ~MultiPassEffectDescriptor();

    MultiPassEffectDescriptor(const MemoryView<const PCEffectDescriptor>& passes);

    size_t size() const { return _size; }
    bool empty() const { return 0 == _size; }

    MemoryView<const PCEffectDescriptor> Passes() const { return MemoryView<const PCEffectDescriptor>(_passes, _size); }

    void AddPass(const EffectDescriptor *pass);

    virtual size_t FillEffectPasses(const EffectDescriptor **pOutPasses, const size_t capacity) const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    size_t _size;
    PCEffectDescriptor _passes[MaxPassCount];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
