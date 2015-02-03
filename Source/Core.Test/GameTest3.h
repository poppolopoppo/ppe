#pragma once

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"

#include "Core.Application/Application.h"
#include "Core.Application/ApplicationWindow.h"

#include "Core.Engine/Mesh/Model_fwd.h"

#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
}

namespace Engine {
FWD_REFPTR(PerspectiveCamera);
FWD_REFPTR(RenderContext);
FWD_REFPTR(Scene);
FWD_REFPTR(World);
FWD_REFPTR(KeyboardMouseCameraController);
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GameTest3 : public Core::Application::ApplicationWindow {
public:
    typedef Core::Application::ApplicationWindow parent_type;

    GameTest3() : GameTest3(L"Core Game Window Test Pipa Bimba 3") {}
    explicit GameTest3(const wchar_t *appname);
    virtual ~GameTest3();

    virtual void Start() override;
    virtual void Shutdown() override;

protected:
    virtual void Initialize(const Timeline& time) override;
    virtual void Destroy() override;

    virtual void LoadContent() override;
    virtual void UnloadContent() override;

    virtual void Update(const Timeline& time) override;
    virtual void Draw(const Timeline& time) override;

    virtual void Present() override;

private:
    Engine::PPerspectiveCamera _camera;
    Engine::PKeyboardMouseCameraController _cameraController;

    Engine::PWorld _world;
    Engine::PRenderContext _context;
    Engine::PScene _mainScene;

    Engine::PModel _model;
    Engine::UModelRenderCommand _renderCommand;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
