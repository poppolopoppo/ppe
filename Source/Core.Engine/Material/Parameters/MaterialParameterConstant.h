#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/IMaterialParameter.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarMatrix_fwd.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMaterialParameterConstant : public ITypedMaterialParameter<T> {
public:
    explicit TMaterialParameterConstant(T&& rvalue);
    explicit TMaterialParameterConstant(const T& value);
    virtual ~TMaterialParameterConstant();

    const T& FValue() const { return _value; }

    virtual FMaterialParameterInfo Info() const override;

    virtual void Eval(const FMaterialParameterContext& context, void *dst, size_t sizeInBytes) override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    T _value;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DECL(TMaterialParameterConstant, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool TryCreateOptionalMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
