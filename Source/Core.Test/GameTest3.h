#pragma once

#include "Core.Graphics/Device/DeviceAPI.h"

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
class FGameTest3 : public Core::Application::FApplicationWindow {
public:
    typedef Core::Application::FApplicationWindow parent_type;

    FGameTest3() : FGameTest3(L"Core Game Window Test Pipa Bimba 3") {}
    explicit FGameTest3(const wchar_t *appname);
    virtual ~FGameTest3();

    virtual void Start() override;
    virtual void Shutdown() override;

protected:
    virtual void Initialize(const FTimeline& time) override;
    virtual void Destroy() override;

    virtual void LoadContent() override;
    virtual void UnloadContent() override;

    virtual void Update(const FTimeline& time) override;
    virtual void Draw(const FTimeline& time) override;

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
