#pragma once

#include "MeshBuilder_fwd.h"

#include "RHI/ResourceEnums.h"

#include "Allocator/Allocation.h"
#include "Container/AssociativeVector.h"
#include "IO/Filename.h"
#include "IO/String.h"
#include "Memory/RefPtr.h"
#include "Misc/Opaque.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericTextureSampler {
    FFilename Filename;
    RHI::EImageView View{ Default };
    RHI::EAddressMode AddressMode{ Default };
    RHI::ETextureFilter Filter{ Default };
};
//----------------------------------------------------------------------------
using FGenericParameterBlock = Opaq::TValueBlock<ALLOCATOR(MeshBuilder)>;
//----------------------------------------------------------------------------
FWD_REFPTR(GenericMaterial);
class FGenericMaterial : public FRefCountable {
public:
    typedef VECTORINSITU(MeshBuilder, FString, 3) FTagVector;
    typedef ASSOCIATIVE_VECTORINSITU(MeshBuilder, FString, FGenericParameterBlock, 3) FParameterMap;
    typedef ASSOCIATIVE_VECTORINSITU(MeshBuilder, FString, FGenericTextureSampler, 3) FTextureMap;

    FGenericMaterial() = default;
    PPE_MESHBUILDER_API FGenericMaterial(FString&& name,
                     FString&& technique,
                     FTagVector&& tags,
                     FParameterMap&& parameters,
                     FTextureMap&& textures );

    FGenericMaterial(FGenericMaterial&& rvalue) = default;
    FGenericMaterial& operator =(FGenericMaterial&& rvalue) = default;

    FGenericMaterial(const FGenericMaterial& ) = delete;
    FGenericMaterial& operator =(const FGenericMaterial& ) = delete;

    NODISCARD const FString& Name() const { return _name; }
    void SetName(FString&& rvalue) { Assert(!rvalue.empty()); _name = std::move(rvalue); }
    void SetName(const FString& value) { Assert(!value.empty()); _name = value; }
    void SetName(const FStringView& value) { Assert(!value.empty()); _name = ToString(value); }

    NODISCARD const FString& Technique() const { return _technique; }
    void SetTechnique(FString&& rvalue) { Assert(!rvalue.empty()); _technique = std::move(rvalue); }
    void SetTechnique(const FString& value) { Assert(!value.empty()); _technique = value; }
    void SetTechnique(const FStringView& value) { Assert(!value.empty()); _technique = ToString(value); }

    NODISCARD const FParameterMap& Parameters() const { return _parameters; }
    void SetParameters(FParameterMap&& rvalue) { _parameters = std::move(rvalue); }

    NODISCARD const FTextureMap& Textures() const { return _textures; }
    void SetTextures(FTextureMap&& rvalue) { _textures = std::move(rvalue); }

    PPE_MESHBUILDER_API void AddTag(const FStringView& tag);
    NODISCARD PPE_MESHBUILDER_API bool HasTag(const FStringView& tag) const;
    NODISCARD PPE_MESHBUILDER_API bool RemoveTag(const FStringView& tag);

    PPE_MESHBUILDER_API void AddParameter(const FStringView& key, const Opaq::value_init& value); // must be unique
    PPE_MESHBUILDER_API void SetParameter(const FStringView& key, const Opaq::value_init& value); // will override existing value
    NODISCARD PPE_MESHBUILDER_API FGenericParameterBlock& GetParameterBlock(const FStringView& key);
    NODISCARD PPE_MESHBUILDER_API FGenericParameterBlock* GetParameterBlockIFP(const FStringView& key);
    NODISCARD const FGenericParameterBlock& GetParameterBlock(const FStringView& key) const { return remove_const(this)->GetParameterBlock(key); }
    NODISCARD const FGenericParameterBlock* GetParameterBlockIFP(const FStringView& key) const { return remove_const(this)->GetParameterBlockIFP(key); }
    NODISCARD PPE_MESHBUILDER_API bool RemoveParameter(const FStringView& key);

    template <typename T>
    NODISCARD T& GetParameter(const FStringView& key) { return std::get<T>(*GetParameterBlock(key)); }
    template <typename T>
    NODISCARD const T& GetParameter(const FStringView& key) const { return std::get<T>(*GetParameterBlock(key)); }

    template <typename T>
    NODISCARD T* GetParameterIFP(const FStringView& key) {
        if (FGenericParameterBlock* const pBlock = GetParameterBlockIFP(key))
            return std::get_if<T>(**pBlock);
        return nullptr;
    }
    template <typename T> const T* GetParameterIFP(const FStringView& key) const {
        if (const FGenericParameterBlock* const pBlock = GetParameterBlockIFP(key))
            return std::get_if<T>(**pBlock);
        return nullptr;
    }

    PPE_MESHBUILDER_API void AddTexture(const FStringView& key,
                    const FFilename& filename,
                    RHI::EImageView view,
                    RHI::EAddressMode addressMode,
                    RHI::ETextureFilter filter );
    PPE_MESHBUILDER_API void AddTexture2D(const FStringView& key, const FFilename& filename);
    PPE_MESHBUILDER_API void AddTexture3D(const FStringView& key, const FFilename& filename);
    PPE_MESHBUILDER_API void AddTextureCube(const FStringView& key, const FFilename& filename);
    PPE_MESHBUILDER_API void AddTexture(const FStringView& key, const FGenericTextureSampler& sampler);
    NODISCARD PPE_MESHBUILDER_API const FGenericTextureSampler& GetTexture(const FStringView& key) const;
    NODISCARD PPE_MESHBUILDER_API const FGenericTextureSampler* GetTextureIFP(const FStringView& key) const;
    PPE_MESHBUILDER_API bool RemoveTexture(const FStringView& key);

private:
    FString _name;
    FString _technique;
    FTagVector _tags;
    FParameterMap _parameters;
    FTextureMap _textures;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
