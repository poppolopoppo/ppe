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
enum class TextureAddressMode;
enum class TextureDimension;
enum class TextureFilter;
}

namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct GenericTextureSampler {
    Filename Filename;
    Graphics::TextureDimension Dimension;
    Graphics::TextureAddressMode AddressMode;
    Graphics::TextureFilter Filter;
};
//----------------------------------------------------------------------------
FWD_REFPTR(GenericMaterial);
class GenericMaterial : public RefCountable {
public:
    typedef VECTORINSITU(GenericMaterial, String, 3) TagVector;
    typedef ASSOCIATIVE_VECTORINSITU(GenericMaterial, String, Graphics::Value, 3) ParameterMap;
    typedef ASSOCIATIVE_VECTORINSITU(GenericMaterial, String, GenericTextureSampler, 3) TextureMap;

    GenericMaterial();
    GenericMaterial(String&& name,
                    String&& technique,
                    TagVector&& tags,
                    ParameterMap&& parameters,
                    TextureMap&& textures );
    ~GenericMaterial();

    GenericMaterial(GenericMaterial&& rvalue) = default;
    GenericMaterial& operator =(GenericMaterial&& rvalue) = default;

    GenericMaterial(const GenericMaterial& ) = delete;
    GenericMaterial& operator =(const GenericMaterial& ) = delete;

    const String& Name() const { return _name; }
    void SetName(String&& rvalue) { Assert(!rvalue.empty()); _name = std::move(rvalue); }
    void SetName(const String& value) { Assert(!value.empty()); _name = value; }
    void SetName(const StringView& value) { Assert(!value.empty()); _name = ToString(value); }

    const String& Technique() const { return _technique; }
    void SetTechnique(String&& rvalue) { Assert(!rvalue.empty()); _technique = std::move(rvalue); }
    void SetTechnique(const String& value) { Assert(!value.empty()); _technique = value; }
    void SetTechnique(const StringView& value) { Assert(!value.empty()); _technique = ToString(value); }

    const ParameterMap& Parameters() const { return _parameters; }
    void SetParameters(ParameterMap&& rvalue) { _parameters = std::move(rvalue); }

    const TextureMap& Textures() const { return _textures; }
    void SetTextures(TextureMap&& rvalue) { _textures = std::move(rvalue); }

    void AddTag(const StringView& tag);
    bool HasTag(const StringView& tag) const;
    bool RemoveTag(const StringView& tag);

    void AddParameter(const StringView& key, const Graphics::Value& value); // must be unique
    void SetParameter(const StringView& key, const Graphics::Value& value); // will override existing value
    Graphics::Value& GetParameterValue(const StringView& key);
    Graphics::Value* GetParameterValueIFP(const StringView& key);
    const Graphics::Value& GetParameterValue(const StringView& key) const { return remove_const(this)->GetParameterValue(key); }
    const Graphics::Value* GetParameterValueIFP(const StringView& key) const { return remove_const(this)->GetParameterValueIFP(key); }
    bool RemoveParameter(const StringView& key);

    template <typename T> T& GetParameter(const StringView& key) { return GetParameterValue(key).Get<T>(); }
    template <typename T> T* GetParameterIFP(const StringView& key) { return auto v = GetParameterValueIFP(key), (v ? v->Get<T>() : nullptr); }
    template <typename T> const T& GetParameter(const StringView& key) const { return GetParameterValue(key).Get<T>(); }
    template <typename T> const T* GetParameterIFP(const StringView& key) const { return auto v = GetParameterValueIFP(key), (v ? v->Get<T>() : nullptr); }

    void AddTexture(const StringView& key,
                    const Filename& filename,
                    Graphics::TextureDimension dimension,
                    Graphics::TextureAddressMode addressMode,
                    Graphics::TextureFilter filter );
    void AddTexture2D(const StringView& key, const Filename& filename);
    void AddTexture3D(const StringView& key, const Filename& filename);
    void AddTextureCube(const StringView& key, const Filename& filename);
    void AddTexture(const StringView& key, const GenericTextureSampler& sampler);
    const GenericTextureSampler& GetTexture(const StringView& key) const;
    const GenericTextureSampler* GetTextureIFP(const StringView& key) const;
    bool RemoveTexture(const StringView& key);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    String _name;
    String _technique;
    TagVector _tags;
    ParameterMap _parameters;
    TextureMap _textures;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
