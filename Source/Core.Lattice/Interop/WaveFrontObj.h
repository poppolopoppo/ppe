#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core/Container/Vector.h"
#include "Core/IO/FileSystem_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class IBufferedStreamWriter;
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_LATTICE_API FWaveFrontObj {
public:
    static const FExtname& Ext;

    static bool Load(FGenericMesh* dst, const FFilename& filename);
    static bool Load(FGenericMesh* dst, const FFilename& filename, const FStringView& content);

    static bool Save(const FGenericMesh* src, const FFilename& filename);
    static bool Save(const FGenericMesh* src, const FFilename& filename, IBufferedStreamWriter* writer);

private:
    FWaveFrontObj() = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
