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
#include "Core/IO/Stream.h"
#include "Core/IO/String.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarVector.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
class TPrettyPrinter_ : public IAtomVisitor {
public:
    explicit TPrettyPrinter_(std::basic_ostream<_Char>& oss)
        : _oss(oss)
    {}

    virtual bool Visit(const IPairTraits* pair, const FAtom& atom) override final {
        const FAtom first = pair->First(atom);
        const FAtom second = pair->Second(atom);

        _oss << '(';
        first.Accept(this);
        _oss << ',' << ' ';
        second.Accept(this);
        _oss << ')';

        return true;
    }

    virtual bool Visit(const IListTraits* list, const FAtom& atom) override final {
        _oss << '[';

        const bool empty = (list->Count(atom) == 0);

        if (not empty)
            _oss << eol;

        _indent.Inc();

        list->ForEach(atom, [this](const FAtom& item) {
            _oss << _indent;
            item.Accept(this);
            _oss << ',' << eol;

            return true;
        });

        _indent.Dec();

        if (not empty)
            _oss << _indent;

        _oss << ']';

        return true;
    }

    virtual bool Visit(const IDicoTraits* dico, const FAtom& atom) override final {
        _oss << '{';

        const bool empty = (dico->Count(atom) == 0);

        if (not empty)
            _oss << eol;

        _indent.Inc();

        dico->ForEach(atom, [this](const FAtom& key, const FAtom& value) {
            _oss << _indent << '(';
            key.Accept(this);
            _oss << ',' << ' ';
            value.Accept(this);
            _oss << ')' << ',' << eol;

            return true;
        });

        _indent.Dec();

        if (not empty)
            _oss << _indent;

        _oss << '}';

        return true;
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* scalar, T& value) override final { \
        _oss << scalar->TypeInfos().Name() << ':'; \
        Print_(value); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
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
                _oss << "export " << obj.RTTI_Name() << " is ";

            _oss << metaClass->Name() << ' ' << '{';

            const bool empty = (metaClass->NumProperties() == 0);

            if (not empty)
                _oss << eol;

            _indent.Inc();

            for (const FMetaProperty* prop : metaClass->AllProperties()) {
                _oss << _indent << prop->Name() << ' ' << '=' << ' ';
                prop->Get(obj).Accept(this);
                _oss << eol;
            }

            _indent.Dec();

            if (not empty)
                _oss << _indent;

            _oss << '}';
        }
        else {
            _oss << "nil";
        }
    }

    void Print_(bool b) { _oss << (b ? "true" : "false"); }

    void Print_(i8 ch) { _oss << int(ch); }
    void Print_(u8 uch) { _oss << unsigned(uch); }

    void Print_(const FName& name) { _oss << name; }
    void Print_(const FString& str) { _oss << '"' << str << '"'; }
    void Print_(const FWString& wstr) { _oss << '"' << wstr << '"'; }

    void Print_(const FDirpath& dirpath) { Print_(dirpath.ToString()); }
    void Print_(const FFilename& filename) { Print_(filename.ToString()); }

    template <typename T, size_t _Dim>
    void Print_(const TScalarVector<T, _Dim>& vec) {
        _oss << '[';
        auto sep = Fmt::NotFirstTime(", ");
        for (T f : vec._data) {
            _oss << sep;
            Print_(f);
        }
        _oss << ']';
    }

    template <typename T, size_t _W, size_t _H>
    void Print_(const TScalarMatrix<T, _W, _H>& mat) {
        _oss << '[' << eol;
        _indent.Inc();
        forrange(i, 0, _W) {
            _oss << _indent;
            Print_(mat.Column(i));
            _oss << ',' << eol;
        }
        _indent.Dec();
        _oss << _indent << ']';
    }

    template <typename T>
    void Print_(const T& value) {
        _oss << value;
    }

    Fmt::FIndent _indent;
    std::basic_ostream<_Char>& _oss;
};
//----------------------------------------------------------------------------
class FCircularReferenceVisitor_ : FBaseAtomVisitor {
public:
    FCircularReferenceVisitor_() : _succeed(true) {}

    bool CheckReferences(const FMetaObject& root) {
        _visiteds.clear();
        _chainRef.clear();
        _succeed = true;

        _visiteds.emplace(&root, _chainRef.size());
        _chainRef.emplace_back(&root, nullptr);

        PMetaObject src(const_cast<FMetaObject*>(&root));

        if (not InplaceAtom(src).Accept(static_cast<FBaseAtomVisitor*>(this)))
            AssertNotReached();

        Assert(_visiteds.size() == 1);
        Assert(_chainRef.size() == 1);

        RemoveRef_KeepAlive(src);

        return _succeed;
    }

private:
    struct FPropertyRef_ {
        SCMetaObject Object;
        const FMetaProperty* Property;
    };

    virtual bool Visit(const IScalarTraits* scalar, PMetaObject& pobj) override {
        if (pobj) {
            const auto r = _visiteds.insert(MakePair(pobj.get(), _chainRef.size()));
            if (not r.second) {
                Assert(pobj == r.first->first);
                Assert(r.first->second < _chainRef.size());

                // Circular dependency found !
                // Printing chain reference :
                DumpChainReference_(pobj, r.first->second);
            }

            FMetaObject& obj = (*pobj);
            const FMetaClass* metaClass = obj.RTTI_Class();
            Assert(metaClass);

            for (const FMetaProperty* prop : metaClass->AllProperties()) {
                _chainRef.emplace_back(&obj, prop);

                if (not prop->Get(obj).Accept(static_cast<FBaseAtomVisitor*>(this)))
                    return false;

                Assert(r.first->second + 1 == _chainRef.size());
                _chainRef.pop_back();
            }
        }
        return true;
    }

    void DumpChainReference_(const PMetaObject& pobj, size_t chainIndex) const {
#ifdef USE_DEBUG_LOGGER
        FLoggerStream oss(ELogCategory::Error);
        oss << L"[RTTI] Found circular reference !" << eol;

        Fmt::FIndent indent = Fmt::FIndent::TwoSpaces();
        forrange(i, chainIndex, _chainRef.size()) {
            const FPropertyRef_& ref = _chainRef[i];
            const FMetaClass* metaClass = ref.Object->RTTI_Class();

            oss << indent
                << L" -> " << (void*)ref.Object.get()
                << L" : " << metaClass->Name()
                << " (" << metaClass->Flags()
                << L") = " << ref.Property->Name()
                << eol;

            indent.Inc();
        }

        oss << indent << L" => ";
        PrettyPrint(oss, InplaceAtom(pobj));
#else
        UNUSED(obj);
        UNUSED(chainIndex);
#endif
    }

    bool _succeed;
    HASHMAP_THREAD_LOCAL(RTTI, const FMetaObject*, size_t) _visiteds;
    VECTORINSITU_THREAD_LOCAL(RTTI, FPropertyRef_, 32) _chainRef;
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
bool HasCircularDependencies(const FMetaObject& object) {
    FCircularReferenceVisitor_ circularRef;
    return circularRef.CheckReferences(object);
}
//----------------------------------------------------------------------------
bool HasCircularDependencies(const TMemoryView<const PMetaObject>& objects) {
    bool succeed = true;
    FCircularReferenceVisitor_ circularRef;
    for (const PMetaObject& object : objects)
        succeed &= circularRef.CheckReferences(*object);
    return succeed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString PrettyString(const FAny& any) {
    return PrettyString(any.InnerAtom());
}
//----------------------------------------------------------------------------
FString PrettyString(const FAtom& atom) {
    FOStringStream oss;
    PrettyPrint(oss, atom);
    return oss.str();
}
//----------------------------------------------------------------------------
FString PrettyString(const FMetaObject* object) {
    PMetaObject p(const_cast<FMetaObject*>(object));
    FString pp = PrettyString(MakeAtom(&p));
    RemoveRef_KeepAlive(p);
    return pp;
}
//----------------------------------------------------------------------------
std::basic_ostream<char>& PrettyPrint(std::basic_ostream<char>& oss, const FAtom& atom) {
    TPrettyPrinter_<char> printer(oss);
    atom.Accept(&printer);
    return oss;
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& PrettyPrint(std::basic_ostream<wchar_t>& oss, const FAtom& atom) {
    TPrettyPrinter_<wchar_t> printer(oss);
    atom.Accept(&printer);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
