#pragma once

#include "Lattice.h"

#include "XML/XML.h"

#include "Container/Vector.h"
#include "IO/FileSystem_fwd.h"
#include "Memory/RefPtr.h"

namespace PPE {
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

    FCollada() : FCollada(nullptr) {}
    explicit FCollada(XML::FDocument* xml);
    ~FCollada();

    FCollada(const FCollada& ) = delete;
    FCollada& operator =(const FCollada& ) = delete;

    const XML::FDocument* Xml() const { return _xml.get(); }

    bool empty() const { return (nullptr == _xml); }

    bool ImportGeometries(TArray<PGenericMesh>& meshes) const;
    bool ImportMaterials(TArray<PGenericMaterial>& materials) const;

    static const FExtname& Ext;

    static bool Load(FCollada* pdst, const FFilename& filename);
    static bool Load(FCollada* pdst, const FFilename& filename, const FStringView& content);

    static void Start();
    static void Shutdown();

private:
    XML::PDocument _xml;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE
