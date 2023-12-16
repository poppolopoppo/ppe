#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Memory/RefPtr.h"

namespace Core {

namespace Graphics {
enum class EConstantFieldType;
}

namespace Engine {
enum class EMaterialVariability;
struct FMaterialParameterContext;
struct FMaterialParameterMutableContext;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMaterialParameter;
//----------------------------------------------------------------------------
typedef TRefPtr<IMaterialParameter> PMaterialParameter;
typedef TRefPtr<const IMaterialParameter> PCMaterialParameter;
//----------------------------------------------------------------------------
typedef TSafePtr<IMaterialParameter> SMaterialParameter;
typedef TSafePtr<const IMaterialParameter> SCMaterialParameter;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class ITypedMaterialParameter;
//----------------------------------------------------------------------------
template <typename T>
using TPTypedMaterialParameter = TRefPtr<ITypedMaterialParameter<T> >;
template <typename T>
using TPCTypedMaterialParameter = TRefPtr<const ITypedMaterialParameter<T> >;
//----------------------------------------------------------------------------
template <typename T>
using TSTypedMaterialParameter = TSafePtr<ITypedMaterialParameter<T> >;
template <typename T>
using TSCTypedMaterialParameter = TSafePtr<const ITypedMaterialParameter<T> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
