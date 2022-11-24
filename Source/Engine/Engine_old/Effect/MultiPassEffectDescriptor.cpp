// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MultiPassEffectDescriptor.h"

#include "EffectDescriptor.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FMultiPassEffectDescriptor, );
//----------------------------------------------------------------------------
FMultiPassEffectDescriptor::FMultiPassEffectDescriptor()
:   _size(0) {}
//----------------------------------------------------------------------------
FMultiPassEffectDescriptor::~FMultiPassEffectDescriptor() {}
//----------------------------------------------------------------------------
void FMultiPassEffectDescriptor::AddPass(const FEffectDescriptor *pass) {
    Assert(pass);
    AssertRelease(_size < MaxPassCount);

    _passes[_size++] = pass;
}
//----------------------------------------------------------------------------
size_t FMultiPassEffectDescriptor::FillEffectPasses(const FEffectDescriptor **pOutPasses, const size_t capacity) const {
    Assert(pOutPasses);
    Assert(capacity >= _size);
    Assert(_size > 0);

    forrange(i, 0, _size)
        pOutPasses[i] = _passes[i].get();

    return _size;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
