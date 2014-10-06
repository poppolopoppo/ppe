#pragma once

#include "Engine.h"

#include "AbstractMaterialParameter.h"

#include "Core.Graphics/ConstantField.h"

#include "Core/PoolAllocator.h"
#include "Core/ScalarMatrix.h"
#include "Core/ScalarVector.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MaterialContext;
//----------------------------------------------------------------------------
template <typename T>
class MaterialParameterBlock : public TypedMaterialParameter<T> {
public:
    MaterialParameterBlock();
    virtual ~MaterialParameterBlock();

    MaterialParameterBlock(T&& rvalue);
    MaterialParameterBlock(const T& rvalue);

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
} //!namespace Engine
} //!namespace Core
