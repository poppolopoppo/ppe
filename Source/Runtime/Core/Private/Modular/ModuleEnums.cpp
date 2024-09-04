// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Modular/ModuleEnums.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& FormatEnum_(TBasicTextWriter<_Char>& oss, EModulePhase phase) {
    switch (phase) {
    case EModulePhase::Bare: return oss << STRING_LITERAL(_Char, "Bare");
    case EModulePhase::System: return oss << STRING_LITERAL(_Char, "System");
    case EModulePhase::Framework: return oss << STRING_LITERAL(_Char, "Framework");
    case EModulePhase::Application: return oss << STRING_LITERAL(_Char, "Application");
    case EModulePhase::User: return oss << STRING_LITERAL(_Char, "User");
    case EModulePhase::_Max: AssertNotReached();
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& FormatEnum_(TBasicTextWriter<_Char>& oss, EModuleSource source) {
    switch (source) {
    case EModuleSource::Core: return oss << STRING_LITERAL(_Char, "Core");
    case EModuleSource::Program: return oss << STRING_LITERAL(_Char, "Program");
    case EModuleSource::Extension: return oss << STRING_LITERAL(_Char, "Extension");
    case EModuleSource::External: return oss << STRING_LITERAL(_Char, "External");
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& FormatEnum_(TBasicTextWriter<_Char>& oss, EModuleUsage usage) {
    switch (usage) {
    case EModuleUsage::Runtime: return oss << STRING_LITERAL(_Char, "Runtime");
    case EModuleUsage::Shipping: return oss << STRING_LITERAL(_Char, "Shipping");
    case EModuleUsage::Tools: return oss << STRING_LITERAL(_Char, "Tools");
    case EModuleUsage::Developer: return oss << STRING_LITERAL(_Char, "Developer");
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EModulePhase phase) {
    return FormatEnum_(oss, phase);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EModulePhase phase) {
    return FormatEnum_(oss, phase);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EModuleSource source) {
    return FormatEnum_(oss, source);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EModuleSource source) {
    return FormatEnum_(oss, source);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EModuleUsage usage) {
   return FormatEnum_(oss, usage);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EModuleUsage usage) {
    return FormatEnum_(oss, usage);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
