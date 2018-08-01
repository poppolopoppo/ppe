#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Stack.h"

namespace Core {
namespace Pixmap {
FWD_REFPTR(FloatImage);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MipMapChain);
class FMipMapChain : public FRefCountable {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxCount, 16);

    FMipMapChain();
    ~FMipMapChain();

    FMipMapChain(const FMipMapChain& ) = delete;
    FMipMapChain& operator =(const FMipMapChain& ) = delete;

    bool empty() const { return _chain.empty(); }
    size_t size() const { return _chain.size(); }

    void Generate(FFloatImage* topMip, bool hq = false);
    void PreserveAlphaTestCoverage(float cutoff);

    TMemoryView<const PFloatImage> MakeView() const { return _chain.MakeView(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TFixedSizeStack<PFloatImage, MaxCount> _chain;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
