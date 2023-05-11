// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "vstools.h"

#if USE_PPE_EXTERNAL_VSTOOLS
// Custom native ETW heap events
// https://docs.microsoft.com/fr-fr/visualstudio/profiling/custom-native-etw-heap-events?view=vs-2019
#include <VSCustomNativeHeapEtwProvider.h> // will a static dependency to VSCustomNativeHeapEtwProvider.lib
#endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class VSToolsInterface final : public IVSTools {
public:
#if USE_PPE_EXTERNAL_VSTOOLS
    bool Available() const noexcept override {
        return true;
    }
    FHeapTracker CreateHeapTracker(const char* name) override{
        return ::OpenHeapTracker(name);
    }
    void DestroyHeapTracker(FHeapTracker heap) override{
        ::CloseHeapTracker(heap);
    }
    bool IsValidHeapTracker(FHeapTracker heap) const NOEXCEPT override{
        return ((!!heap) & (heap != INVALID_VSHEAPTRACK_HANDLE_VALUE));
    }
    bool AllocateEvent(FHeapTracker heap, void* ptr, size_t sz) override{
        return (::VSHeapTrackerAllocateEvent(heap, ptr, (unsigned long)sz) == 0);
    }
    bool ReallocateEvent(FHeapTracker heap, void* newp, size_t newsz, void* oldp) override{
        return (::VSHeapTrackerReallocateEvent(heap, newp, (unsigned long)newsz, oldp) == 0);
    }
    bool DeallocateEvent(FHeapTracker heap, void* ptr) override{
        return (::VSHeapTrackerDeallocateEvent(heap, ptr) == 0);
    }
#else
    bool Available() const noexcept override {
        return false;
    }
    FHeapTracker CreateHeapTracker(const char* ) override {
        return nullptr;
    }
    void DestroyHeapTracker(FHeapTracker ) override {
    }
    bool IsValidHeapTracker(FHeapTracker ) const NOEXCEPT override {
        return false;
    }
    bool AllocateEvent(FHeapTracker , void* , size_t ) override {
        return false;
    }
    bool ReallocateEvent(FHeapTracker , void* , size_t , void* ) override {
        return false;
    }
    bool DeallocateEvent(FHeapTracker , void* ) override {
        return false;
    }
#endif //!USE_PPE_EXTERNAL_VSTOOLS
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern "C" {
IVSTools* GetVSToolsInterface() NOEXCEPT {
    static VSToolsInterface GVSTools;
    return &GVSTools;
}
} //!extern "C"
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
