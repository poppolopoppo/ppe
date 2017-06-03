#include "stdafx.h"

#include "RenderDocWrapper.h"

#ifdef WITH_CORE_RENDERDOC

#include "Core/Diagnostic/Logger.h"

#ifdef PLATFORM_WINDOWS
#   include <windows.h>
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void RenderDoc_StartAPI_(void** pLibrary, FRenderDocWrapper::renderdoc_api_type** pAPI) {
#ifdef PLATFORM_WINDOWS
     ::HMODULE const renderDoc = ::LoadLibraryA("C:\\Program Files\\RenderDoc\\renderdoc.dll");

     *pAPI = nullptr;
     *pLibrary = (void*)renderDoc;

     if (nullptr == renderDoc)
         return;

     ::pRENDERDOC_GetAPI RENDERDOC_GetAPI = (::pRENDERDOC_GetAPI)::GetProcAddress(renderDoc, "RENDERDOC_GetAPI");
     Assert(RENDERDOC_GetAPI);
     if (nullptr == RENDERDOC_GetAPI)
         return;

#   ifdef WITH_CORE_ASSERT
     const int result =
#   endif
         RENDERDOC_GetAPI(RENDERDOC_Version::eRENDERDOC_API_Version_1_1_1, (void**)pAPI);
     Assert(result);

#else
#   error "not supported yet"
#endif
}
//----------------------------------------------------------------------------
static void RenderDoc_ShutdownAPI_(void* library, FRenderDocWrapper::renderdoc_api_type* api) {
    if (api)
        api->Shutdown();

#ifdef PLATFORM_WINDOWS
    if (library)
        ::FreeLibrary((::HMODULE)library);

#else
#   error "not supported yet"
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderDocWrapper::FRenderDocWrapper() {
    RenderDoc_StartAPI_(&_library, &_api);

    if (_api) {
        // disable render doc shortcut (prefer to call with our own API)
        _api->SetCaptureKeys(nullptr, 0);
        _api->SetFocusToggleKeys(nullptr, 0);

        // capture options
        _api->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 1);
        _api->SetCaptureOptionU32(eRENDERDOC_Option_VerifyMapWrites, 1);
    }
    else {
        LOG(Warning, L"[RenderDOC] Failed to load 'renderdoc.dll' : scene captures and overlay won't be enabled");
    }
}
//----------------------------------------------------------------------------
FRenderDocWrapper::~FRenderDocWrapper() {
    RenderDoc_ShutdownAPI_(_library, _api);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core

#endif //!WITH_CORE_RENDERDOC
