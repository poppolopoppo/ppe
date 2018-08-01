#include "stdafx.h"

#include "ShaderCompiled.h"

#include "Allocator/PoolAllocator-impl.h"

#include "ConstantBufferLayout.h"
#include "Name.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FShaderCompiled, );
//----------------------------------------------------------------------------
FShaderCompiled::FShaderCompiled(
    u64 fingerprint,
    blob_type&& compiledCode,
    constants_type&& constants,
    textures_type&& textures )
:   _fingerprint(fingerprint)
,   _compiledCode(std::move(compiledCode))
,   _constants(std::move(constants))
,   _textures(std::move(textures)) {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
