// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Mesh/GenericMaterial.h"

#include "RHI/SamplerEnums.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericMaterial::FGenericMaterial(
    FString&& name,
    FString&& technique,
    FTagVector&& tags,
    FParameterMap&& parameters,
    FTextureMap&& textures )
:   _name(std::move(name))
,   _technique(std::move(technique))
,   _tags(std::move(tags))
,   _parameters(std::move(parameters))
,   _textures(std::move(textures)) {
    Assert(!_name.empty());
    Assert(!_technique.empty());
}
//----------------------------------------------------------------------------
void FGenericMaterial::AddTag(const FStringView& tag) {
    Assert(!tag.empty());

    Add_AssertUnique(_tags, ToString(tag));
}
//----------------------------------------------------------------------------
bool FGenericMaterial::HasTag(const FStringView& tag) const {
    Assert(!tag.empty());

    const auto it = std::find_if(_tags.begin(), _tags.end(), [&tag](const FString& value) {
        return EqualsI(MakeStringView(value), tag);
    });
    return (_tags.end() != it);
}
//----------------------------------------------------------------------------
bool FGenericMaterial::RemoveTag(const FStringView& tag) {
    const auto it = std::find_if(_tags.begin(), _tags.end(), [&tag](const FString& value) {
        return EqualsI(MakeStringView(value), tag);
    });
    if (it != _tags.end()) {
        _tags.erase(it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void FGenericMaterial::AddParameter(const FStringView& key, const Opaq::value_init& value) {
    Assert(!key.empty());

    _parameters.Insert_AssertUnique(ToString(key), FGenericParameterBlock{ value });
}
//----------------------------------------------------------------------------
void FGenericMaterial::SetParameter(const FStringView& key, const Opaq::value_init& value) {
    Assert(!key.empty());

    _parameters.Insert_Overwrite(ToString(key), FGenericParameterBlock{ value });
}
//----------------------------------------------------------------------------
FGenericParameterBlock& FGenericMaterial::GetParameterBlock(const FStringView& key) {
    Assert(!key.empty());

    const auto it = std::find_if(_parameters.begin(), _parameters.end(), [&key](const auto& it) {
        return EqualsI(MakeStringView(it.first), key);
    });
    AssertRelease(it != _parameters.end());
    return (it->second);
}
//----------------------------------------------------------------------------
FGenericParameterBlock* FGenericMaterial::GetParameterBlockIFP(const FStringView& key) {
    Assert(!key.empty());

    const auto it = std::find_if(_parameters.begin(), _parameters.end(), [&key](const auto& it) {
        return EqualsI(MakeStringView(it.first), key);
    });

    return (it != _parameters.end() ? &it->second : nullptr);
}
//----------------------------------------------------------------------------
bool FGenericMaterial::RemoveParameter(const FStringView& key) {
    Assert(!key.empty());

    const auto it = std::find_if(_parameters.begin(), _parameters.end(), [&key](const auto& it) {
        return EqualsI(MakeStringView(it.first), key);
    });

    if (_parameters.end() != it) {
        _parameters.Erase(it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void FGenericMaterial::AddTexture(
    const FStringView& key,
    const FFilename& filename,
    RHI::EImageView view,
    RHI::EAddressMode addressMode,
    RHI::ETextureFilter filter ) {
    AddTexture(key, {
        filename,
        view,
        addressMode,
        filter
    });
}
//----------------------------------------------------------------------------
void FGenericMaterial::AddTexture2D(const FStringView& key, const FFilename& filename) {
    AddTexture(key, filename,
        RHI::EImageView_2D,
        RHI::EAddressMode::Repeat,
        RHI::ETextureFilter::Linear );
}
//----------------------------------------------------------------------------
void FGenericMaterial::AddTexture3D(const FStringView& key, const FFilename& filename) {
    AddTexture(key, filename,
        RHI::EImageView_3D,
        RHI::EAddressMode::Repeat,
        RHI::ETextureFilter::Linear );
}
//----------------------------------------------------------------------------
void FGenericMaterial::AddTextureCube(const FStringView& key, const FFilename& filename) {
    AddTexture(key, filename,
        RHI::EImageView_Cube,
        RHI::EAddressMode::Repeat,
        RHI::ETextureFilter::Linear );
}
//----------------------------------------------------------------------------
void FGenericMaterial::AddTexture(const FStringView& key, const FGenericTextureSampler& sampler) {
    Assert(!key.empty());
    Assert(!sampler.Filename.empty());

    _textures.Insert_AssertUnique(ToString(key), sampler);
}
//----------------------------------------------------------------------------
const FGenericTextureSampler& FGenericMaterial::GetTexture(const FStringView& key) const {
    Assert(!key.empty());

    const auto it = std::find_if(_textures.begin(), _textures.end(), [&key](const auto& it) {
        return EqualsI(MakeStringView(it.first), key);
    });
    AssertRelease(it != _textures.end());
    return (it->second);
}
//----------------------------------------------------------------------------
const FGenericTextureSampler* FGenericMaterial::GetTextureIFP(const FStringView& key) const {
    Assert(!key.empty());

    const auto it = std::find_if(_textures.begin(), _textures.end(), [&key](const auto& it) {
        return EqualsI(MakeStringView(it.first), key);
    });

    return (it != _textures.end() ? &it->second : nullptr);
}
//----------------------------------------------------------------------------
bool FGenericMaterial::RemoveTexture(const FStringView& key) {
    Assert(!key.empty());

    const auto it = std::find_if(_textures.begin(), _textures.end(), [&key](const auto& it) {
        return EqualsI(MakeStringView(it.first), key);
    });

    if (_textures.end() != it) {
        _textures.Erase(it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
