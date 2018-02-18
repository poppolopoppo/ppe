#include "stdafx.h"

#include "AtomVisitor.h"

#include "Any.h"
#include "Atom.h"
#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "NativeTypes.h"
#include "TypeTraits.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/Vector.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/StreamProvider.h"
#include "Core/IO/String.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/TextWriter.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarVector.h"

namespace Core {
namespace RTTI {
EXTERN_LOG_CATEGORY(CORE_RTTI_API, RTTI)
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
    static constexpr const char Nil[] = "nil";
    static constexpr const char Sep[] = ", ";
    static constexpr const char True[] = "true";
    static constexpr const char False[] = "false";
};
template <>
struct TPPFormat_<wchar_t> {
    using FIndent = Fmt::FWIndent;
    static constexpr const wchar_t Export[] = L"export";
    static constexpr const wchar_t Is[] = L"is";
    static constexpr const wchar_t Nil[] = L"nil";
    static constexpr const wchar_t Sep[] = L", ";
    static constexpr const wchar_t True[] = L"true";
    static constexpr const wchar_t False[] = L"false";
};
//----------------------------------------------------------------------------
template <typename _Char>
class TPrettyPrinter_ : public IAtomVisitor {
public:
    explicit TPrettyPrinter_(TBasicTextWriter<_Char>& oss)
        : _oss(oss)
    {}

    virtual bool Visit(const IPairTraits* pair, const FAtom& atom) override final {
        const FAtom first = pair->First(atom);
        const FAtom second = pair->Second(atom);

        _oss << Fmt::LParenthese;
        first.Accept(this);
        _oss << Fmt::Comma << Fmt::Space;
        second.Accept(this);
        _oss << Fmt::RParenthese;

        return true;
    }

    virtual bool Visit(const IListTraits* list, const FAtom& atom) override final {
        _oss << Fmt::LBracket;

        const bool empty = (list->Count(atom) == 0);

        if (not empty)
            _oss << Eol;

        _indent.Inc();

        list->ForEach(atom, [this](const FAtom& item) {
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

    virtual bool Visit(const IDicoTraits* dico, const FAtom& atom) override final {
        _oss << Fmt::LBrace;

        const bool empty = (dico->Count(atom) == 0);

        if (not empty)
            _oss << Eol;

        _indent.Inc();

        dico->ForEach(atom, [this](const FAtom& key, const FAtom& value) {
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

            const bool empty = (metaClass->NumProperties() == 0);

            if (not empty)
                _oss << Eol;

            _indent.Inc();

            for (const FMetaProperty* prop : metaClass->AllProperties()) {
                _oss << _indent << prop->Name() << Fmt::Space << Fmt::Assignment << Fmt::Space;
                prop->Get(obj).Accept(this);
                _oss << Eol;
            }

            _indent.Dec();

            if (not empty)
                _oss << _indent;

            _oss << Fmt::RBrace;
        }
        else {
            _oss << PP::Nil;
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

    template <typename T, size_t _Dim>
    void Print_(const TScalarVector<T, _Dim>& vec) {
        _oss << Fmt::LBracket;
        auto sep = Fmt::NotFirstTime(PP::Sep);
        for (T f : vec._data) {
            _oss << sep;
            Print_(f);
        }
        _oss << Fmt::RBracket;
    }

    template <typename T, size_t _W, size_t _H>
    void Print_(const TScalarMatrix<T, _W, _H>& mat) {
        _oss << Fmt::LBracket << Eol;
        _indent.Inc();
        forrange(i, 0, _W) {
            _oss << _indent;
            Print_(mat.Column(i));
            _oss << Fmt::Comma << Eol;
        }
        _indent.Dec();
        _oss << _indent << Fmt::RBracket;
    }

    template <typename T>
    void Print_(const T& value) {
        _oss << value;
    }

    typename PP::FIndent _indent;
    TBasicTextWriter<_Char>& _oss;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IPairTraits* pair, const FAtom& atom) {
    const FAtom first = pair->First(atom);
    const FAtom second = pair->Second(atom);
    return (first.Accept(visitor) && second.Accept(visitor));
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IListTraits* list, const FAtom& atom) {
    return list->ForEach(atom, [visitor](const FAtom& item) {
        return item.Accept(visitor);
    });
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IDicoTraits* dico, const FAtom& atom) {
    return dico->ForEach(atom, [visitor](const FAtom& key, const FAtom& value) {
        return (key.Accept(visitor) && value.Accept(visitor));
    });
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IScalarTraits* , FAny& any) {
    return (any ? any.InnerAtom().Accept(visitor) : true);
}
//----------------------------------------------------------------------------
bool IAtomVisitor::Accept(IAtomVisitor* visitor, const IScalarTraits* scalar, PMetaObject& pobj) {
    if (pobj) {
        FMetaObject& obj = (*pobj);
        const FMetaClass* metaClass = obj.RTTI_Class();
        Assert(metaClass);

        for (const FMetaProperty* prop : metaClass->AllProperties()) {
            if (not prop->Get(obj).Accept(visitor))
                return false;
        }
    }
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
PRAGMA_DISABLE_OPTIMIZATION
NO_INLINE FString PrettyString(const FAny& any) {
    return PrettyString(any.InnerAtom());
}
//----------------------------------------------------------------------------
NO_INLINE FString PrettyString(const FAtom& atom) {
    FStringBuilder oss;
    PrettyPrint(oss, atom);
    return oss.ToString();
}
//----------------------------------------------------------------------------
NO_INLINE FString PrettyString(const FMetaObject* object) {
    PMetaObject p(const_cast<FMetaObject*>(object));
    FString pp = PrettyString(MakeAtom(&p));
    RemoveRef_KeepAlive(p);
    return pp;
}
PRAGMA_ENABLE_OPTIMIZATION
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& PrettyPrint(FTextWriter& oss, const FAtom& atom) {
    TPrettyPrinter_<char> printer(oss);
    atom.Accept(&printer);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& PrettyPrint(FWTextWriter& oss, const FAtom& atom) {
    TPrettyPrinter_<wchar_t> printer(oss);
    atom.Accept(&printer);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
