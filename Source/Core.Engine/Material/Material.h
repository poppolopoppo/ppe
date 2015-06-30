#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/MaterialParameter_fwd.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

#include "Core/Allocator/PoolAllocator.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Material);
class Material : public RefCountable {
public:
    explicit Material(const Graphics::BindName& name);
    Material(   const Graphics::BindName& name,
                const String& description,
                VECTOR(Material, Graphics::BindName)&& tags,
                ASSOCIATIVE_VECTOR(Material, Graphics::BindName, Filename)&& textures,
                ASSOCIATIVE_VECTOR(Material, Graphics::BindName, PMaterialParameter)&& parameters );
    ~Material();

    const Graphics::BindName& Name() const { return _name; }
    const String& Description() const { return _description; }

    const VECTOR(Material, Graphics::BindName)& Tags() const { return _tags; }
    const ASSOCIATIVE_VECTOR(Material, Graphics::BindName, Filename)& Textures() const { return _textures; }
    const ASSOCIATIVE_VECTOR(Material, Graphics::BindName, PMaterialParameter)& Parameters() const { return _parameters; }

    void AddTag(const Graphics::BindName& name);
    void AddTexture(const Graphics::BindName& name, const Filename& filename);
    void AddParameter(const Graphics::BindName& name, const PMaterialParameter& parameter);

    void SetTexture(const Graphics::BindName& name, const Filename& filename);
    void SetParameter(const Graphics::BindName& name, const PMaterialParameter& parameter);

    bool Equals(const Material& other) const;

    bool operator ==(const Material& other) const { return Equals(other); }
    bool operator !=(const Material& other) const { return !operator ==(other); }

    SINGLETON_POOL_ALLOCATED_DECL(Material);

private:
    Graphics::BindName _name;
    String _description;
    VECTOR(Material, Graphics::BindName) _tags;
    ASSOCIATIVE_VECTOR(Material, Graphics::BindName, Filename) _textures;
    ASSOCIATIVE_VECTOR(Material, Graphics::BindName, PMaterialParameter) _parameters;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
