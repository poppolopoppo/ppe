#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
FWD_REFPTR(Material);
FWD_REFPTR(MaterialDatabase);
class Scene;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MaterialParameterMath_Invert : public AbstractMaterialParameterMemoizer<T> {
public:
    MaterialParameterMath_Invert(TypedMaterialParameter<T> *source)
    :   AbstractMaterialParameterMemoizer(source->Variability()), _source(source) {
        Assert(source);
    }
    virtual ~MaterialParameterMath_Invert() {}
    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterMath_Invert);
protected:
    bool Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) override;
private:
    RefPtr<TypedMaterialParameter<T> > _source;
};
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(extern, MaterialParameterMath_Invert, , float3x3);
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(extern, MaterialParameterMath_Invert, , float4x4);
//----------------------------------------------------------------------------
template <typename T>
class MaterialParameterMath_InvertTranspose : public AbstractMaterialParameterMemoizer<T> {
public:
    MaterialParameterMath_InvertTranspose(TypedMaterialParameter<T> *source)
    :   AbstractMaterialParameterMemoizer(source->Variability()), _source(source) {
        Assert(source);
    }
    virtual ~MaterialParameterMath_InvertTranspose() {}
    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterMath_InvertTranspose);
protected:
    bool Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) override;
private:
    RefPtr<TypedMaterialParameter<T> > _source;
};
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(extern, MaterialParameterMath_InvertTranspose, , float3x3);
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(extern, MaterialParameterMath_InvertTranspose, , float4x4);
//----------------------------------------------------------------------------
template <typename T>
class MaterialParameterMath_Rcp : public AbstractMaterialParameterMemoizer<T> {
public:
    MaterialParameterMath_Rcp(TypedMaterialParameter<T> *source)
    :   AbstractMaterialParameterMemoizer(source->Variability()), _source(source) {
        Assert(source);
    }
    virtual ~MaterialParameterMath_Rcp() {}
    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterMath_Rcp);
protected:
    bool Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) override;
private:
    RefPtr<TypedMaterialParameter<T> > _source;
};
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_SCALAR_(extern, MaterialParameterMath_Rcp, );
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_VECTOR_(extern, MaterialParameterMath_Rcp, );
//----------------------------------------------------------------------------
class MaterialParameterMath_SRGB : public AbstractMaterialParameterMemoizer<float3> {
public:
    MaterialParameterMath_SRGB(TypedMaterialParameter<float3> *source)
    :   AbstractMaterialParameterMemoizer(source->Variability())
    ,   _source(source) {
        Assert(source);
    }
    virtual ~MaterialParameterMath_SRGB() {}
    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterMath_SRGB);
protected:
    bool Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) override;
private:
    RefPtr<TypedMaterialParameter<float3> > _source;
};
//----------------------------------------------------------------------------
class MaterialParameterMath_SRGBA : public AbstractMaterialParameterMemoizer<float4> {
public:
    MaterialParameterMath_SRGBA(TypedMaterialParameter<float4> *source)
    :   AbstractMaterialParameterMemoizer(source->Variability())
    ,   _source(source) {
        Assert(source);
    }
    virtual ~MaterialParameterMath_SRGBA() {}
    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterMath_SRGBA);
protected:
    bool Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) override;
private:
    RefPtr<TypedMaterialParameter<float4> > _source;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterMathMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
bool TryCreateMathMaterialParameter(
    AbstractMaterialParameter **param,
    const Material *material,
    const Scene *scene,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#include "Core.Engine/Material/Parameters/MaterialParameterMath-inl.h"
