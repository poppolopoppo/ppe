#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Name.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace Network {
enum class EHttpMethod;
enum class EHttpStatus;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpHeader {
public:
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FName, FString, 9/* according to Chrome */) FEntries;

    FHttpHeader();
    ~FHttpHeader();

    FHttpHeader(const FHttpHeader& ) = delete;
    FHttpHeader& operator =(const FHttpHeader& ) = delete;

    const FEntries& Headers() const { return _headers; }

    void Add(const FName& key, FString&& value);
    void Remove(const FName& key);

    FStringView Get(const FName& key) const;
    FStringView GetIFP(const FName& key) const;

    FStringView operator[](const FName& key) const { return GetIFP(key); }

    void Clear();

    static bool Read(FHttpHeader* pheader, FSocketBuffered& socket);

    static FStringView ProtocolVersion();

private:
    FEntries _headers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
