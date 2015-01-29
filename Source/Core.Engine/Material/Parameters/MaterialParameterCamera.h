#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace Engine {
FWD_REFPTR(MaterialDatabase);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MaterialParameterCamera_EyePosition : public AbstractMaterialParameterMemoizer<float3> {
public:
    MaterialParameterCamera_EyePosition() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_EyePosition() {}
protected:
    bool Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_EyeDirection : public AbstractMaterialParameterMemoizer<float3> {
public:
    MaterialParameterCamera_EyeDirection() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_EyeDirection() {}
protected:
    bool Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_EyeUp : public AbstractMaterialParameterMemoizer<float3> {
public:
    MaterialParameterCamera_EyeUp() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_EyeUp() {}
protected:
    bool Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_View : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_View() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_View() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_InvertView : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_InvertView() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_InvertView() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_Projection : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_Projection() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_Projection() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_InvertProjection : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_InvertProjection() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_InvertProjection() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_ViewProjection : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_ViewProjection() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_ViewProjection() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_InvertViewProjection : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_InvertViewProjection() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_InvertViewProjection() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_FrustumRays : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_FrustumRays() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_FrustumRays() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_NearFarZ : public AbstractMaterialParameterMemoizer<float2> {
public:
    MaterialParameterCamera_NearFarZ() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_NearFarZ() {}
protected:
    bool Memoize_ReturnIfChanged_(float2 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_NearCorners : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_NearCorners() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_NearCorners() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterCamera_FarCorners : public AbstractMaterialParameterMemoizer<float4x4> {
public:
    MaterialParameterCamera_FarCorners() : AbstractMaterialParameterMemoizer(MaterialVariability::Scene) {}
    virtual ~MaterialParameterCamera_FarCorners() {}
protected:
    bool Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterCameraMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
