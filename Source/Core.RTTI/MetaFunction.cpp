#include "stdafx.h"

#include "MetaFunction.h"

#include "Core/IO/FormatHelpers.h"
#include "Core/IO/TextWriter.h"

#if USE_CORE_RTTI_CHECKS
#   include "MetaObject.h"
#   include "Core/Diagnostic/Logger.h"
namespace Core {
namespace RTTI {
EXTERN_LOG_CATEGORY(CORE_RTTI_API, RTTI);
} //!namespace RTTI
} //!namespace Core
#endif

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaParameter::FMetaParameter() {
    _traitsAndFlags.Reset(nullptr, false, false);
}
//----------------------------------------------------------------------------
FMetaParameter::FMetaParameter(const FName& name, const PTypeTraits& traits, EParameterFlags flags)
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
FMetaFunction::FMetaFunction()
    : _invoke(nullptr)
    , _flags(EFunctionFlags(0))
{}
//----------------------------------------------------------------------------
FMetaFunction::FMetaFunction(
    const FName& name,
    EFunctionFlags flags,
    const PTypeTraits& result,
    std::initializer_list<FMetaParameter> parameters,
    invoke_func invoke )
    : _name(name)
    , _invoke(invoke)
    , _flags(flags)
    , _result(result)
    , _parameters(parameters) {
    Assert(not _name.empty());
    Assert(_invoke);

#if USE_CORE_RTTI_CHECKS
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
FMetaFunction::~FMetaFunction()
{}
//----------------------------------------------------------------------------
void FMetaFunction::Invoke(
    const FMetaObject& obj,
    const FAtom& result,
    const TMemoryView<const FAtom>& arguments ) const {
    Assert(_invoke);
    Assert(result || not HasReturnValue());
    Assert(arguments.size() <= _parameters.size()); // can have optional parameters

#if USE_CORE_RTTI_CHECKS
    if (IsDeprecated()) {
        LOG(RTTI, Warning, L"using deprecated function \"{0} {1}::{2}({3})\" on \"{4}\" ({5})",
            _result ? _result->TypeInfos().Name() : "void",
            obj.RTTI_Class()->Name(),
            _name,
            Fmt::FWFormator([this](FWTextWriter& oss) {
                const auto& prms = this->Parameters();
                forrange(i, 0, prms.size()) {
                    if (i > 0) oss << L", ";
                    oss << prms[i].Name() << L" : " << prms[i].Traits()->TypeInfos().Name() << L" <" << prms[i].Flags() << L'>';
                }
            }),
            obj.RTTI_Name(),
            obj.RTTI_Flags() );
    }
    // still no support for optional parameters even if they're allowed in declaration ...
    AssertRelease(_parameters.size() == arguments.size());
    // check that types of given arguments match the parameters
    forrange(i, 0, arguments.size()) {
        Assert(arguments[i].Cast(_parameters[i].Traits()));
    }
#endif

    _invoke(obj, result, arguments);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
