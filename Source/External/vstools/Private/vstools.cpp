#include "vstools.h"

#if USE_PPE_EXTERNAL_VSTOOLS

// Custom native ETW heap events
// https://docs.microsoft.com/fr-fr/visualstudio/profiling/custom-native-etw-heap-events?view=vs-2019
#include <VSCustomNativeHeapEtwProvider.h> // will a static dependency to VSCustomNativeHeapEtwProvider.lib

extern "C" {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
vstools_FHeapTracker vstools_CreateHeapTracker(const char* name) {
    return ::OpenHeapTracker(name);
}
//----------------------------------------------------------------------------
void vstools_DestroyHeapTracker(vstools_FHeapTracker heap) {
    ::CloseHeapTracker(heap);
}
//----------------------------------------------------------------------------
bool vstools_IsValidHeapTracker(vstools_FHeapTracker heap) NOEXCEPT {
    return ((!!heap) & (heap != INVALID_VSHEAPTRACK_HANDLE_VALUE));
}
//----------------------------------------------------------------------------
bool vstools_AllocateEvent(vstools_FHeapTracker heap, void* ptr, size_t sz) {
    return (::VSHeapTrackerAllocateEvent(heap, ptr, (unsigned long)sz) == 0);
}
//----------------------------------------------------------------------------
bool vstools_ReallocateEvent(vstools_FHeapTracker heap, void* newp, size_t newsz, void* oldp) {
    return (::VSHeapTrackerReallocateEvent(heap, newp, (unsigned long)newsz, oldp) == 0);
}
//----------------------------------------------------------------------------
bool vstools_DeallocateEvent(vstools_FHeapTracker heap, void* ptr) {
    return (::VSHeapTrackerDeallocateEvent(heap, ptr) == 0);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!extern "C"

#endif //!USE_PPE_EXTERNAL_VSTOOLS
