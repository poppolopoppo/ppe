#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/AbstractMaterialParameter.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace Engine {
struct MaterialContext;
FWD_REFPTR(MaterialDatabase);
FWD_REFPTR(MaterialEffect);
class Scene;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MaterialParameterBlock : public TypedMaterialParameter<T> {
public:
    MaterialParameterBlock();
    virtual ~MaterialParameterBlock();

    MaterialParameterBlock(T&& rvalue);
    MaterialParameterBlock(const T& value);

    const T& Value() const { return _value; }

    void SetValue(T&& value);
    void SetValue(const T& value);

    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterBlock);

protected:
    virtual bool EvalIFN_ReturnIfChanged_(const MaterialContext& context) override;
    virtual void CopyTo_AssumeEvaluated_(void *dst, size_t sizeInBytes) const override;

private:
    T _value;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DECL(MaterialParameterBlock, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool TryCreateOptionalMaterialParameter(
    AbstractMaterialParameter **param,
    MaterialEffect *materialEffect,
    MaterialDatabase *materialDatabase,
    const Scene *scene,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
