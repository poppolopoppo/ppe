// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/BasenameNoExt.h"

#include "Allocator/Alloca.h"
#include "HAL/PlatformMemory.h"
#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasenameNoExt::FBasenameNoExt(const FileSystem::FStringView& content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
FBasenameNoExt& FBasenameNoExt::operator =(const FileSystem::FStringView& content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
FBasenameNoExt::FBasenameNoExt(const FileSystem::FString& content)
    : FBasenameNoExt(content.MakeView())
{}
//----------------------------------------------------------------------------
void FBasenameNoExt::Clear() NOEXCEPT {
    parent_type::Clear();
}
//----------------------------------------------------------------------------
void FBasenameNoExt::Swap(FBasenameNoExt& other) NOEXCEPT {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasenameNoExt operator +(const FBasenameNoExt& lhs, const FileSystem::FStringView& rhs) {
    const size_t len = (lhs.size() + rhs.size());
    Assert(len);

    STACKLOCAL_POD_ARRAY(FileSystem::char_type, concat, len);
    FPlatformMemory::Memcpy(concat.data(), lhs.MakeView().data(), lhs.MakeView().SizeInBytes());
    FPlatformMemory::Memcpy(concat.data() + lhs.size(), rhs.data(), rhs.SizeInBytes());

    return FBasenameNoExt(FileSystem::FStringView(concat));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
