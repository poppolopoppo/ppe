#include "stdafx.h"

#include "MetaFunction.h"

#include <numeric>

#include "Container/Stack.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
#   include "MetaObject.h"
#   include "Diagnostic/Logger.h"
#   define CheckFunctionCallIFN(obj, result, args) CheckFunctionCall_(obj, result, args)
namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
} //!namespace RTTI
} //!namespace PPE
#else
#   define CheckFunctionCallIFN(obj, result, args) NOOP()
#endif

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if defined(WITH_PPE_RTTI_FUNCTION_CHECKS) && USE_PPE_LOGGER
struct FMetaFunctionCallFormattor_ {
    const FMetaObject* pObj;
    const FMetaFunction* pFunc;
    FMetaFunctionCallFormattor_(const FMetaObject& obj, const FMetaFunction& func) NOEXCEPT
    :    pObj(&obj)
    ,    pFunc(&func)
    {}
    template <typename _Char>
    friend TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const FMetaFunctionCallFormattor_& call) {
        oss << L"function \"";

        if (call.pFunc->Result())
            oss << call.pFunc->Result()->NamedTypeInfos();
        else
            oss << L"void";

        oss << L' ' << call.pObj->RTTI_Class()->Name() << L"::" << call.pFunc->Name() << L'(';

        forrange(i, 0, call.pFunc->Parameters().size()) {
            if (i > 0) oss << L", ";
            const FMetaParameter& prm = call.pFunc->Parameters()[i];
            oss << prm.Name() << L" : " << prm.Traits()->TypeName() << L" <" << prm.Flags() << L'>';
        }

        oss << L") on object \""
            << call.pObj->RTTI_Name()
            << L"\" ("
            << call.pObj->RTTI_Flags()
            << L')';

        return oss;
    }
};
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_ASSERT_TYPE_IS_POD(FMetaParameter);
//----------------------------------------------------------------------------
FMetaParameter::FMetaParameter() NOEXCEPT {
    _traitsAndFlags.Reset(nullptr, false, false);
}
//----------------------------------------------------------------------------
FMetaParameter::FMetaParameter(const FName& name, const PTypeTraits& traits, EParameterFlags flags) NOEXCEPT
    : _name(name) {
    Assert(not _name.empty());
    Assert(traits.Valid());
    STATIC_ASSERT(sizeof(PTypeTraits) == sizeof(void*));
    _traitsAndFlags.Reset(union_cast_t{ traits }.Raw, uintptr_t(flags));
    Assert(traits == Traits());
    Assert(flags == Flags());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaFunction::FMetaFunction() NOEXCEPT
    : _invoke(nullptr)
    , _flags(EFunctionFlags(0))
{}
//----------------------------------------------------------------------------
FMetaFunction::FMetaFunction(
    const FName& name,
    EFunctionFlags flags,
    const PTypeTraits& result,
    std::initializer_list<FMetaParameter> parameters,
    invoke_func invoke ) NOEXCEPT
    : _name(name)
    , _invoke(invoke)
    , _flags(flags)
    , _result(result)
    , _parameters(parameters) {
    Assert(not _name.empty());
    Assert(_invoke);

#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
    // check that optional parameters are all packed ate
    bool optional = false;
    for (const FMetaParameter& prm : _parameters) {
        if (prm.IsOptional())
            optional = true;
        else
            Assert(not optional);
    }
#endif
}
//----------------------------------------------------------------------------
FMetaFunction::~FMetaFunction() = default;
//----------------------------------------------------------------------------
void FMetaFunction::Invoke(
    const FMetaObject& obj,
    const FAtom& result,
    const TMemoryView<const FAtom>& arguments ) const {
    Assert(_invoke);
    Assert(result || not HasReturnValue());
    Assert(arguments.size() <= _parameters.size()); // can have optional parameters

    CheckFunctionCallIFN(obj, result, arguments);

    _invoke(obj, result, arguments);
}
//----------------------------------------------------------------------------
bool FMetaFunction::InvokeIFP(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) const {
    Assert(_invoke);

    if (arguments.size() != _parameters.size()) {
        LOG(RTTI, Warning, L"given {0} arguments instead of {1} when calling {2}",
            arguments.size(), _parameters.size(),
            FMetaFunctionCallFormattor_(obj COMMA *this) );
        return false;
    }

    if (!!_result != !!result) {
        LOG(RTTI, Warning, L"no result is returned by {0}",
            FMetaFunctionCallFormattor_(obj COMMA *this) );
        return false;
    }

    size_t strideInBytes = ((_result && result.Traits() != _result) ? _result->SizeInBytes() : 0);
    forrange(i, 0, _parameters.size()) {
        Assert(arguments[i]);

        if (_parameters[i].Traits() != arguments[i].Traits())
            strideInBytes += _parameters[i].Traits()->SizeInBytes();
    }

    if (Likely(0 == strideInBytes)) {
        CheckFunctionCallIFN(obj, result, arguments);
        _invoke(obj, result, arguments);
        return true;
    }
    else {
        return PromoteInvoke_(obj, result, arguments, strideInBytes);
    }
}
//----------------------------------------------------------------------------
bool FMetaFunction::PromoteInvoke_(
    const FMetaObject& obj,
    const FAtom& result,
    const TMemoryView<const FAtom>& arguments,
    size_t strideInBytes ) const {
    const bool resultPromotion = (_result && result.Traits() != _result);

    strideInBytes += _parameters.size() * sizeof(FAtom);
    STACKLOCAL_POD_ARRAY(u8, tmp, strideInBytes);
    FRawMemory rawData = tmp;

    const TMemoryView<FAtom> nativeArgs = rawData.CutStartingAt(_parameters.size() * sizeof(FAtom)).Cast<FAtom>();

    FAtom nativeResult = result;
    if (resultPromotion) {
        nativeResult = FAtom{ rawData.data(), _result };
        _result->Construct(nativeResult.Data());
        rawData = rawData.CutStartingAt(_result->SizeInBytes());
    }

    forrange(i, 0, _parameters.size()) {
        FAtom& nativeArg = nativeArgs[i];
        if (_parameters[i].Traits() != arguments[i].Traits()) {
            nativeArg = FAtom{ rawData.data(), _parameters[i].Traits() };
            nativeArg.Traits()->Construct(nativeArg.Data());
            rawData = rawData.CutStartingAt(nativeArg.Traits()->SizeInBytes());
        }
        else {
            nativeArg = arguments[i];
        }
    }

    Assert_NoAssume(rawData.empty());
    CheckFunctionCallIFN(obj, nativeResult, nativeArgs);

    _invoke(obj, nativeResult, nativeArgs);

    bool succeed = true;

    if (resultPromotion) {
        Assert(nativeResult != result);

        if (not nativeResult.PromoteMove(result)) {
            LOG(RTTI, Warning, L"wrong result type <{1}> when calling {0}",
                FMetaFunctionCallFormattor_(obj COMMA *this),
                result.Traits()->NamedTypeInfos() );
            succeed = false;
        }

        nativeResult.Traits()->Destroy(nativeResult.Data());
    }

    forrange(i, 0, _parameters.size()) {
        FAtom& arg = nativeArgs[i];
        if (arg != arguments[i]) {
            if (not arg.PromoteMove(arguments[i])) {
                LOG(RTTI, Warning, L"wrong type for argument #{1} {2} instead of {3} when calling {0}",
                    FMetaFunctionCallFormattor_(obj COMMA *this),
                    i, arguments[i].Traits()->NamedTypeInfos(), arg.Traits()->NamedTypeInfos() );
                succeed = false;
            }

            arg.Traits()->Destroy(arg.Data());
        }
    }

    return succeed;
}
//----------------------------------------------------------------------------
#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
void FMetaFunction::CheckFunctionCall_(
    const FMetaObject& obj,
    const FAtom& result,
    const TMemoryView<const FAtom>& arguments ) const {

    if (not IsConst() && obj.RTTI_IsFrozen())
        LOG(RTTI, Fatal, L"can't use frozen object with non-const {0}",
            FMetaFunctionCallFormattor_(obj COMMA *this) );

    if (IsDeprecated())
        LOG(RTTI, Warning, L"calling deprecated {0}",
            FMetaFunctionCallFormattor_(obj COMMA *this) );

    // still no support for optional parameters even if they're allowed in declaration ...
    AssertRelease(_parameters.size() == arguments.size());
    // check return value
    AssertRelease((!!_result) == (!!result));
    AssertRelease((!!_result) || (_result->TypeId() == result.TypeId()));
}
#endif //!WITH_PPE_RTTI_FUNCTION_CHECKS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::EParameterFlags flags) {
    if (flags == RTTI::EParameterFlags::Default)
        return oss << "Default";

    auto sep = Fmt::NotFirstTime('|');

    if (flags & RTTI::EParameterFlags::Output)      { oss << sep << "Output"; }
    if (flags & RTTI::EParameterFlags::Optional)    { oss << sep << "Optional"; }

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EParameterFlags flags) {
    if (flags == RTTI::EParameterFlags::Default)
        return oss << L"Default";

    auto sep = Fmt::NotFirstTime(L'|');

    if (flags & RTTI::EParameterFlags::Output)      { oss << sep << L"Output"; }
    if (flags & RTTI::EParameterFlags::Optional)    { oss << sep << L"Optional"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::EFunctionFlags flags) {
    auto sep = Fmt::NotFirstTime('|');

    if (flags & RTTI::EFunctionFlags::Const)        { oss << sep << "Const"; }
    if (flags & RTTI::EFunctionFlags::Public)       { oss << sep << "Public"; }
    if (flags & RTTI::EFunctionFlags::Protected)    { oss << sep << "Protected"; }
    if (flags & RTTI::EFunctionFlags::Private)      { oss << sep << "Private"; }
    if (flags & RTTI::EFunctionFlags::Deprecated)   { oss << sep << "Deprecated"; }

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EFunctionFlags flags) {
    auto sep = Fmt::NotFirstTime(L'|');

    if (flags & RTTI::EFunctionFlags::Const)        { oss << sep << L"Const"; }
    if (flags & RTTI::EFunctionFlags::Public)       { oss << sep << L"Public"; }
    if (flags & RTTI::EFunctionFlags::Protected)    { oss << sep << L"Protected"; }
    if (flags & RTTI::EFunctionFlags::Private)      { oss << sep << L"Private"; }
    if (flags & RTTI::EFunctionFlags::Deprecated)   { oss << sep << L"Deprecated"; }

    return oss;
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FMetaFunction& fun) {
    Format(oss, "{0} {1}({2}) [{3}]",
        fun.Result() ? fun.Result()->TypeName() : "void",
        fun.Name(),
        Fmt::Join(fun.Parameters().Map([](const RTTI::FMetaParameter& prm) {
            return Fmt::Formator<char>([&prm](FTextWriter& o) {
                o << prm.Name() << " : " << prm.Traits()->TypeName() << " <" << prm.Flags() << '>';
            }); }), ", "),
        fun.Flags() );
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaFunction& fun) {
    Format(oss, L"{0} {1}({2}) [{3}]",
        fun.Result() ? fun.Result()->TypeName() : "void",
        fun.Name(),
        Fmt::Join(fun.Parameters().Map([](const RTTI::FMetaParameter& prm) {
            return Fmt::Formator<wchar_t>([&prm](FWTextWriter& o) {
                o << prm.Name() << L" : " << prm.Traits()->TypeName() << L" <" << prm.Flags() << L'>';
            }); }), L", "),
        fun.Flags() );
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#undef CheckFunctionCallIFN
