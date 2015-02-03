#include "stdafx.h"

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
DefaultMouseService::DefaultMouseService(Graphics::GraphicsWindow *window)
:   IMouseService(ENGINESERVICE_CONSTRUCT(IMouseService)) 
,   _window(window) {
    Assert(_window);
}
//----------------------------------------------------------------------------
DefaultMouseService::~DefaultMouseService() {}
//----------------------------------------------------------------------------
Engine::MouseInputHandler *DefaultMouseService::MouseInputHandler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_mouseInputHandler;
}
//----------------------------------------------------------------------------
const Engine::MouseInputHandler *DefaultMouseService::MouseInputHandler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_mouseInputHandler;
}
//----------------------------------------------------------------------------
void DefaultMouseService::Start(IServiceProvider *provider, const Guid& guid) {
    IMouseService::Start(provider, guid);

    _window->RegisterMessageHandler(&_mouseInputHandler);
}
//----------------------------------------------------------------------------
void DefaultMouseService::Shutdown(IServiceProvider *provider, const Guid& guid) {
    IMouseService::Shutdown(provider, guid);

    _window->UnregisterMessageHandler(&_mouseInputHandler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
