#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/IMaterialParameter.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarMatrix_fwd.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMaterialParameterVariable : public ITypedMaterialParameter<T> {
public:
    explicit TMaterialParameterVariable(T&& rvalue);
    explicit TMaterialParameterVariable(const T& value);
    virtual ~TMaterialParameterVariable();

    const T& FValue() const;
    void SetValue(const T& value);

    virtual FMaterialParameterInfo Info() const override;

    virtual void Eval(const FMaterialParameterContext& context, void *dst, size_t sizeInBytes) override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    T _value;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DECL(TMaterialParameterVariable, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
