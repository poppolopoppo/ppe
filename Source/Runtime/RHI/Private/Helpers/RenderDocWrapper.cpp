#include "stdafx.h"

#include "RenderDocWrapper.h"

#ifdef WITH_PPE_RENDERDOC

#include "Diagnostic/Logger.h"

#include "External/renderdoc_app.h"

using renderdoc_api_type = ::RENDERDOC_API_1_1_1;

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderDocWrapper::FRenderDocWrapper() 
    : _api(nullptr) {

    if (not _renderdoc_dll.Attach("renderdoc.dll"))
        if (not _renderdoc_dll.Load("C:\\Program Files\\RenderDoc\\renderdoc.dll"))
            return;
    
    ::pRENDERDOC_GetAPI RENDERDOC_GetAPI = (::pRENDERDOC_GetAPI)_renderdoc_dll.FunctionAddr("RENDERDOC_GetAPI");
    Assert(RENDERDOC_GetAPI);
    if (nullptr == RENDERDOC_GetAPI)
        return;

    RENDERDOC_GetAPI(RENDERDOC_Version::eRENDERDOC_API_Version_1_1_1, &_api);

    if (_api) {
        LOG(Info, L"[RenderDOC] Successfully loaded : scenes captures and overlay are available");

        // disable render doc shortcut (prefer to call with our own API)
        reinterpret_cast<renderdoc_api_type*>(_api)->SetCaptureKeys(nullptr, 0);
        reinterpret_cast<renderdoc_api_type*>(_api)->SetFocusToggleKeys(nullptr, 0);

        // capture options
        reinterpret_cast<renderdoc_api_type*>(_api)->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 1);
        reinterpret_cast<renderdoc_api_type*>(_api)->SetCaptureOptionU32(eRENDERDOC_Option_VerifyMapWrites, 1);
    }
    else {
        LOG(Warning, L"[RenderDOC] Failed to load : scene captures and overlay won't be enabled");
    }
}
//----------------------------------------------------------------------------
FRenderDocWrapper::~FRenderDocWrapper() {
    LOG(Info, L"[RenderDOC] Destroying wrapper");
}
//----------------------------------------------------------------------------
bool FRenderDocWrapper::IsAvailable() const {
    return (_renderdoc_dll.IsValid() && _api);
}
//----------------------------------------------------------------------------
bool FRenderDocWrapper::IsTargetControlConnected() const {
    return (IsAvailable() && reinterpret_cast<renderdoc_api_type*>(_api)->IsTargetControlConnected());
}
//----------------------------------------------------------------------------
bool FRenderDocWrapper::IsFrameCapturing() const {
    return (IsAvailable() && reinterpret_cast<renderdoc_api_type*>(_api)->IsFrameCapturing());
}
//----------------------------------------------------------------------------
bool FRenderDocWrapper::LaunchReplayUI() const {
    return (IsAvailable() && reinterpret_cast<renderdoc_api_type*>(_api)->LaunchReplayUI(1, nullptr) != 0);
}
//----------------------------------------------------------------------------
bool FRenderDocWrapper::SetActiveWindow(void* device, void* hwnd) const {
    if (IsAvailable()) {
        reinterpret_cast<renderdoc_api_type*>(_api)->SetActiveWindow(device, hwnd);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FRenderDocWrapper::TriggerCapture() const {
    if (IsAvailable()) {
        reinterpret_cast<renderdoc_api_type*>(_api)->TriggerCapture();
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FRenderDocWrapper::TriggerMultiFrameCapture(size_t numFrames) const {
    if (IsAvailable()) {
        reinterpret_cast<renderdoc_api_type*>(_api)->TriggerMultiFrameCapture(checked_cast<u32>(numFrames));
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE

#endif //!WITH_PPE_RENDERDOC
