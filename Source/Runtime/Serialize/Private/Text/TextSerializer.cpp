// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Text/TextSerializer.h"

#include "Text/Grammar.h"

#include "Lexer/Lexer.h"

#include "Parser/ParseExpression.h"
#include "Parser/ParseItem.h"
#include "Parser/ParseList.h"

#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"
#include "RTTI/Any.h"
#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"

#include "Container/HashMap.h"
#include "Container/HashSet.h"
#include "Container/Vector.h"
#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryStream.h"

#include "TransactionLinker.h"
#include "TransactionSaver.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FTextSerialize_ : public RTTI::IAtomVisitor {
public:
    FTextSerialize_(IBufferedStreamWriter& ostream, bool minify)
    :   _oss(&ostream)
    ,   _indent(minify ? FStringView() : MakeStringView("  "))
    ,   _newLine(minify ? ' ' : '\n')
    {}

    void Append(const TMemoryView<const RTTI::SMetaObject>& refs) {
        _visiteds.reserve_Additional(refs.size());

        for (const RTTI::SMetaObject& ref : refs)
            Append(ref.get());
    }

    void Append(const RTTI::FMetaObject* ref) {
        Assert(ref);

        FString name;
        if (ref->RTTI_IsExported()) {
            Assert_NoAssume(not ref->RTTI_Name().empty());

            name = ref->RTTI_Name().MakeView();
        }
        else { // generate temporary name (for internal references)
            Assert_NoAssume(ref->RTTI_Name().empty());

            name = StringFormat("{0}_{1}", ref->RTTI_Class()->Name(), _visiteds.size());
        }

        Insert_AssertUnique(_visiteds, ref, name);

        // definition :
        if (ref->RTTI_IsExported())
            _oss << "export ";
        _oss << name;

        const RTTI::FMetaClass* const klass = ref->RTTI_Class();
        Assert(klass);

        _oss << " is " << klass->Name() << " {";

        _indent.Inc();

        bool hasProperties = false;
        for (const RTTI::FMetaProperty* prop : klass->AllProperties()) {
            const RTTI::FAtom value = prop->Get(*ref);
            Assert(value);

            if (value.IsDefaultValue())
                continue;

            if (not hasProperties) {
                hasProperties = true;
                _oss << _newLine;
            }

            _oss << _indent << prop->Name() << " = ";
            value.Accept(this);
            _oss << _newLine;
        }

        _indent.Dec();

        if (hasProperties)
            _oss << _indent;

        _oss << '}' << _newLine;
    }

public: //RTTI::IAtomVisitor
    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        _oss << '(';

        auto sep = Fmt::NotFirstTime(", ");
        tuple->ForEach(data, [this, &sep](const RTTI::FAtom& elt) {
            _oss << sep;
            elt.Accept(this);

            return true;
        });

        _oss << ')';

        return true;
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        const size_t n = list->Count(data);

        if (0 == n) {
            _oss << "[]";
            return true;
        }

        _oss << '[' << _newLine;
        _indent.Inc();

        auto sep = Fmt::NotLastTime(',', n);
        list->ForEach(data, [this, &sep](const RTTI::FAtom& item) {
            _oss << _indent;
            item.Accept(this);
            _oss << sep << _newLine;

            return true;
        });

        _indent.Dec();
        _oss << _indent << ']';

        return true;
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override final {
        const size_t n = dico->Count(data);

        if (0 == n) {
            _oss << "{}";
            return true;
        }

        _oss << '{' << _newLine;
        _indent.Inc();

        auto sep = Fmt::NotLastTime(',', n);
        dico->ForEach(data, [this, &sep](const RTTI::FAtom& key, const RTTI::FAtom& value) {
            _oss << _indent << '(';
            key.Accept(this);
            _oss << ", ";
            value.Accept(this);
            _oss << ')' << sep << _newLine;

            return true;
        });

        _indent.Dec();
        _oss << _indent << '}';

        return true;
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const RTTI::IScalarTraits* , T& value) override final { \
        Print_(value); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    void Print_(const RTTI::FAny& any) {
        AssertRelease(any); // not implemented : "null" or none ?

        RTTI::FAtom inner = any.InnerAtom();
        _oss << inner.TypeName() << ':';
        inner.Accept(this);
    }

    void Print_(const RTTI::PMetaObject& obj) {
        if (not obj) {
            _oss << "null";
            return;
        }

        const auto it = _visiteds.find(RTTI::SMetaObject{ obj });
        if (_visiteds.end() == it) {
            // import
            _oss << RTTI::FPathName::FromObject(*obj);
        }
        else {
            // internal
            _oss << it->second;
        }
    }

    void Print_(bool b) { _oss << (b ? "true" : "false"); }

    void Print_(i8 ch) { _oss << int(ch); }
    void Print_(u8 uch) { _oss << unsigned(uch) << 'u'; }
    void Print_(u16 u_16) { _oss << u_16 << 'u'; }
    void Print_(u32 u_32) { _oss << u_32 << 'u'; }
    void Print_(u64 u_64) { _oss << u_64 << 'u'; }

    void Print_(const FString& str) {
        _oss << '"';
        Escape(_oss, str, EEscape::Hexadecimal);
        _oss << '"';
    }
    void Print_(const FWString& wstr) {
        Print_(ToString(wstr));
    }

    void Print_(const FDirpath& dirpath) { Print_(dirpath.ToString()); }
    void Print_(const FFilename& filename) { Print_(filename.ToString()); }

    void Print_(const RTTI::FName& name) {
        _oss << '\"' << name << '"';
    }

    void Print_(const RTTI::FBinaryData& binaryData) {
        _oss << '"' << binaryData << '"';
    }

    template <typename T>
    void Print_(const T& value) {
        _oss << value;
    }

    FTextWriter _oss;
    Fmt::FIndent _indent;
    char _newLine;

    HASHMAP(Transient, const RTTI::FMetaObject*, FString) _visiteds;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextSerializer::FTextSerializer(bool minify/* = true */) {
    SetMinify(minify);
}
//----------------------------------------------------------------------------
void FTextSerializer::Deserialize(IStreamReader& input, FTransactionLinker* linker) const {
    Assert(linker);

    const FWString fname = linker->Filename().ToWString();

    Parser::FParseList parseList;
    UsingBufferedStream(&input, [&parseList, &fname](IBufferedStreamReader* buffered) {
        Lexer::FLexer lexer(*buffered, fname, true);
        VerifyRelease(parseList.Parse(&lexer));
    });

    Parser::FParseContext parseContext(Meta::ForceInit);

    while (Parser::PCParseExpression expr = FGrammarStartup::ParseExpression(parseList)) {
        const RTTI::FAtom atom = expr->Eval(&parseContext);
        const RTTI::PMetaObject* const ppobj = atom.TypedConstDataIFP<RTTI::PMetaObject>();

        if (ppobj && *ppobj)
            linker->AddTopObject(ppobj->get());
    }

    for (const auto& it : parseContext.GlobalScope()->LocalScope()) {
        if (const RTTI::PMetaObject* exported = it.second.TypedDataIFP<RTTI::PMetaObject>())
            linker->AddExport(it.first, *exported);
    }
}
//----------------------------------------------------------------------------
void FTextSerializer::Serialize(const FTransactionSaver& saver, IStreamWriter* output) const {
    Assert(output);

    UsingBufferedStream(output, [this, &saver](IBufferedStreamWriter* buffered) {
        FTextSerialize_ visitor(*buffered, Minify());
        visitor.Append(saver.LoadedRefs());
    });
}
//----------------------------------------------------------------------------
FExtname FTextSerializer::Extname() {
    return FFSConstNames::Txt();
}
//----------------------------------------------------------------------------
USerializer FTextSerializer::Get() {
    return FTextSerializer();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
