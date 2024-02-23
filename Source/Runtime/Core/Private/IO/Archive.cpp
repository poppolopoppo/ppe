// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/Archive.h"

#include "IO/ArchiveHelpers.h"

#include "Container/RawStorage.h"
#include "IO/Basename.h"
#include "IO/BasenameNoExt.h"
#include "IO/Dirpath.h"
#include "IO/Extname.h"
#include "IO/Filename.h"
#include "IO/String.h"
#include "Memory/Compression.h"
#include "Memory/SharedBuffer.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FArchive::SerializeCompactInt(TCompactInt<i16>* value) {
	SerializeCompactSigned<i16>(*this, value);
}
//----------------------------------------------------------------------------
void FArchive::SerializeCompactInt(TCompactInt<i32>* value){
	SerializeCompactSigned<i32>(*this, value);
}
//----------------------------------------------------------------------------
void FArchive::SerializeCompactInt(TCompactInt<i64>* value){
	SerializeCompactSigned<i64>(*this, value);
}
//----------------------------------------------------------------------------
void FArchive::SerializeCompactInt(TCompactInt<u16>* value) {
	SerializeCompactUnsigned<u16>(*this, value);
}
//----------------------------------------------------------------------------
void FArchive::SerializeCompactInt(TCompactInt<u32>* value){
	SerializeCompactUnsigned<u32>(*this, value);
}
//----------------------------------------------------------------------------
void FArchive::SerializeCompactInt(TCompactInt<u64>* value){
	SerializeCompactUnsigned<u64>(*this, value);
}
//----------------------------------------------------------------------------
void FArchive::SerializeString(FString* str) {
	SerializeResizableArray(*this, MakeResizable(str));
}
//----------------------------------------------------------------------------
void FArchive::SerializeString(FWString* str) {
	SerializeResizableArray(*this, MakeResizable(str));
}
//----------------------------------------------------------------------------
void FArchive::SerializeBuffer(FUniqueBuffer* buffer) {
	SerializeResizableArray(*this, MakeResizable(buffer));
}
//----------------------------------------------------------------------------
void FArchive::SerializeFilesystem(FBasenameNoExt* value) {
	SerializeToken(*this, value);
}
//----------------------------------------------------------------------------
void FArchive::SerializeFilesystem(FBasename* value) {
	FBasenameNoExt basenameNoExt = value->BasenameNoExt();
	SerializeFilesystem(&basenameNoExt);

	FExtname extname = value->Extname();
	SerializeFilesystem(&extname);

	if (IsLoading())
		*value = {basenameNoExt, extname};
}
//----------------------------------------------------------------------------
void FArchive::SerializeFilesystem(FDirpath* value) {
	TCompactInt<u64> depth{ value->Depth() };
	SerializeCompactInt(&depth);

	if (IsSaving()) {
		for (const FFileSystemToken& it : value->ExpandTokens())
			SerializeToken(*this, const_cast<FFileSystemToken*>(&it/*won't be muted though*/));
	}
	else {
		STACKLOCAL_POD_ARRAY(FFileSystemToken, entries, *depth);
		for (FFileSystemToken& it : entries)
			SerializeToken(*this, &it);

		value->AssignTokens(entries);
	}
}
//----------------------------------------------------------------------------
void FArchive::SerializeFilesystem(FExtname* value) {
	SerializeToken(*this, value);
}
//----------------------------------------------------------------------------
void FArchive::SerializeFilesystem(FFilename* value) {
	FDirpath dirpath = value->Dirpath();
	SerializeFilesystem(&dirpath);

	FBasename basename = value->Basename();
	SerializeFilesystem(&basename);

	if (IsLoading())
		*value = {dirpath, basename};
}
//----------------------------------------------------------------------------
void FArchive::SerializeCompressed(FUniqueBuffer* buffer) {
	MEMORYDOMAIN_THREAD_SCOPE(Compress);
	RAWSTORAGE(Compress, u8) compressed;
	if (IsLoading()) {
		*this << &compressed;

		const size_t decompressedSize = Compression::DecompressedSize(compressed.MakeView());
		if (buffer->SizeInBytes() != decompressedSize)
			*buffer = FUniqueBuffer::Allocate(decompressedSize);

		VerifyRelease(Compression::DecompressMemory(buffer->MakeView(), compressed.MakeView()));
	}
	else {
		VerifyRelease(Compression::CompressMemory(compressed, buffer->MakeView()));

		*this << &compressed;
	}
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
