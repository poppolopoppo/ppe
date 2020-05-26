#pragma once

#include "Graphics.h"

#ifndef FINAL_RELEASE
//#   define WITH_PPE_RENDERDOC
#endif

#ifdef WITH_PPE_RENDERDOC

#include "Meta/Singleton.h"
#include "Meta/ThreadResource.h"
#include "Misc/DLLWrapper.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRenderDocWrapper :
    public Meta::TSingleton<FRenderDocWrapper>
,   public Meta::FThreadResource {
public:
    using Meta::TSingleton<FRenderDocWrapper>::Destroy;
    static void Create() { Meta::TSingleton<FRenderDocWrapper>::Create(); }
    static const FRenderDocWrapper& Get() { return Meta::TSingleton<FRenderDocWrapper>::Get(); }

    bool IsAvailable() const;
    bool IsTargetControlConnected() const;
    bool IsFrameCapturing() const;

    bool LaunchReplayUI() const;

    bool SetActiveWindow(void* device, void* hwnd) const;

    bool TriggerCapture() const;
    bool TriggerMultiFrameCapture(size_t numFrames) const;

private:
    friend class Meta::TSingleton<FRenderDocWrapper>;

    FRenderDocWrapper();
    ~FRenderDocWrapper();

    void* _api;
    FDLLWrapper _renderdoc_dll;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE

#endif //!WITH_PPE_RENDERDOC
