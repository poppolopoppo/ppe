#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Serialize/XML/XML.h"

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
class Collada {
public:
    template <typename T>
    using Array = VECTOR_THREAD_LOCAL(Collada, T);

    Collada() : Collada(nullptr) {}
    explicit Collada(XML::Document* xml);
    ~Collada();

    Collada(const Collada& ) = delete;
    Collada& operator =(const Collada& ) = delete;

    const XML::Document* Xml() const { return _xml.get(); }

    bool empty() const { return (nullptr == _xml); }

    bool ImportGeometries(Array<PGenericMesh>& meshes) const;
    bool ImportMaterials(Array<PGenericMaterial>& materials) const;

    static const Extname& Ext;

    static bool Load(Collada* pdst, const Filename& filename);
    static bool Load(Collada* pdst, const Filename& filename, const StringView& content);

    static void Start();
    static void Shutdown();

private:
    XML::PDocument _xml;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
