#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Serialize/Markup/Xml.h"

#include "Core/Container/Vector.h"
#include "Core/IO/FileSystem_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Lattice {
FWD_REFPTR(GenericMaterial);
FWD_REFPTR(GenericMesh);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FCollada {
public:
    template <typename T>
    using TArray = VECTOR(Collada, T);

    FCollada();
    ~FCollada();

    FCollada(const FCollada& ) = delete;
    FCollada& operator =(const FCollada& ) = delete;

    const Serialize::FXml& Xml() const { return _xml; }

    bool empty() const { return _xml.empty(); }

    bool ImportGeometries(TArray<PGenericMesh>& meshes) const;
    bool ImportMaterials(TArray<PGenericMaterial>& materials) const;

    static const FExtname& Ext;

    static bool Load(FCollada* pdst, const FFilename& filename);
    static bool Load(FCollada* pdst, const FFilename& filename, const FStringView& content);

    static void Start();
    static void Shutdown();

private:
    Serialize::FXml _xml;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
