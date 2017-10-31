#include "stdafx.h"

#include "AtomVisitor.h"

#include "Atom.h"
#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "NativeTypes.h"
#include "TypeTraits.h"

#include "Core/IO/FileSystem.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/Stream.h"
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

    virtual bool Visit(const IPairTraits* traits, const FAtom& atom) override final {
        const FAtom first = traits->First(atom);
        const FAtom second = traits->Second(atom);

        _oss << '(';
        first.Accept(this);
        _oss << ',' << ' ';
        second.Accept(this);
        _oss << ')';

        return true;
    }

    virtual bool Visit(const IListTraits* traits, const FAtom& atom) override final {
        _oss << '[';

        const bool empty = (traits->Count(atom) == 0);

        if (not empty)
            _oss << eol;

        _indent.Inc();

        traits->ForEach(atom, [this](const FAtom& item) {
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

    virtual bool Visit(const IDicoTraits* traits, const FAtom& atom) override final {
        _oss << '{';

        const bool empty = (traits->Count(atom) == 0);

        if (not empty)
            _oss << eol;

        _indent.Inc();

        traits->ForEach(atom, [this](const FAtom& key, const FAtom& value) {
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
    virtual bool Visit(const IScalarTraits* , T& value) override final { \
        Visit_(value); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    void Visit_(const PMetaObject& pobj) {
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

    template <typename T>
    void Visit_(const T& value) {
        _oss << value;
    }

    Fmt::FIndent _indent;
    std::basic_ostream<_Char>& _oss;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FBaseAtomVisitor::Visit(const IPairTraits* traits, const FAtom& atom) {
    const FAtom first = traits->First(atom);
    const FAtom second = traits->Second(atom);
    return (first.Accept(this) && second.Accept(this));
}
//----------------------------------------------------------------------------
bool FBaseAtomVisitor::Visit(const IListTraits* traits, const FAtom& atom) {
    return traits->ForEach(atom, [this](const FAtom& item) {
        return item.Accept(this);
    });
}
//----------------------------------------------------------------------------
bool FBaseAtomVisitor::Visit(const IDicoTraits* traits, const FAtom& atom) {
    return traits->ForEach(atom, [this](const FAtom& key, const FAtom& value) {
        return (key.Accept(this) && value.Accept(this));
    });
}
//----------------------------------------------------------------------------
bool FBaseAtomVisitor::Visit(const IScalarTraits* traits, PMetaObject& pobj) {
    if (pobj) {
        FMetaObject& obj = (*pobj);
        const FMetaClass* metaClass = obj.RTTI_Class();
        Assert(metaClass);

        for (const FMetaProperty* prop : metaClass->AllProperties()) {
            if (not prop->Get(obj).Accept(this))
                return false;
        }
    }
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
