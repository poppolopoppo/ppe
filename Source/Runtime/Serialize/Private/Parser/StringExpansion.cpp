#include "stdafx.h"

#include "Parser/StringExpansion.h"

#include "RTTI/Atom.h"
#include "RTTI/AtomHelpers.h"
#include "RTTI/TypeTraits.h"

#include "Allocator/Alloca.h"
#include "Container/Vector.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"

// We expand string using PPE::Format(), which gives many options for formating inputs

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FString ExpandString_(const FString& fmt, const TMemoryView<const FString>& strs) {
    STACKLOCAL_ASSUMEPOD_ARRAY(FFormatArg, args, strs.size());
    forrange(i, 0, strs.size())
        args[i] = MakeFormatArg<char>(strs[i]);

    FStringBuilder sb;
    FormatArgs(sb, fmt.MakeView(), args.AddConst());

    return sb.ToString();
}
//----------------------------------------------------------------------------
class FAtomFormatArgVisitor_ : public IAtomVisitor {
public:
    FAtomFormatArgVisitor_() {}

    FFormatArgList FormatArgs() const { return _formatArgs.MakeView(); }

    virtual bool Visit(const ITupleTraits* tuple, void* data) override final {
        _formatArgs.emplace_back(MakeFormatArg(tuple->TypeName()));
        return true;
    }

    virtual bool Visit(const IListTraits* list, void* data) override final {
        _formatArgs.emplace_back(MakeFormatArg(tuple->TypeName()));
        return true;
    }

    virtual bool Visit(const IDicoTraits* dico, void* data) override final {
        _formatArgs.emplace_back(MakeFormatArg(tuple->TypeName()));
        return true;
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* scalar, T& value) override final { \
        _oss << scalar->TypeInfos().Name() << Fmt::Colon; \
        Print_(value); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    using PP = TPPFormat_<_Char>;

    void Print_(const FAny& any) {
        if (any)
            any.InnerAtom().Accept(this);
    }

    void Print_(const PMetaObject& pobj) {
        if (pobj) {
            FMetaObject& obj = (*pobj);
            const FMetaClass* metaClass = obj.RTTI_Class();
            Assert(metaClass);

            if (obj.RTTI_IsExported())
                _oss << PP::Export << Fmt::Space
                     << obj.RTTI_Name() << Fmt::Space << PP::Is << Fmt::Space;

            _oss << metaClass->Name() << Fmt::Space << Fmt::LBrace;

            bool empty = true;
            for (const FMetaProperty* prop : metaClass->AllProperties()) {
                const RTTI::FAtom val = prop->Get(obj);
                if (not _showDefaults & val.IsDefaultValue())
                    continue;

                if (empty) {
                    empty = false;
                    _oss << Eol;
                    _indent.Inc();
                }

                _oss << _indent << prop->Name() << Fmt::Space << Fmt::Assignment << Fmt::Space;

                val.Accept(this);
                _oss << Eol;
            }

            if (not empty) {
                _indent.Dec();
                _oss << _indent;
            }

            _oss << Fmt::RBrace;
        }
        else {
            _oss << PP::Null;
        }
    }

    void Print_(bool b) { _oss << (b ? PP::True : PP::False); }

    void Print_(i8 ch) { _oss << int(ch); }
    void Print_(u8 uch) { _oss << unsigned(uch); }

    void Print_(const FName& name) { _oss << name; }
    void Print_(const FString& str) { _oss << Fmt::DoubleQuote << str << Fmt::DoubleQuote; }
    void Print_(const FWString& wstr) { _oss << Fmt::DoubleQuote << wstr << Fmt::DoubleQuote; }

    void Print_(const FDirpath& dirpath) { Print_(dirpath.ToString()); }
    void Print_(const FFilename& filename) { Print_(filename.ToString()); }

    template <typename T>
    void Print_(const T& value) {
        _oss << value;
    }

    typename PP::FIndent _indent;
    TBasicTextWriter<_Char>& _oss;
    const bool _showDefaults;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString PerformStringExpansion(const FString& fmt, const RTTI::FAtom& scalar, const RTTI::IScalarTraits& traits, const Lexer::FSpan& site) {
    Assert(scalar);

    UNUSED(traits);
    UNUSED(site);

    VECTOR(Parser, FString) args;
    args.emplace_back(scalar.ToString());

    return ExpandString_(fmt, args);
}
//----------------------------------------------------------------------------
FString PerformStringExpansion(const FString& fmt, const RTTI::FAtom& tuple, const RTTI::ITupleTraits& traits, const Lexer::FSpan& site) {
    Assert(tuple);

    UNUSED(traits);
    UNUSED(site);

    VECTOR(Parser, FString) args;
    args.reserve(traits.Arity());

    traits.ForEach(tuple.Data(), [&args](const RTTI::FAtom& it) {
        args.emplace_back(it.ToString());
        return true;
    });

    return ExpandString_(fmt, args);
}
//----------------------------------------------------------------------------
FString PerformStringExpansion(const FString& fmt, const RTTI::FAtom& list, const RTTI::IListTraits& traits, const Lexer::FSpan& site) {
    Assert(list);

    UNUSED(traits);
    UNUSED(site);

    VECTOR(Parser, FString) args;
    args.reserve(traits.Count(list));

    traits.ForEach(list.Data(), [&args](const RTTI::FAtom& it) {
        args.emplace_back(it.ToString());
        return true;
    });

    return ExpandString_(fmt, args);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE