// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RTTI/AtomVisitor.h"

#include "RTTI/Any.h"
#include "RTTI/Atom.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/TypeTraits.h"
#include "RTTI/Typedefs.h"

#include "MetaClass.h"
#include "MetaEnum.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "Diagnostic/DebugFunction.h"
#include "Diagnostic/Logger.h"
#include "IO/FileSystem.h"
#include "IO/FormatHelpers.h"
#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
struct TPPFormat_;
template <>
struct TPPFormat_<char> {
    using FIndent = Fmt::FIndent;
    static constexpr const char Export[] = "export";
    static constexpr const char Is[] = "is";
    static constexpr const char Null[] = "null";
    static constexpr const char Sep[] = ", ";
    static constexpr const char True[] = "true";
    static constexpr const char False[] = "false";
};
template <>
struct TPPFormat_<wchar_t> {
    using FIndent = Fmt::FWIndent;
    static constexpr const wchar_t Export[] = L"export";
    static constexpr const wchar_t Is[] = L"is";
    static constexpr const wchar_t Null[] = L"null";
    static constexpr const wchar_t Sep[] = L", ";
    static constexpr const wchar_t True[] = L"true";
    static constexpr const wchar_t False[] = L"false";
};
//----------------------------------------------------------------------------
template <typename _Char>
class TPrettyPrinter_ : public IAtomVisitor {
public:
    TPrettyPrinter_(TBasicTextWriter<_Char>& oss, EPrettyPrintFlags flags)
        : IAtomVisitor(flags ^ EPrettyPrintFlags::NoRecursion
            ? EVisitorFlags::NoRecursion|Default
            : Default )
        , _oss(oss)
        , _prettyPrintFlags(flags)
    {}

    virtual bool Visit(const ITupleTraits* tuple, void* data) override final {
        _oss << tuple->TypeName() << Fmt::Colon << Fmt::LParenthese;

        auto sep = Fmt::NotFirstTime(MakeStringView(TPPFormat_<_Char>::Sep));
        tuple->ForEach(data, [this, &sep](const FAtom& elt) {
            _oss << sep;
            elt.Accept(this);

            return true;
        });

        _oss << Fmt::RParenthese;

        return true;
    }

    virtual bool Visit(const IListTraits* list, void* data) override final {
        _oss << list->TypeName() << Fmt::Colon << Fmt::LBracket;

        const bool empty = (list->Count(data) == 0);

        if (not empty)
            _oss << Eol;

        _indent.Inc();

        list->ForEach(data, [this](const FAtom& item) {
            _oss << _indent;
            item.Accept(this);
            _oss << Fmt::Comma << Eol;

            return true;
        });

        _indent.Dec();

        if (not empty)
            _oss << _indent;

        _oss << Fmt::RBracket;

        return true;
    }

    virtual bool Visit(const IDicoTraits* dico, void* data) override final {
        _oss << dico->TypeName() << Fmt::Colon << Fmt::LBrace;

        const bool empty = (dico->Count(data) == 0);

        if (not empty)
            _oss << Eol;

        _indent.Inc();

        dico->ForEach(data, [this](const FAtom& key, const FAtom& value) {
            _oss << _indent << Fmt::LParenthese;
            key.Accept(this);
            _oss << Fmt::Comma << Fmt::Space;
            value.Accept(this);
            _oss << Fmt::RParenthese << Fmt::Comma << Eol;

            return true;
        });

        _indent.Dec();

        if (not empty)
            _oss << _indent;

        _oss << Fmt::RBrace;

        return true;
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* scalar, T& value) override final { \
        PrintDispatch_(scalar, value, typename std::is_integral<T>::type{}); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    using PP = TPPFormat_<_Char>;

    template <typename T>
    void PrintDispatch_(const IScalarTraits* scalar, const T& value, std::false_type) {
        if (_prettyPrintFlags & EPrettyPrintFlags::ShowTypeNames)
            _oss << scalar->TypeName() << Fmt::Colon;

        Print_(value);
    }

    template <typename T>
    void PrintDispatch_(const IScalarTraits* scalar, T integral, std::true_type) {
        if (_prettyPrintFlags & EPrettyPrintFlags::ShowEnumNames) {
            if (const FMetaEnum* const e = scalar->EnumClass()) {
                if (e->IsFlags()) {
                    FMetaEnum::FExpansion values;
                    if (e->ExpandValues(integral, &values)) {
                        Assert(not values.empty());

                        _oss << Fmt::LParenthese << scalar->TypeName() << Fmt::Colon << values[0].Name;

                        forrange(i, 1, values.size())
                            _oss << Fmt::Or << scalar->TypeName() << Fmt::Colon << values[i].Name;

                        _oss << Fmt::RParenthese;

                        return;
                    }
                }
                else {
                    const FMetaEnumValue* const v = e->ValueToNameIFP(integral);
                    if (v) {
                        _oss << scalar->TypeName() << Fmt::Colon << v->Name;
                        return;
                    }
                }
            }
        }

        PrintDispatch_(scalar, integral, std::false_type{});
    }

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
                if (not (_prettyPrintFlags & EPrettyPrintFlags::ShowDefaults) && val.IsDefaultValue())
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
    const EPrettyPrintFlags _prettyPrintFlags;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IAtomVisitor::ShouldSkipTraits(IAtomVisitor* visitor, const ITypeTraits& traits) NOEXCEPT {
    return ((!!visitor->OnlyObjects()) && (not is_object_v(traits.TypeFlags())));
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const ITupleTraits* tuple, void* data) {
    if (ShouldSkipTraits(visitor, *tuple))
        return true;

    return tuple->ForEach(data, [visitor](const FAtom& elt) {
        return elt.Accept(visitor);
    });
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IListTraits* list, void* data) {
    if (ShouldSkipTraits(visitor, *list))
        return true;

    return list->ForEach(data, [visitor](const FAtom& item) {
        return item.Accept(visitor);
    });
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IDicoTraits* dico, void* data) {
    if (ShouldSkipTraits(visitor, *dico))
        return true;

    return dico->ForEach(data, [visitor](const FAtom& key, const FAtom& value) {
        return (key.Accept(visitor) && value.Accept(visitor));
    });
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IScalarTraits* , FAny& any) {
    if (any) {
        const FAtom inner{ any.InnerAtom() };
        if (not ShouldSkipTraits(visitor, *inner.Traits()))
            return inner.Accept(visitor);
    }

    return true;
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IScalarTraits* , PMetaObject& pobj) {
    if (pobj) {
        FMetaObject& obj = (*pobj);

        if (visitor->NoRecursion() || ((not visitor->KeepTransient()) && obj.RTTI_IsTransient()) )
            return true;

        const FMetaClass* const metaClass = obj.RTTI_Class();
        Assert(metaClass);

        for (const FMetaProperty* prop : metaClass->AllProperties()) {
            if ((visitor->KeepDeprecated() || (not prop->IsDeprecated())) &&
                (visitor->KeepTransient() || (not prop->IsTransient())) &&
                (not ShouldSkipTraits(visitor, *prop->Traits())) ) {
                if (not prop->Get(obj).Accept(visitor))
                    return false;
            }
        }
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DEBUG_FUNCTION(PPE_RTTI_API, DebugPrintAtom, FString, (const FAtom& atom), {
    FStringBuilder oss;
    PrettyPrint(oss, atom, EPrettyPrintFlags::Minimal | EPrettyPrintFlags::ShowEnumNames);
    return oss.ToString();
})
//----------------------------------------------------------------------------
DEBUG_FUNCTION(PPE_RTTI_API, DebugPrintObject, FString, (const FMetaObject* object), {
    PMetaObject p(const_cast<FMetaObject*>(object));
    FString pp = DebugPrintAtom(FAtom::FromObj(p));
    RemoveRef_KeepAlive(p);
    return pp;
})
//----------------------------------------------------------------------------
DEBUG_FUNCTION(PPE_RTTI_API, DebugPrintAny, FString, (const FAny& any), {
    return DebugPrintAtom(any.InnerAtom());
})
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& PrettyPrint(FTextWriter& oss, const FAtom& atom, EPrettyPrintFlags flags/* = EPrettyPrintFlags::Default */) {
    TPrettyPrinter_<char> printer(oss, flags);
    atom.Accept(&printer);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& PrettyPrint(FWTextWriter& oss, const FAtom& atom, EPrettyPrintFlags flags/* = EPrettyPrintFlags::Default */) {
    TPrettyPrinter_<wchar_t> printer(oss, flags);
    atom.Accept(&printer);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
