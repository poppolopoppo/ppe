#pragma once

#include "Core/Color/Color.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Time/Timely.h"

#include "Core.Graphics/Device/DeviceAPI.h"

#include "Core.Engine/Material/MaterialVariability.h"

#include "Core.Application/Application.h"
#include "Core.Application/ApplicationWindow.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
FWD_REFPTR(RenderTarget);
}

namespace Engine {
FWD_REFPTR(AbstractRenderSurface);
FWD_REFPTR(KeyboardMouseCameraController);
FWD_REFPTR(Material);
FWD_REFPTR(PerspectiveCamera);
struct RenderCommandRegistration;
typedef UniquePtr<const RenderCommandRegistration> URenderCommand;
FWD_REFPTR(RenderContext);
FWD_REFPTR(Scene);
FWD_REFPTR(World);
template <typename T>
class MaterialParameterBlock;
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GameTest2 : public Core::Application::ApplicationWindow {
public:
    typedef Core::Application::ApplicationWindow parent_type;

    GameTest2() : GameTest2(L"Core Game Window Test Pipa Bimba 2") {}
    explicit GameTest2(const wchar_t *appname);
    virtual ~GameTest2();

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

    // TODO : mesh pack
    Graphics::PIndexBuffer _indices[2];
    Graphics::PVertexBuffer _vertices[2];

    Engine::PMaterial _materials[3];
    Engine::URenderCommand _commands[3];
    RefPtr<Engine::MaterialParameterBlock<float4x4> > _transforms[3];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
