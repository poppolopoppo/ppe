#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Memory/RefPtr.h"

namespace Core {

namespace Graphics {
enum class ConstantFieldType;
}

namespace Engine {
enum class MaterialVariability;
struct MaterialParameterContext;
struct MaterialParameterMutableContext;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMaterialParameter;
//----------------------------------------------------------------------------
typedef RefPtr<IMaterialParameter> PMaterialParameter;
typedef RefPtr<const IMaterialParameter> PCMaterialParameter;
//----------------------------------------------------------------------------
typedef SafePtr<IMaterialParameter> SMaterialParameter;
typedef SafePtr<const IMaterialParameter> SCMaterialParameter;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class ITypedMaterialParameter;
//----------------------------------------------------------------------------
template <typename T>
using PTypedMaterialParameter = RefPtr<ITypedMaterialParameter<T> >;
template <typename T>
using PCTypedMaterialParameter = RefPtr<const ITypedMaterialParameter<T> >;
//----------------------------------------------------------------------------
template <typename T>
using STypedMaterialParameter = SafePtr<ITypedMaterialParameter<T> >;
template <typename T>
using SCTypedMaterialParameter = SafePtr<const ITypedMaterialParameter<T> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
