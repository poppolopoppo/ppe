// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MouseService.h"

#include "Core/Meta/Guid.h"

#include "Core.Graphics/Window/GraphicsWindow.h"

#include "IServiceProvider.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ENGINESERVICE_GUID_DEF(IMouseService);
//----------------------------------------------------------------------------
FDefaultMouseService::FDefaultMouseService(Graphics::FGraphicsWindow *window)
:   IMouseService(ENGINESERVICE_CONSTRUCT(IMouseService)) 
,   _window(window) {
    Assert(_window);
}
//----------------------------------------------------------------------------
FDefaultMouseService::~FDefaultMouseService() {}
//----------------------------------------------------------------------------
Engine::FMouseInputHandler *FDefaultMouseService::FMouseInputHandler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_mouseInputHandler;
}
//----------------------------------------------------------------------------
const Engine::FMouseInputHandler *FDefaultMouseService::FMouseInputHandler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_mouseInputHandler;
}
//----------------------------------------------------------------------------
void FDefaultMouseService::Start(IServiceProvider *provider, const FGuid& guid) {
    IMouseService::Start(provider, guid);

    _window->RegisterMessageHandler(&_mouseInputHandler);
}
//----------------------------------------------------------------------------
void FDefaultMouseService::Shutdown(IServiceProvider *provider, const FGuid& guid) {
    IMouseService::Shutdown(provider, guid);

    _window->UnregisterMessageHandler(&_mouseInputHandler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
