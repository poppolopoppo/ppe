#include "stdafx.h"

#include "MaterialParameterBool.h"

#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialDatabase.h"
#include "Material/IMaterialParameter.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static IMaterialParameter *CreateNotParam_(IMaterialParameter *source) {
    Assert(source);

    switch (source->Info().Type) {
    case Graphics::EConstantFieldType::Bool:
        return new MaterialParameterBool::TMemoizer_Not<bool>(source->Cast<bool>());
    case Graphics::EConstantFieldType::Int:
        return new MaterialParameterBool::TMemoizer_Not<i32>(source->Cast<i32>());
    case Graphics::EConstantFieldType::UInt:
        return new MaterialParameterBool::TMemoizer_Not<u32>(source->Cast<u32>());
    
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterBool {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_BOOL(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database) {
    Assert(database);

#define BIND_MATERIALPARAMETER(_Variability, _Type, _Name) \
    database->BindParameter("uni" STRINGIZE(_Name), new MATERIALPARAMETER_FN(_Variability, _Type, _Name)() );

    EACH_MATERIALPARAMETER_BOOL(BIND_MATERIALPARAMETER)

#undef BIND_MATERIALPARAMETER
}
//----------------------------------------------------------------------------
} //!MaterialParameterBool
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterBool {
//----------------------------------------------------------------------------
// TNot
//----------------------------------------------------------------------------
template <typename T>
TNot<T>::TNot(ITypedMaterialParameter<T> *source) 
:   Source(source) { Assert(Source); }
//----------------------------------------------------------------------------
template <typename T>
void TNot<T>::TypedEval(const FMaterialParameterContext& context, T& dst) {
    T original;
    Source->TypedEval(context, original);
    dst = (~original);
}
//----------------------------------------------------------------------------
template <>
void TNot<bool>::TypedEval(const FMaterialParameterContext& context, bool& dst) {
    bool original;
    Source->TypedEval(context, original);
    dst = (!original);
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<TNot<bool> >;
template class TMaterialParameterMemoizer<TNot<i32> >;
template class TMaterialParameterMemoizer<TNot<u32> >;
//----------------------------------------------------------------------------
} //!MaterialParameterBool
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterBool {
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field ) {
    Assert(param);
    Assert(context.MaterialEffect);
    Assert(context.Database);
    Assert(context.Scene);
    Assert(!name.empty());

    const char *cstr = name.c_str();

    static const char uniNot[] = "uniNot_";

    IMaterialParameter *(*paramFunc)(IMaterialParameter *) = nullptr;
    Graphics::FBindName parameterName;

    if (StartsWith(cstr, uniNot)) {
        paramFunc = &CreateNotParam_;
        parameterName = &cstr[lengthof(uniNot) - 1];
    }
    else {
        Assert(!*param);
        return false;
    }
    Assert(paramFunc);
    Assert(!parameterName.empty());

    PMaterialParameter source;
    if (!GetOrCreateMaterialParameter(&source, context, parameterName, field))
        return false;

    Assert(source);
    *param = (paramFunc)(source.get());

    if (nullptr == param->get())
        return false;

    context.MaterialEffect->BindParameter(name, *param); // TODO : bind in local context.MaterialParameterDatabase
    return true;
}
//----------------------------------------------------------------------------
} //!MaterialParameterBool
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
