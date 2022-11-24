// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "BulkMesh.h"

#include "GenericMaterial.h"
#include "GenericMesh.h"

#include "Device/Geometry/VertexDeclaration.h"

#include "Container/Vector.h"
#include "IO/BufferedStreamProvider.h"
#include "IO/FS/ConstNames.h"
#include "VirtualFileSystemStream.h"
#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/VirtualFileSystem.h"
#include "Maths/ScalarVector.h"
#include "Memory/MemoryProvider.h"
#include "Memory/MemoryStream.h"
#include "Misc/FourCC.h"

namespace PPE {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FExtname& FBulkMesh::Ext = FFSConstNames::Bkm();
//----------------------------------------------------------------------------
namespace {
static const FFourCC FILE_MAGIC_        ("BLKM");
static const FFourCC FILE_VERSION_      ("1.00");
static const FFourCC SECTION_INDICES_   ("IDCS");
static const FFourCC SECTION_VERTICES_  ("VTCS");
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FBulkMesh::Load(FGenericMesh* dst, const FFilename& filename) {
    EAccessPolicy policy = EAccessPolicy::Binary;
    if (filename.Extname() == FFSConstNames::Bkmz()) {
        policy = policy + EAccessPolicy::Compress;

        RAWSTORAGE(FileSystem, u8) content;
        if (false == VFS_ReadAll(&content, filename, policy))
            return false;

        return Load(dst, filename, content.MakeConstView());
    }
    else {
        const TUniquePtr<IVirtualFileSystemIStream> readable =
            VFS_OpenReadable(filename, policy);

        FBufferedStreamReader buffered(readable.get());
        return (readable && Load(dst, filename, &buffered));
    }
}
//----------------------------------------------------------------------------
bool FBulkMesh::Load(FGenericMesh* dst, const FFilename& filename, const TMemoryView<const u8>& content) {
    Assert(dst);
    if (content.empty())
        return false;

    FMemoryViewReader reader(content);
    return Load(dst, filename, &reader);
}
//----------------------------------------------------------------------------
bool FBulkMesh::Load(FGenericMesh* dst, const FFilename& filename, IBufferedStreamReader* reader) {
    Assert(dst);

    // Test magic and version :
    if (!reader->ExpectPOD(FILE_MAGIC_) ||
        !reader->ExpectPOD(FILE_VERSION_) )
        return false;

    u32 indexCount = 0, vertexCount = 0, subpartCount = 0;
    if (!reader->ReadPOD(&indexCount) ||
        !reader->ReadPOD(&vertexCount) ||
        !reader->ReadPOD(&subpartCount) )
        return false;

    dst->Resize(indexCount, vertexCount, false);

    if (!reader->ExpectPOD(SECTION_INDICES_) ||
        !reader->ReadView(dst->Indices()) )
        return false;

    char semanticCstr[32];
    forrange(s, 0, subpartCount) {
        if (!reader->ExpectPOD(SECTION_VERTICES_))
            return false;

        u32 type = 0, index = 0, length = 0;
        if (!reader->ReadPOD(&type) ||
            !reader->ReadPOD(&index) ||
            !reader->ReadPOD(&length))
            return false;

        AssertRelease(length < lengthof(semanticCstr));
        const auto semanticBuf = MakeView(semanticCstr).FirstNElements(length);
        if (!reader->ReadView(semanticBuf))
            return false;

        const Graphics::FName semantic(semanticBuf);
        FGenericVertexData* const vertexData = dst->AddVertexData(
            Graphics::FVertexSemantic::FromName(semantic),
            index, Graphics::EValueType(type) );

        vertexData->Resize(vertexCount);
        if (!reader->ReadView(vertexData->MakeView()))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FBulkMesh::Save(const FGenericMesh* src, const FFilename& filename) {
    Assert(src);

    EAccessPolicy policy = EAccessPolicy::Truncate_Binary;

    if (filename.Extname() == FFSConstNames::Bkmz()) {
        MEMORYSTREAM(Image) writer;
        if (false == Save(src, filename, &writer))
            return false;

        policy = policy + EAccessPolicy::Compress;

        return VFS_WriteAll(filename, writer.MakeView(), policy);
    }
    else {
        const TUniquePtr<IVirtualFileSystemOStream> writable =
            VFS_OpenWritable(filename, policy);

        if (not writable)
            return false;

        bool result = false;
        UsingBufferedStream(writable.get(), [src, &filename, &result](IBufferedStreamWriter* buffered) {
            result = Save(src, filename, buffered);
        });

        return result;
    }
}
//----------------------------------------------------------------------------
bool FBulkMesh::Save(const FGenericMesh* src, const FFilename& filename, IBufferedStreamWriter* writer) {
    Assert(src);
    Assert(writer);

    writer->WritePOD(FILE_MAGIC_);
    writer->WritePOD(FILE_VERSION_);

    writer->WritePOD(checked_cast<u32>(src->IndexCount()));
    writer->WritePOD(checked_cast<u32>(src->VertexCount()));
    writer->WritePOD(checked_cast<u32>(src->SubPartCount()));

    writer->WritePOD(SECTION_INDICES_);
    writer->WriteView(src->Indices());

    for (const UGenericVertexData& subPart : src->Vertices()) {
        writer->WritePOD(SECTION_VERTICES_);
        writer->WritePOD(u32(subPart->Type()));
        writer->WritePOD(checked_cast<u32>(subPart->Index()));
        writer->WritePOD(checked_cast<u32>(subPart->Semantic().size()));
        writer->WriteView(subPart->Semantic().MakeView());
        writer->WriteView(subPart->MakeView());
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE

