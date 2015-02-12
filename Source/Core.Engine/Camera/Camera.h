#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Camera/CameraModel.h"

#include "Core/Maths/Geometry/ScalarRectangle_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;

namespace Engine {
class ICameraController;
typedef RefPtr<ICameraController> PCameraController;
typedef RefPtr<const ICameraController> PCCameraController;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ICamera : public RefCountable {
public:
    ICamera(float znear, float zfar);
    virtual ~ICamera();

    ICamera(const ICamera& ) = delete;
    ICamera& operator =(const ICamera& ) = delete;

    float ZNear() const { return _znear; }
    float ZFar() const { return _zfar; }

    const CameraModel& Model() const { return _model; }
    const ICameraController *Controller() const { return _controller.get(); }

    void SetController(ICameraController *controller);

    void CurrentProjection(float4x4 *projection) const;

    void Update(const Timeline& time);
    void OnResize(const ViewportF& viewport);

protected:
    virtual void UpdateImpl(float4x4 *projection, const Timeline& time) = 0;
    virtual void OnResizeImpl(const ViewportF& viewport) = 0;

    float _znear;
    float _zfar;

private:
    float4x4 _projection;
    CameraModel _model;
    PCameraController _controller;
};
//----------------------------------------------------------------------------
typedef RefPtr<ICamera> PCamera;
typedef RefPtr<const ICamera> PCCamera;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(PerspectiveCamera);
class PerspectiveCamera : public ICamera {
public:
    PerspectiveCamera(float fov/* rad */, float znear, float zfar, const ViewportF& viewport);
    virtual ~PerspectiveCamera();

    float FOV() const { return _fov; }

    float AspectRatio() const { return _aspectRatio; }

protected:
    virtual void UpdateImpl(float4x4 *projection, const Timeline& time) override;
    virtual void OnResizeImpl(const ViewportF& viewport) override;

private:
    float _fov; // rad
    float _aspectRatio;
};
//----------------------------------------------------------------------------
FWD_REFPTR(OrthographicOffCenterCamera);
class OrthographicOffCenterCamera : public ICamera {
public:
    OrthographicOffCenterCamera(float znear, float zfar, const ViewportF& viewport);
    virtual ~OrthographicOffCenterCamera();

    float Left() const { return _left; }
    float Right() const { return _right; }

    float Bottom() const { return _bottom; }
    float Top() const { return _top; }

protected:
    virtual void UpdateImpl(float4x4 *projection, const Timeline& time) override;
    virtual void OnResizeImpl(const ViewportF& viewport) override;

private:
    float _left;
    float _right;

    float _bottom;
    float _top;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
