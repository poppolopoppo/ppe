#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Container/RawStorage.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(ConstantBufferLayout);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ShaderTextureDimension {
    Texture2D = 0,
    //Texture3D, TODO
    TextureCube,
    //Texture2DArray, TODO
    //Texture3DArray, TODO
    //TextureCubeArray, TODO
};
//----------------------------------------------------------------------------
struct ShaderProgramTexture {
    Graphics::BindName Name;
    ShaderTextureDimension Dimension;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderCompiled);
class ShaderCompiled : public RefCountable {
public:
    typedef RAWSTORAGE(Shader, u8) blob_type;
    typedef ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout) constants_type;
    typedef VECTOR(Shader, ShaderProgramTexture) textures_type;

    ShaderCompiled( u64 fingerprint,
                    blob_type&& blob,
                    constants_type&& constants,
                    textures_type&& textures );

    ShaderCompiled(const ShaderCompiled& ) = delete;
    ShaderCompiled& operator =(const ShaderCompiled& ) = delete;

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
} //!namespace Core
