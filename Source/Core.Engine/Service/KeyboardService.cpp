#include "stdafx.h"

#include "KeyboardService.h"

#include "Core/Meta/Guid.h"

#include "Core.Graphics/Window/GraphicsWindow.h"

#include "IServiceProvider.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ENGINESERVICE_GUID_DEF(IKeyboardService);
//----------------------------------------------------------------------------
DefaultKeyboardService::DefaultKeyboardService(Graphics::GraphicsWindow *window)
:   IKeyboardService(ENGINESERVICE_CONSTRUCT(IKeyboardService)) 
,   _window(window) {
    Assert(_window);
}
//----------------------------------------------------------------------------
DefaultKeyboardService::~DefaultKeyboardService() {}
//----------------------------------------------------------------------------
Engine::KeyboardInputHandler *DefaultKeyboardService::KeyboardInputHandler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_KeyboardInputHandler;
}
//----------------------------------------------------------------------------
const Engine::KeyboardInputHandler *DefaultKeyboardService::KeyboardInputHandler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_KeyboardInputHandler;
}
//----------------------------------------------------------------------------
void DefaultKeyboardService::Start(IServiceProvider *provider, const Guid& guid) {
    IKeyboardService::Start(provider, guid);

    _window->RegisterMessageHandler(&_KeyboardInputHandler);
}
//----------------------------------------------------------------------------
void DefaultKeyboardService::Shutdown(IServiceProvider *provider, const Guid& guid) {
    IKeyboardService::Shutdown(provider, guid);

    _window->UnregisterMessageHandler(&_KeyboardInputHandler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
