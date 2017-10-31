#include "stdafx.h"

#include "MetaFunction.h"

#if USE_CORE_RTTI_CHECKS
#   include "MetaObject.h"
#   include "Core/Diagnostic/Logger.h"
#   include "Core/IO/FormatHelpers.h"
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
    Assert(arguments.size() == _parameters.size());

#if USE_CORE_RTTI_CHECKS
    if (IsDeprecated()) {
        LOG(Warning, L"[RTTI] Using deprecated function \"{0} {1}::{2}({3})\" on \"{4}\" ({5})",
            _result ? _result->TypeInfos().Name() : "void",
            obj.RTTI_Class()->Name(),
            _name,
            Fmt::FWFormator([this](std::basic_ostream<wchar_t>& oss) {
                const auto& prms = this->Parameters();
                forrange(i, 0, prms.size()) {
                    if (i > 0) oss << L", ";
                    oss << prms[i].Name() << L" : " << prms[i].Traits()->TypeInfos().Name() << L" <" << prms[i].Flags() << L'>';
                }
            }),
            obj.RTTI_Name(),
            obj.RTTI_Flags() );
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
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, RTTI::EParameterFlags flags) {
    if (flags == RTTI::EParameterFlags::Default)
        return oss << "Default";

    bool s = false;

    if (flags & RTTI::EParameterFlags::Output)      { if (s) oss << '|'; else s = true; oss << "Output"; }
    if (flags & RTTI::EParameterFlags::Optional)    { if (s) oss << '|'; else s = true; oss << "Optional"; }

    return oss;
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, RTTI::EParameterFlags flags) {
    if (flags == RTTI::EParameterFlags::Default)
        return oss << L"Default";

    bool s = false;

    if (flags & RTTI::EParameterFlags::Output)      { if (s) oss << L'|'; else s = true; oss << L"Output"; }
    if (flags & RTTI::EParameterFlags::Optional)    { if (s) oss << L'|'; else s = true; oss << L"Optional"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, RTTI::EFunctionFlags flags) {
    bool s = false;

    if (flags & RTTI::EFunctionFlags::Const)        { if (s) oss << '|'; else s = true; oss << "Const"; }
    if (flags & RTTI::EFunctionFlags::Public)       { if (s) oss << '|'; else s = true; oss << "Public"; }
    if (flags & RTTI::EFunctionFlags::Protected)    { if (s) oss << '|'; else s = true; oss << "Protected"; }
    if (flags & RTTI::EFunctionFlags::Private)      { if (s) oss << '|'; else s = true; oss << "Private"; }
    if (flags & RTTI::EFunctionFlags::Deprecated)   { if (s) oss << '|'; else s = true; oss << "Deprecated"; }

    return oss;
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, RTTI::EFunctionFlags flags) {
    bool s = false;

    if (flags & RTTI::EFunctionFlags::Const)        { if (s) oss << L'|'; else s = true; oss << L"Const"; }
    if (flags & RTTI::EFunctionFlags::Public)       { if (s) oss << L'|'; else s = true; oss << L"Public"; }
    if (flags & RTTI::EFunctionFlags::Protected)    { if (s) oss << L'|'; else s = true; oss << L"Protected"; }
    if (flags & RTTI::EFunctionFlags::Private)      { if (s) oss << L'|'; else s = true; oss << L"Private"; }
    if (flags & RTTI::EFunctionFlags::Deprecated)   { if (s) oss << L'|'; else s = true; oss << L"Deprecated"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
