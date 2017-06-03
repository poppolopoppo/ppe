#pragma once

#include "Core.Graphics/Graphics.h"

#ifndef FINAL_RELEASE
#   define WITH_CORE_RENDERDOC
#endif

#ifdef WITH_CORE_RENDERDOC

#include "Core/Meta/Singleton.h"
#include "Core/Meta/ThreadResource.h"

#include "renderdoc_app.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRenderDocWrapper :
    public Meta::TSingleton<FRenderDocWrapper>
,   public Meta::FThreadResource {
public:
    static void Create() { Meta::TSingleton<FRenderDocWrapper>::Create(); }
    using Meta::TSingleton<FRenderDocWrapper>::Destroy;

    typedef ::RENDERDOC_API_1_1_1 renderdoc_api_type;

    static const renderdoc_api_type* API() {
#ifdef WITH_CORE_ASSERT
        const FRenderDocWrapper& wrapper = Instance();
        THREADRESOURCE_CHECKACCESS(&wrapper);
        return (wrapper._api);
#else
        return Instance()._api;
#endif
    }

private:
    friend class Meta::TSingleton<FRenderDocWrapper>;

#ifdef WITH_CORE_ASSERT
    using Meta::TSingleton<FRenderDocWrapper>::HasInstance;
#endif
    using Meta::TSingleton<FRenderDocWrapper>::Instance;

    FRenderDocWrapper();
    ~FRenderDocWrapper();

    renderdoc_api_type* _api;
    void* _library;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core

#endif //!WITH_CORE_RENDERDOC
