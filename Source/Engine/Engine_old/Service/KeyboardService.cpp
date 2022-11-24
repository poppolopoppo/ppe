// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
FDefaultKeyboardService::FDefaultKeyboardService(Graphics::FGraphicsWindow *window)
:   IKeyboardService(ENGINESERVICE_CONSTRUCT(IKeyboardService)) 
,   _window(window) {
    Assert(_window);
}
//----------------------------------------------------------------------------
FDefaultKeyboardService::~FDefaultKeyboardService() {}
//----------------------------------------------------------------------------
Engine::FKeyboardInputHandler *FDefaultKeyboardService::FKeyboardInputHandler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_KeyboardInputHandler;
}
//----------------------------------------------------------------------------
const Engine::FKeyboardInputHandler *FDefaultKeyboardService::FKeyboardInputHandler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_KeyboardInputHandler;
}
//----------------------------------------------------------------------------
void FDefaultKeyboardService::Start(IServiceProvider *provider, const FGuid& guid) {
    IKeyboardService::Start(provider, guid);

    _window->RegisterMessageHandler(&_KeyboardInputHandler);
}
//----------------------------------------------------------------------------
void FDefaultKeyboardService::Shutdown(IServiceProvider *provider, const FGuid& guid) {
    IKeyboardService::Shutdown(provider, guid);

    _window->UnregisterMessageHandler(&_KeyboardInputHandler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
