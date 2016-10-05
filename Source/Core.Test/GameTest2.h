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
struct FRenderCommandRegistration;
typedef TUniquePtr<const FRenderCommandRegistration> URenderCommand;
FWD_REFPTR(RenderContext);
FWD_REFPTR(Scene);
FWD_REFPTR(World);
template <typename T>
class TMaterialParameterBlock;
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGameTest2 : public Core::Application::FApplicationWindow {
public:
    typedef Core::Application::FApplicationWindow parent_type;

    FGameTest2() : FGameTest2(L"Core Game Window Test Pipa Bimba 2") {}
    explicit FGameTest2(const wchar_t *appname);
    virtual ~FGameTest2();

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

    // TODO : mesh pack
    Graphics::PIndexBuffer _indices[2];
    Graphics::PVertexBuffer _vertices[2];

    Engine::PMaterial _materials[3];
    Engine::URenderCommand _commands[3];
    TRefPtr<Engine::TMaterialParameterBlock<float4x4> > _transforms[3];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
