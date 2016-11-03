#include "stdafx.h"

#include "Header.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHttpHeader::FHttpHeader() {}
//----------------------------------------------------------------------------
FHttpHeader::~FHttpHeader() {}
//----------------------------------------------------------------------------
void FHttpHeader::Add(const FName& key, FString&& value) {
    Assert(not key.empty());
    Assert(not value.empty());

    _headers.GetOrAdd(key) = std::move(value);
}
//----------------------------------------------------------------------------
void FHttpHeader::Remove(const FName& key) {
    Assert(not key.empty());

    _headers.Remove_AssertExists(key);
}
//----------------------------------------------------------------------------
FStringView FHttpHeader::Get(const FName& key) const {
    Assert(not key.empty());

    return MakeStringView(_headers.At(key));
}
//----------------------------------------------------------------------------
FStringView FHttpHeader::GetIFP(const FName& key) const {
    Assert(not key.empty());

    const FString* pvalue = _headers.GetIFP(key);
    return (pvalue ? MakeStringView(*pvalue) : FStringView());
}
//----------------------------------------------------------------------------
void FHttpHeader::Clear() {
    _headers.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
