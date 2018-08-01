#pragma once

#include "Graphics.h"

#include "Name.h"
#include "Device/Shader/ConstantBufferLayout.h"

#include "Allocator/PoolAllocator.h"
#include "Container/AssociativeVector.h"
#include "Container/RawStorage.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace Graphics {
FWD_REFPTR(ConstantBufferLayout);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FShaderProgramTexture {
    FName Name;
    enum class ETextureDimension Dimension;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderCompiled);
class FShaderCompiled : public FRefCountable {
public:
    typedef RAWSTORAGE(Shader, u8) blob_type;
    typedef ASSOCIATIVE_VECTOR(Shader, FName, PCConstantBufferLayout) constants_type;
    typedef VECTOR(Shader, FShaderProgramTexture) textures_type;

    FShaderCompiled( u64 fingerprint,
                    blob_type&& blob,
                    constants_type&& constants,
                    textures_type&& textures );

    FShaderCompiled(const FShaderCompiled& ) = delete;
    FShaderCompiled& operator =(const FShaderCompiled& ) = delete;

    u64 Fingerprint() const { return _fingerprint; }
    const blob_type& Blob() const { return _compiledCode; }
    const constants_type& Constants() const { return _constants; }
    const textures_type& Textures() const { return _textures; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    u64 _fingerprint;
    blob_type _compiledCode;
    constants_type _constants;
    textures_type _textures;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
