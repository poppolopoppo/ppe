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
class FMaterial : public FRefCountable {
public:
    explicit FMaterial(const Graphics::FBindName& name);
    FMaterial(   const Graphics::FBindName& name,
                const FString& description,
                VECTOR(FMaterial, Graphics::FBindName)&& tags,
                ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, FFilename)&& textures,
                ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, PMaterialParameter)&& parameters );
    ~FMaterial();

    const Graphics::FBindName& FName() const { return _name; }
    const FString& Description() const { return _description; }

    const VECTOR(FMaterial, Graphics::FBindName)& Tags() const { return _tags; }
    const ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, FFilename)& Textures() const { return _textures; }
    const ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, PMaterialParameter)& Parameters() const { return _parameters; }

    void AddTag(const Graphics::FBindName& name);
    void AddTexture(const Graphics::FBindName& name, const FFilename& filename);
    void AddParameter(const Graphics::FBindName& name, const PMaterialParameter& parameter);

    void SetTexture(const Graphics::FBindName& name, const FFilename& filename);
    void SetParameter(const Graphics::FBindName& name, const PMaterialParameter& parameter);

    bool Equals(const FMaterial& other) const;

    bool operator ==(const FMaterial& other) const { return Equals(other); }
    bool operator !=(const FMaterial& other) const { return !operator ==(other); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    Graphics::FBindName _name;
    FString _description;
    VECTOR(FMaterial, Graphics::FBindName) _tags;
    ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, FFilename) _textures;
    ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, PMaterialParameter) _parameters;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
