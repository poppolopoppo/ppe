#pragma once

#include "Core.Serialize/XML/XML.h"

#include "Core.Serialize/XML/Name.h"

#include "Core/Container/StringHashMap.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/IO/VirtualFileSystem_fwd.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Function.h"

namespace Core {
class IBufferedStreamReader;
namespace XML {
FWD_REFPTR(Element);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Document);
class FDocument : public FRefCountable {
public:
    typedef STRINGVIEW_HASHMAP(XML, SElement, ECase::Sensitive) byidentifier_type;

    FDocument();
    ~FDocument();

    FDocument(const FDocument& ) = delete;
    FDocument& operator =(const FDocument& ) = delete;

    FElement* Root() { return _root.get(); }
    const FElement* Root() const { return _root.get(); }
    void SetRoot(FElement* element) { _root.reset(element); }

    const FString& Version() const { return _version; }
    void Version(FString&& value) { _version = std::move(value); }

    const FString& Encoding() const { return _encoding; }
    void Encoding(FString&& encoding) { _encoding = std::move(encoding); }

    const byidentifier_type& ByIdentifier() const { return _byIdentifier; }

    const FElement* FindById(const FStringView& Id) const;

    bool empty() const { return (nullptr != _root); }

    void ToStream(FTextWriter& oss) const;

    const FElement* XPath(const TMemoryView<const FName>& path) const;
    size_t XPath(const TMemoryView<const FName>& path, const Meta::TFunction<void(const FElement&)>& functor) const;

    static bool Load(FDocument* document, const FFilename& filename);
    static bool Load(FDocument* document, const FFilename& filename, IBufferedStreamReader* input);
    static bool Load(FDocument* document, const FFilename& filename, const FStringView& content);

private:
    PElement _root;

    FString _version;
    FString _encoding;
    FString _standalone;

    byidentifier_type _byIdentifier;
};
//----------------------------------------------------------------------------
inline FTextWriter& operator <<(FTextWriter& oss, const FDocument& doc) {
    doc.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
