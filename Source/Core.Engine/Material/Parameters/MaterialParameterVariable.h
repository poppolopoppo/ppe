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
class MaterialParameterVariable : public ITypedMaterialParameter<T> {
public:
    explicit MaterialParameterVariable(T&& rvalue);
    explicit MaterialParameterVariable(const T& value);
    virtual ~MaterialParameterVariable();

    const T& Value() const;
    void SetValue(const T& value);

    virtual MaterialParameterInfo Info() const override;

    virtual void Eval(const MaterialParameterContext& context, void *dst, size_t sizeInBytes) override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    T _value;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DECL(MaterialParameterVariable, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
