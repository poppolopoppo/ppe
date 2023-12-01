// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MetaFunction.h"

#include <numeric>

#include "Container/Stack.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
#   include "MetaObject.h"
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
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//STATIC_ASSERT(Meta::is_pod_v<FMetaParameter>); // NOT ALLOWED with FMetaParameterFacet :/
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
        oss << STRING_LITERAL(_Char, "function ");

        if (call.pFunc->Result())
            oss << call.pFunc->Result()->TypeName();
        else
            oss << STRING_LITERAL(_Char, "void");

        oss << STRING_LITERAL(_Char, ' ') << call.pObj->RTTI_Class()->Name() << STRING_LITERAL(_Char, "::") << call.pFunc->Name() << STRING_LITERAL(_Char, '(');

        forrange(i, 0, call.pFunc->Parameters().size()) {
            if (i > 0) oss << STRING_LITERAL(_Char, ", ");
            const FMetaParameter& prm = call.pFunc->Parameters()[i];
            oss << prm.Name() << STRING_LITERAL(_Char, " : ") << prm.Traits()->TypeName() << STRING_LITERAL(_Char, " <") << prm.Flags() << STRING_LITERAL(_Char, '>');
        }

        oss << STRING_LITERAL(_Char, ") on object \"")
            << call.pObj->RTTI_Name()
            << STRING_LITERAL(_Char, "\" (")
            << call.pObj->RTTI_Flags()
            << STRING_LITERAL(_Char, ')');

        return oss;
    }
};
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//PPE_ASSERT_TYPE_IS_POD(FMetaParameter); // NOT ALLOWED with FMetaParameterFacet :/
//----------------------------------------------------------------------------
FMetaParameter::FMetaParameter() NOEXCEPT {
    _traitsAndFlags.Reset(nullptr, false, false);
}
//----------------------------------------------------------------------------
FMetaParameter::FMetaParameter(const FName& name, const PTypeTraits& traits, EParameterFlags flags) NOEXCEPT
:   _name(name) {
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
:   _invoke(nullptr)
,   _flags(EFunctionFlags(0))
{}
//----------------------------------------------------------------------------
FMetaFunction::FMetaFunction(
    const FName& name,
    EFunctionFlags flags,
    const PTypeTraits& result,
    TRValueInitializerList<FMetaParameter> parameters,
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
bool FMetaFunction::InvokeIFP_(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments, bool preferMove) const {
    Assert(_invoke);

    if (arguments.size() != _parameters.size()) {
#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
        PPE_LOG(RTTI, Warning, "given {0} arguments instead of {1} when calling {2}",
            arguments.size(), _parameters.size(),
            FMetaFunctionCallFormattor_(obj COMMA *this) );
#endif
        return false;
    }

    if (_result.Valid() != result.Valid()) {
#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
        PPE_LOG(RTTI, Warning, "no result is returned by {0}",
            FMetaFunctionCallFormattor_(obj COMMA *this) );
#endif
        return false;
    }

    size_t strideInBytes = ((_result && result.Traits() != _result) ? _result->SizeInBytes() : 0);
    forrange(i, 0, _parameters.size()) {
        Assert(arguments[i]);

        if (_parameters[i].Traits() != arguments[i].Traits()) {
            const FSizeAndFlags sizeAndFlags = _parameters[i].Traits()->SizeAndFlags();
            strideInBytes = Meta::RoundToNextPow2(strideInBytes, sizeAndFlags.Alignment());
            strideInBytes += sizeAndFlags.SizeInBytes;
        }
    }

    if (Likely(0 == strideInBytes)) {
        CheckFunctionCallIFN(obj, result, arguments);
        _invoke(obj, result, arguments);
        return true;
    }

    return PromoteInvoke_(obj, result, arguments, strideInBytes, preferMove);
}
//----------------------------------------------------------------------------
bool FMetaFunction::PromoteInvoke_(
    const FMetaObject& obj,
    const FAtom& result,
    const TMemoryView<const FAtom>& arguments,
    const size_t strideInBytes,
    bool preferMove ) const {
    const bool resultPromotion = (_result && result.Traits() != _result);

    STACKLOCAL_POD_ARRAY(u8, rawData, ROUND_TO_NEXT_16(strideInBytes) + _parameters.size() * sizeof(FAtom));
    const TMemoryView<FAtom> nativeArgs = rawData.CutStartingAt(
        ROUND_TO_NEXT_16(strideInBytes) ).Cast<FAtom>();

    FAtom nativeResult = result;

    size_t offsetInBytes = 0;
    if (resultPromotion) {
        const FSizeAndFlags sizeAndFlags = _result->SizeAndFlags();
        Assert(Meta::IsAlignedPow2(sizeAndFlags.Alignment(), offsetInBytes));

        nativeResult = FAtom{ rawData.SubRange(offsetInBytes, offsetInBytes + sizeAndFlags.SizeInBytes).data(), _result };
        Assert(Meta::IsAlignedPow2(sizeAndFlags.Alignment(), nativeResult.Data()));
        _result->Construct(nativeResult.Data());

        offsetInBytes += sizeAndFlags.SizeInBytes;
    }

    bool succeed = true;

    forrange(i, 0, _parameters.size()) {
        FAtom& nativeArg = nativeArgs[i];
        const PTypeTraits& traits = _parameters[i].Traits();
        if (traits != arguments[i].Traits()) {
            const FSizeAndFlags sizeAndFlags = traits->SizeAndFlags();
            offsetInBytes = Meta::RoundToNextPow2(sizeAndFlags.Alignment(), offsetInBytes);

            nativeArg = FAtom{ rawData.SubRange(offsetInBytes, offsetInBytes + sizeAndFlags.SizeInBytes).data(), traits };
            Assert(Meta::IsAlignedPow2(sizeAndFlags.Alignment(), nativeArg.Data()));
            nativeArg.Traits()->Construct(nativeArg.Data());

            const bool promoted = (preferMove
                ? arguments[i].PromoteMove(nativeArg)
                : arguments[i].PromoteCopy(nativeArg) );

            if (not promoted) {
                succeed = false;
#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
                PPE_LOG(RTTI, Error, "wrong type for argument #{1} {2} instead of {3} when calling {0}, promote {4} failed",
                    FMetaFunctionCallFormattor_(obj COMMA * this),
                    i, arguments[i].Traits()->NamedTypeInfos(), traits->NamedTypeInfos(),
                    preferMove ? "move"_view : "copy"_view);
#endif
            }

            offsetInBytes += sizeAndFlags.SizeInBytes;
        }
        else {
            nativeArg = arguments[i];
        }
    }

    Assert_NoAssume(offsetInBytes == strideInBytes);

    if (succeed) {
        CheckFunctionCallIFN(obj, nativeResult, nativeArgs);

        _invoke(obj, nativeResult, nativeArgs);

        if (resultPromotion) {
            Assert(nativeResult.Data() != result.Data());

            if (not nativeResult.PromoteMove(result)) {
#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
                PPE_LOG(RTTI, Warning, "wrong result type <{1}> when calling {0}",
                    FMetaFunctionCallFormattor_(obj COMMA * this),
                    result.Traits()->NamedTypeInfos());
#endif
                succeed = false;
            }

            nativeResult.Traits()->Destroy(nativeResult.Data());
        }
    }

    forrange(i, 0, _parameters.size()) {
        FAtom& arg = nativeArgs[i];
        if (arg.Data() != arguments[i].Data())
            arg.Traits()->Destroy(arg.Data());
    }

    return succeed;
}
//----------------------------------------------------------------------------
#ifdef WITH_PPE_RTTI_FUNCTION_CHECKS
void FMetaFunction::CheckFunctionCall_(
    const FMetaObject& obj,
    const FAtom& result,
    const TMemoryView<const FAtom>& arguments ) const {

    PPE_CLOG(not IsConst() && obj.RTTI_IsFrozen(),
        RTTI, Fatal, "can't use frozen object with non-const {0}",
        FMetaFunctionCallFormattor_(obj COMMA *this) );

    PPE_CLOG(IsDeprecated(),
        RTTI, Warning, "calling deprecated {0}",
        FMetaFunctionCallFormattor_(obj COMMA *this) );

    PPE_CLOG(_parameters.size() != arguments.size(), // optional parameters must still be provided with their default value
        RTTI, Fatal, "invalid arguments count for {0}, expected {1} but given {2}",
        FMetaFunctionCallFormattor_(obj COMMA * this), _parameters.size(), arguments.size());

    forrange(i, 0, _parameters.size()) {
        const PTypeTraits expected = _parameters[i].Traits();
        const PTypeTraits given = arguments[i].Traits();
        const PTypeTraits common = expected->CommonType(given);

        PPE_CLOG(not common.Valid(),
            RTTI, Fatal, "invalid argument #{1} for {0}, expected {2} but given {3} (common type = {4})",
            FMetaFunctionCallFormattor_(obj COMMA * this),
            i, expected->NamedTypeInfos(), given->NamedTypeInfos(), common->NamedTypeInfos() );
    }

    if (_result)
        PPE_CLOG(_result->TypeId() != result.TypeId(),
            RTTI, Fatal, "invalid return value for {0}, expected {1} but given {2}",
            FMetaFunctionCallFormattor_(obj COMMA* this),
            _result->NamedTypeInfos(), result.Traits()->NamedTypeInfos() );
    else
        PPE_CLOG(!!result,
            RTTI, Fatal, "function {0} has no return type, but given {1}",
            FMetaFunctionCallFormattor_(obj COMMA* this),
            result.Traits()->NamedTypeInfos() );
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
template <typename _Char>
static TBasicTextWriter<_Char>& Print_(TBasicTextWriter<_Char>& oss, RTTI::EParameterFlags flags) {
    if (flags == RTTI::EParameterFlags::Default)
        return oss << STRING_LITERAL(_Char, "Default");

    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, '|'));

    if (flags & RTTI::EParameterFlags::Output)      { oss << sep << STRING_LITERAL(_Char, "Output"); }
    if (flags & RTTI::EParameterFlags::Optional)    { oss << sep << STRING_LITERAL(_Char, "Optional"); }

    return oss;
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::EParameterFlags flags) {
    return Print_(oss, flags);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EParameterFlags flags) {
    return Print_(oss, flags);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& Print_(TBasicTextWriter<_Char>& oss, RTTI::EFunctionFlags flags) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, '|'));

    if (flags & RTTI::EFunctionFlags::Const)        { oss << sep << STRING_LITERAL(_Char, "Const"); }
    if (flags & RTTI::EFunctionFlags::Public)       { oss << sep << STRING_LITERAL(_Char, "Public"); }
    if (flags & RTTI::EFunctionFlags::Protected)    { oss << sep << STRING_LITERAL(_Char, "Protected"); }
    if (flags & RTTI::EFunctionFlags::Private)      { oss << sep << STRING_LITERAL(_Char, "Private"); }
    if (flags & RTTI::EFunctionFlags::Deprecated)   { oss << sep << STRING_LITERAL(_Char, "Deprecated"); }

    return oss;
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::EFunctionFlags flags) {
    return Print_(oss, flags);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EFunctionFlags flags) {
    return Print_(oss, flags);
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& Print_(TBasicTextWriter<_Char>& oss, const RTTI::FMetaFunction& fun) {
    Format(oss, STRING_LITERAL(_Char, "{0} {1}({2}) [{3}]"),
        fun.Result() ? fun.Result()->TypeName() : "void",
        fun.Name(),
        Fmt::Join(fun.Parameters().Map([](const RTTI::FMetaParameter& prm) {
            return Fmt::Formator<wchar_t>([&prm](FWTextWriter& o) {
                o << prm.Name() << STRING_LITERAL(_Char, " : ") << prm.Traits()->TypeName() << STRING_LITERAL(_Char, " <") << prm.Flags() << STRING_LITERAL(_Char, '>');
            }); }), STRING_LITERAL(_Char, ", ")),
        fun.Flags() );
    return oss;
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FMetaFunction& fun) {
    return Print_(oss, fun);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaFunction& fun) {
    return Print_(oss, fun);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#undef CheckFunctionCallIFN
