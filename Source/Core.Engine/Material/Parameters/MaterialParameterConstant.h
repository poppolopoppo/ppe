#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/IMaterialParameter.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Maths/Transform/ScalarMatrix_fwd.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MaterialParameterConstant : public ITypedMaterialParameter<T> {
public:
    explicit MaterialParameterConstant(T&& rvalue);
    explicit MaterialParameterConstant(const T& value);
    virtual ~MaterialParameterConstant();

    const T& Value() const { return _value; }

    virtual MaterialParameterInfo Info() const override;

    virtual void Eval(const MaterialParameterContext& context, void *dst, size_t sizeInBytes) override;

    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterConstant);

private:
    T _value;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DECL(MaterialParameterConstant, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool TryCreateOptionalMaterialParameter(
    PMaterialParameter *param,
    const MaterialParameterMutableContext& context,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
