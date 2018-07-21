#include "stdafx.h"

#include "TextSerializer.h"

#include "Grammar.h"

#include "Lexer/Lexer.h"

#include "Parser/ParseExpression.h"
#include "Parser/ParseItem.h"
#include "Parser/ParseList.h"

#include "Core.RTTI/Any.h"
#include "Core.RTTI/Atom.h"
#include "Core.RTTI/AtomVisitor.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaTransaction.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/HashSet.h"
#include "Core/Container/Vector.h"
#include "Core/IO/BufferedStream.h"
#include "Core/IO/Format.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/IO/TextWriter.h"
#include "Core/Memory/MemoryStream.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FTextSerialize_ : public RTTI::IAtomVisitor {
public:
    FTextSerialize_(IBufferedStreamWriter* ostream, bool minify)
        : _oss(ostream)
        , _indent(minify ? FStringView() : MakeStringView("  "))
        , _newLine(minify ? ' ' : '\n')
        , _rootIndex(INDEX_NONE)
        , _outer(nullptr)
    {}

    void Serialize(const RTTI::FMetaTransaction& transaction) {
        Assert(nullptr == _outer);
        Assert(INDEX_NONE == _rootIndex);
        Assert(_anons.empty());

        RTTI::FLinearizedTransaction linearized;
        transaction.Linearize(&linearized);

        _outer = &transaction;
        _linearized = linearized.MakeConstView();

        forrange(i, 0, _linearized.size()) {
            RTTI::PMetaObject ref{ _linearized[_rootIndex = i].get() };
            Print_(ref);
        }

        _anons.clear();
        _rootIndex = INDEX_NONE;
        _outer = nullptr;
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
        _oss << inner.TypeInfos().Name() << ':';
        inner.Accept(this);
    }

    void Print_(const RTTI::PMetaObject& pobj) {
        if (not pobj) {
            _oss << "null";
            return;
        }

        RTTI::FMetaObject& obj = (*pobj);
        Assert(_linearized.end() != _linearized.FindIf([&obj](const RTTI::SMetaObject& o) {
            return (o.get() == &obj);
        }));

        // reference ?
        if (_linearized[_rootIndex].get() != &obj) {
            if (obj.RTTI_Outer() != _outer) {
                AssertRelease(obj.RTTI_IsExported()); // can't reference an object from another transaction if it's not exported
                Assert(obj.RTTI_Outer());
                Assert(not obj.RTTI_Name().empty());

                _oss << "$/" << obj.RTTI_Outer()->Name() << '/' << obj.RTTI_Name();
            }
            else if (obj.RTTI_IsExported()) {
                Assert(not obj.RTTI_Name().empty());

                _oss << "~/" << obj.RTTI_Name();
            }
            else {
                _oss << "~/" << MakeAnon_(pobj);
            }
            return;
        }

        // definition :
        if (obj.RTTI_Outer() != _outer) {
            AssertRelease(obj.RTTI_IsExported()); // can't reference an object from another transaction if it's not exported
            Assert(obj.RTTI_Outer());
            Assert(not obj.RTTI_Name().empty());

#if 0 // TODO : parser doesn't support this yet
            _oss << "import $/" << obj.RTTI_Outer()->Name() << '/' << obj.RTTI_Name() << '\n';
#endif
            return; // defined in another transaction
        }
        else {
            if (obj.RTTI_IsExported()) {
                Assert(not obj.RTTI_Name().empty());

                _oss << "export " << obj.RTTI_Name();
            }
            else {
                _oss << MakeAnon_(pobj);
            }
        }

        const RTTI::FMetaClass* metaClass = obj.RTTI_Class();
        Assert(metaClass);

        _oss << " is " << metaClass->Name() << " (";

        _indent.Inc();

        bool hasProperties = false;
        for (const RTTI::FMetaProperty* prop : metaClass->AllProperties()) {
            const RTTI::FAtom value = prop->Get(obj);
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

        _oss << ')' << _newLine;
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

    template <typename T>
    void Print_(const T& value) {
        _oss << value;
    }

    RTTI::FName MakeAnon_(const RTTI::SMetaObject& pobj) {
        Assert(pobj);

        RTTI::FName& name = _anons[pobj];
        if (name.empty()) {
            name = pobj->RTTI_Name();
            if (name.empty())
                name = RTTI::FName(StringFormat("anon_{0}_{1}", pobj->RTTI_Class()->Name(), _anons.size()));
        }

        Assert(not name.empty());
        return name;
    }

    FTextWriter _oss;
    Fmt::FIndent _indent;
    char _newLine;

    size_t _rootIndex;

    const RTTI::FMetaTransaction* _outer;
    TMemoryView<const RTTI::SMetaObject> _linearized;

    HASHMAP(Transient, RTTI::SMetaObject, RTTI::FName) _anons;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextSerializer::FTextSerializer(bool minify/* = true */)
    : _minify(minify)
{}
//----------------------------------------------------------------------------
FTextSerializer::~FTextSerializer() {}
//----------------------------------------------------------------------------
void FTextSerializer::DeserializeImpl(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName) {
    Assert(transaction);
    Assert(input);

    Parser::FParseList parseList;
    UsingBufferedStream(input, [&parseList, sourceName](IBufferedStreamReader* buffered) {
        Lexer::FLexer lexer(buffered, MakeCStringView(sourceName), true);
        parseList.Parse(&lexer);
    });

    Parser::FParseContext parseContext(Meta::ForceInit);

    while (Parser::PCParseExpression expr = FGrammarStartup::ParseExpression(parseList)) {
        if (expr) {
            const RTTI::FAtom atom = expr->Eval(&parseContext);
            const RTTI::PMetaObject* ppobj = atom.TypedConstDataIFP<RTTI::PMetaObject>();

            if (ppobj && *ppobj)
                transaction->RegisterObject(ppobj->get());
        }
    }
}
//----------------------------------------------------------------------------
void FTextSerializer::SerializeImpl(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);

    UsingBufferedStream(output, [this, transaction](IBufferedStreamWriter* buffered) {
        FTextSerialize_ visitor(buffered, _minify);
        visitor.Serialize(*transaction);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
