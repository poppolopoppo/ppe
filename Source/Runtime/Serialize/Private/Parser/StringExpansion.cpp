#include "stdafx.h"

#include "Parser/StringExpansion.h"

#include "MetaEnum.h"
#include "MetaObject.h"

#include "RTTI/Any.h"
#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/TypeTraits.h"

#include "Allocator/Alloca.h"
#include "Container/Stack.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"

#include "IO/FileSystem.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"

// We expand string using PPE::Format(), which gives many options for formating inputs

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FAtomFormatArgVisitor_ : public RTTI::IAtomVisitor {
public:
    FAtomFormatArgVisitor_()
    :   _null("null")
    {}

    FFormatArgList MakeView() const { return _args.MakeView(); }

    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        PrettyPrint_(tuple, data);
        return true;
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        PrettyPrint_(list, data);
        return true;
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override final {
        PrettyPrint_(dico, data);
        return true;
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const RTTI::IScalarTraits* scalar, T& value) override final { \
        PrintValue_(scalar, value, std::bool_constant< RTTI::is_integral(RTTI::FTypeId(_TypeId)) >{}); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    template <typename T>
    void PrintValue_(const RTTI::IScalarTraits* scalar, const T& integral, std::true_type) {
        if (scalar->TypeFlags() & RTTI::ETypeFlags::Enum)
            PrettyPrint_(scalar, &integral);
        else
            _args.Push(MakeFormatArg<char>(integral)); // *BEWARE* : keep const T& since we're passing a ref here !
    }

    void PrintValue_(const RTTI::IScalarTraits*, const RTTI::PMetaObject& pobj, std::false_type) {
        if (pobj) {
            _args.Push(MakeFormatLambda<char>(
                [](FTextWriter& oss, const void* data) {
                    const auto& o = *reinterpret_cast<const RTTI::FMetaObject*>(data);
                    if (o.RTTI_IsExported() && o.RTTI_Outer())
                        oss << RTTI::FPathName::FromObject(o);
                    else
                        oss << o.RTTI_Class()->Name() << '/' << (void*)&o;

                },  pobj.get() ));
        }
        else {
            _args.Push(MakeFormatArg<char>(_null));
        }
    }

    void PrintValue_(const RTTI::IScalarTraits*, const RTTI::FAny& any, std::false_type) {
        if (any)
            any.InnerAtom().Accept(this);
    }

    template <typename T>
    void PrintValue_(const RTTI::IScalarTraits*, const T& value, std::false_type) {
        _args.Push(MakeFormatArg<char>(value));
    }

    void PrettyPrint_(const RTTI::ITypeTraits* traits, const void* data) {
        Assert(traits);
        UNUSED(data);

        FStringBuilder sb;
        PrettyPrint(sb, RTTI::FAtom{ data, RTTI::PTypeTraits(traits) },
            RTTI::EPrettyPrintFlags::Minimal|
            RTTI::EPrettyPrintFlags::ShowEnumNames );

        _strs.Push(sb.ToString());
        _args.Push(MakeFormatArg<char>(*_strs.Peek()));
    }

    const FStringView _null;

    STATIC_CONST_INTEGRAL(size_t, MaxArgs, 9);

    TFixedSizeStack<FFormatArg, MaxArgs> _args;
    TFixedSizeStack<FString, MaxArgs> _strs;
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

    FAtomFormatArgVisitor_ args;
    scalar.Accept(&args);

    FStringBuilder sb;
    FormatArgs(sb, fmt.MakeView(), args.MakeView());
    return sb.ToString();
}
//----------------------------------------------------------------------------
FString PerformStringExpansion(const FString& fmt, const RTTI::FAtom& tuple, const RTTI::ITupleTraits& traits, const Lexer::FSpan& site) {
    Assert(tuple);
    UNUSED(traits);
    UNUSED(site);

    FAtomFormatArgVisitor_ args;
    traits.ForEach(tuple.Data(), [&args](const RTTI::FAtom& it) {
        return it.Accept(&args);
    });

    FStringBuilder sb;
    FormatArgs(sb, fmt.MakeView(), args.MakeView());
    return sb.ToString();
}
//----------------------------------------------------------------------------
FString PerformStringExpansion(const FString& fmt, const RTTI::FAtom& list, const RTTI::IListTraits& traits, const Lexer::FSpan& site) {
    Assert(list);
    UNUSED(traits);
    UNUSED(site);

    FAtomFormatArgVisitor_ args;
    traits.ForEach(list.Data(), [&args](const RTTI::FAtom& it) {
        return it.Accept(&args);
    });

    FStringBuilder sb;
    FormatArgs(sb, fmt.MakeView(), args.MakeView());
    return sb.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE