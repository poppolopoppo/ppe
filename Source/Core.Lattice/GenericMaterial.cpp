#include "stdafx.h"

#include "GenericMaterial.h"

#include "Lattice_fwd.h"

#include "Core.Graphics/Device/State/SamplerState.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Lattice, GenericMaterial, );
//----------------------------------------------------------------------------
GenericMaterial::GenericMaterial() {}
//----------------------------------------------------------------------------
GenericMaterial::GenericMaterial(
    String&& name,
    String&& technique,
    TagVector&& tags,
    ParameterMap&& parameters,
    TextureMap&& textures )
:   _name(std::move(name))
,   _technique(std::move(technique))
,   _tags(std::move(tags))
,   _parameters(std::move(parameters))
,   _textures(std::move(textures)) {
    Assert(!_name.empty());
    Assert(!_technique.empty());
}
//----------------------------------------------------------------------------
GenericMaterial::~GenericMaterial() {}
//----------------------------------------------------------------------------
void GenericMaterial::AddTag(const StringSlice& tag) {
    Assert(!tag.empty());

    Add_AssertUnique(_tags, ToString(tag));
}
//----------------------------------------------------------------------------
bool GenericMaterial::HasTag(const StringSlice& tag) const {
    Assert(!tag.empty());

    const auto it = std::find_if(_tags.begin(), _tags.end(), [&tag](const String& value) {
        return EqualsI(MakeStringSlice(value), tag);
    });
    return (_tags.end() != it);
}
//----------------------------------------------------------------------------
bool GenericMaterial::RemoveTag(const StringSlice& tag) {
    const auto it = std::find_if(_tags.begin(), _tags.end(), [&tag](const String& value) {
        return EqualsI(MakeStringSlice(value), tag);
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
void GenericMaterial::AddParameter(const StringSlice& key, const Graphics::Value& value) {
    Assert(!key.empty());
    Assert(!value.empty());

    _parameters.Insert_AssertUnique(ToString(key), value);
}
//----------------------------------------------------------------------------
void GenericMaterial::SetParameter(const StringSlice& key, const Graphics::Value& value) {
    Assert(!key.empty());
    Assert(!value.empty());

    _parameters.GetOrAdd(ToString(key)) = value;
}
//----------------------------------------------------------------------------
Graphics::Value& GenericMaterial::GetParameterValue(const StringSlice& key) {
    Assert(!key.empty());

    const auto it = std::find_if(_parameters.begin(), _parameters.end(), [&key](const auto& it) {
        return EqualsI(MakeStringSlice(it.first), key);
    });
    AssertRelease(it != _parameters.end());
    return (it->second);
}
//----------------------------------------------------------------------------
Graphics::Value* GenericMaterial::GetParameterValueIFP(const StringSlice& key) {
    Assert(!key.empty());

    const auto it = std::find_if(_parameters.begin(), _parameters.end(), [&key](const auto& it) {
        return EqualsI(MakeStringSlice(it.first), key);
    });

    return (it != _parameters.end() ? &it->second : nullptr);
}
//----------------------------------------------------------------------------
bool GenericMaterial::RemoveParameter(const StringSlice& key) {
    Assert(!key.empty());

    const auto it = std::find_if(_parameters.begin(), _parameters.end(), [&key](const auto& it) {
        return EqualsI(MakeStringSlice(it.first), key);
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
void GenericMaterial::AddTexture(
    const StringSlice& key,
    const Filename& filename,
    Graphics::TextureDimension dimension,
    Graphics::TextureAddressMode addressMode,
    Graphics::TextureFilter filter ) {
    const GenericTextureSampler sampler{
        filename,
        dimension,
        addressMode,
        filter
    };
    AddTexture(key, sampler);
}
//----------------------------------------------------------------------------
void GenericMaterial::AddTexture2D(const StringSlice& key, const Filename& filename) {
    AddTexture(key, filename,
        Graphics::TextureDimension::Texture2D,
        Graphics::TextureAddressMode::Wrap,
        Graphics::TextureFilter::Linear );
}
//----------------------------------------------------------------------------
void GenericMaterial::AddTexture3D(const StringSlice& key, const Filename& filename) {
    AddTexture(key, filename,
        Graphics::TextureDimension::Texture3D,
        Graphics::TextureAddressMode::Wrap,
        Graphics::TextureFilter::Linear );
}
//----------------------------------------------------------------------------
void GenericMaterial::AddTextureCube(const StringSlice& key, const Filename& filename) {
    AddTexture(key, filename,
        Graphics::TextureDimension::TextureCube,
        Graphics::TextureAddressMode::Wrap,
        Graphics::TextureFilter::Linear );
}
//----------------------------------------------------------------------------
void GenericMaterial::AddTexture(const StringSlice& key, const GenericTextureSampler& sampler) {
    Assert(!key.empty());
    Assert(!sampler.Filename.empty());

    _textures.Insert_AssertUnique(ToString(key), sampler);
}
//----------------------------------------------------------------------------
const GenericTextureSampler& GenericMaterial::GetTexture(const StringSlice& key) const {
    Assert(!key.empty());

    const auto it = std::find_if(_textures.begin(), _textures.end(), [&key](const auto& it) {
        return EqualsI(MakeStringSlice(it.first), key);
    });
    AssertRelease(it != _textures.end());
    return (it->second);
}
//----------------------------------------------------------------------------
const GenericTextureSampler* GenericMaterial::GetTextureIFP(const StringSlice& key) const {
    Assert(!key.empty());

    const auto it = std::find_if(_textures.begin(), _textures.end(), [&key](const auto& it) {
        return EqualsI(MakeStringSlice(it.first), key);
    });

    return (it != _textures.end() ? &it->second : nullptr);
}
//----------------------------------------------------------------------------
bool GenericMaterial::RemoveTexture(const StringSlice& key) {
    Assert(!key.empty());

    const auto it = std::find_if(_textures.begin(), _textures.end(), [&key](const auto& it) {
        return EqualsI(MakeStringSlice(it.first), key);
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
} //!namespace Lattice
} //!namespace Core
