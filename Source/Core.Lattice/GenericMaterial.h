#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

#include "Core.Graphics/Value.h"

namespace Core {
namespace Graphics {
enum class ETextureAddressMode;
enum class ETextureDimension;
enum class ETextureFilter;
}

namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericTextureSampler {
    FFilename Filename;
    Graphics::ETextureDimension Dimension;
    Graphics::ETextureAddressMode AddressMode;
    Graphics::ETextureFilter Filter;
};
//----------------------------------------------------------------------------
FWD_REFPTR(GenericMaterial);
class FGenericMaterial : public FRefCountable {
public:
    typedef VECTORINSITU(GenericMaterial, FString, 3) FTagVector;
    typedef ASSOCIATIVE_VECTORINSITU(GenericMaterial, FString, Graphics::FValue, 3) FParameterMap;
    typedef ASSOCIATIVE_VECTORINSITU(GenericMaterial, FString, FGenericTextureSampler, 3) FTextureMap;

    FGenericMaterial();
    FGenericMaterial(FString&& name,
                     FString&& technique,
                     FTagVector&& tags,
                     FParameterMap&& parameters,
                     FTextureMap&& textures );
    ~FGenericMaterial();

    FGenericMaterial(FGenericMaterial&& rvalue) = default;
    FGenericMaterial& operator =(FGenericMaterial&& rvalue) = default;

    FGenericMaterial(const FGenericMaterial& ) = delete;
    FGenericMaterial& operator =(const FGenericMaterial& ) = delete;

    const FString& Name() const { return _name; }
    void SetName(FString&& rvalue) { Assert(!rvalue.empty()); _name = std::move(rvalue); }
    void SetName(const FString& value) { Assert(!value.empty()); _name = value; }
    void SetName(const FStringView& value) { Assert(!value.empty()); _name = ToString(value); }

    const FString& Technique() const { return _technique; }
    void SetTechnique(FString&& rvalue) { Assert(!rvalue.empty()); _technique = std::move(rvalue); }
    void SetTechnique(const FString& value) { Assert(!value.empty()); _technique = value; }
    void SetTechnique(const FStringView& value) { Assert(!value.empty()); _technique = ToString(value); }

    const FParameterMap& Parameters() const { return _parameters; }
    void SetParameters(FParameterMap&& rvalue) { _parameters = std::move(rvalue); }

    const FTextureMap& Textures() const { return _textures; }
    void SetTextures(FTextureMap&& rvalue) { _textures = std::move(rvalue); }

    void AddTag(const FStringView& tag);
    bool HasTag(const FStringView& tag) const;
    bool RemoveTag(const FStringView& tag);

    void AddParameter(const FStringView& key, const Graphics::FValue& value); // must be unique
    void SetParameter(const FStringView& key, const Graphics::FValue& value); // will override existing value
    Graphics::FValue& GetParameterValue(const FStringView& key);
    Graphics::FValue* GetParameterValueIFP(const FStringView& key);
    const Graphics::FValue& GetParameterValue(const FStringView& key) const { return remove_const(this)->GetParameterValue(key); }
    const Graphics::FValue* GetParameterValueIFP(const FStringView& key) const { return remove_const(this)->GetParameterValueIFP(key); }
    bool RemoveParameter(const FStringView& key);

    template <typename T> T& GetParameter(const FStringView& key) { return GetParameterValue(key).Get<T>(); }
    template <typename T> T* GetParameterIFP(const FStringView& key) { return auto v = GetParameterValueIFP(key), (v ? v->Get<T>() : nullptr); }
    template <typename T> const T& GetParameter(const FStringView& key) const { return GetParameterValue(key).Get<T>(); }
    template <typename T> const T* GetParameterIFP(const FStringView& key) const { return auto v = GetParameterValueIFP(key), (v ? v->Get<T>() : nullptr); }

    void AddTexture(const FStringView& key,
                    const FFilename& filename,
                    Graphics::ETextureDimension dimension,
                    Graphics::ETextureAddressMode addressMode,
                    Graphics::ETextureFilter filter );
    void AddTexture2D(const FStringView& key, const FFilename& filename);
    void AddTexture3D(const FStringView& key, const FFilename& filename);
    void AddTextureCube(const FStringView& key, const FFilename& filename);
    void AddTexture(const FStringView& key, const FGenericTextureSampler& sampler);
    const FGenericTextureSampler& GetTexture(const FStringView& key) const;
    const FGenericTextureSampler* GetTextureIFP(const FStringView& key) const;
    bool RemoveTexture(const FStringView& key);

    SINGLETON_POOL_ALLOCATED_DECL();

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
} //!namespace Lattice
} //!namespace Core
