#include "stdafx.h"

#include "Modular/ModuleEnums.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EModulePhase phase) {
    switch (phase) {
    case EModulePhase::Bare: return oss << "Bare";
    case EModulePhase::System: return oss << "System";
    case EModulePhase::Framework: return oss << "Framework";
    case EModulePhase::Application: return oss << "Application";
    case EModulePhase::User: return oss << "User";
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EModulePhase phase) {
    switch (phase) {
    case EModulePhase::Bare: return oss << L"Bare";
    case EModulePhase::System: return oss << L"System";
    case EModulePhase::Framework: return oss << L"Framework";
    case EModulePhase::Application: return oss << L"Application";
    case EModulePhase::User: return oss << L"User";
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EModuleSource source) {
    switch (source) {
    case EModuleSource::Core: return oss << "Core";
    case EModuleSource::Program: return oss << "Program";
    case EModuleSource::Extensions: return oss << "Extensions";
    case EModuleSource::External: return oss << "External";
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EModuleSource source) {
    switch (source) {
    case EModuleSource::Core: return oss << "Core";
    case EModuleSource::Program: return oss << "Program";
    case EModuleSource::External: return oss << "External";
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EModuleUsage usage) {
    switch (usage) {
    case EModuleUsage::Runtime: return oss << "Runtime";
    case EModuleUsage::Shipping: return oss << "Shipping";
    case EModuleUsage::Tools: return oss << "Tools";
    case EModuleUsage::Developer: return oss << "Developer";
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EModuleUsage usage) {
    switch (usage) {
    case EModuleUsage::Runtime: return oss << L"Runtime";
    case EModuleUsage::Shipping: return oss << L"Shipping";
    case EModuleUsage::Tools: return oss << L"Tools";
    case EModuleUsage::Developer: return oss << L"Developer";
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
