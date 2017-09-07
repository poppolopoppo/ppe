#pragma once

#include "Core.Graphics/Graphics.h"

#ifndef FINAL_RELEASE
//#   define WITH_CORE_RENDERDOC
#endif

#ifdef WITH_CORE_RENDERDOC

#include "Core/Meta/Singleton.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Misc/DLLWrapper.h"

namespace Core {
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
    static const FRenderDocWrapper& Instance() { return Meta::TSingleton<FRenderDocWrapper>::Instance(); }

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
} //!namespace Core

#endif //!WITH_CORE_RENDERDOC
