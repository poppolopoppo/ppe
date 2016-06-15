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
class MipMapChain : public RefCountable {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxCount, 16);

    MipMapChain();
    ~MipMapChain();

    MipMapChain(const MipMapChain& ) = delete;
    MipMapChain& operator =(const MipMapChain& ) = delete;

    bool empty() const { return _chain.empty(); }
    size_t size() const { return _chain.size(); }

    void Generate(FloatImage* topMip, bool hq = false);
    void PreserveAlphaTestCoverage(float cutoff);

    MemoryView<const PFloatImage> MakeView() const { return _chain.MakeView(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FixedSizeStack<PFloatImage, MaxCount> _chain;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
