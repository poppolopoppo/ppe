#pragma once

#include "Core.Serialize/XML/XML.h"

#include "Core.Serialize/XML/Name.h"

#include "Core/Container/StringHashMap.h"
#include "Core/IO/String.h"
#include "Core/IO/StringSlice.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace XML {
FWD_REFPTR(Element);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Document);
class Document : public RefCountable {
public:
    typedef STRINGSLICE_HASHMAP(XML, SElement, Case::Sensitive) byidentifier_type;

    Document();
    ~Document();

    Document(const Document& ) = delete;
    Document& operator =(const Document& ) = delete;

    Element* Root() { return _root.get(); }
    const Element* Root() const { return _root.get(); }
    void SetRoot(Element* element) { _root.reset(element); }

    const String& Version() const { return _version; }
    void Version(String&& value) { _version = std::move(value); }

    const String& Encoding() const { return _encoding; }
    void Encoding(String&& encoding) { _encoding = std::move(encoding); }

    const byidentifier_type& ByIdentifier() const { return _byIdentifier; }

    const Element* FindById(const StringSlice& Id) const;

    bool empty() const { return (nullptr != _root); }

    void ToStream(std::basic_ostream<char>& oss) const;

    const Element* XPath(const MemoryView<const Name>& path) const;
    size_t XPath(const MemoryView<const Name>& path, const std::function<void(const Element&)>& functor) const;

    static bool Load(Document* document, const Filename& filename);
    static bool Load(Document* document, const Filename& filename, const StringSlice& content);

private:
    PElement _root;

    String _version;
    String _encoding;
    String _standalone;

    byidentifier_type _byIdentifier;
};
//----------------------------------------------------------------------------
inline std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const Document& doc) {
    doc.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
