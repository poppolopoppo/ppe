#include "stdafx.h"

#include "StreamPolicies.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EAccessPolicy policy) {
    bool notFirst = false;
    for (size_t i = 0; i < 32; ++i) {
        switch(EAccessPolicy(size_t(policy) & (size_t(1) << i))) {
        case EAccessPolicy::None: continue;
        case EAccessPolicy::Binary:     oss << Fmt::Conditional('|', notFirst) << "Binary"; break;
        case EAccessPolicy::Text:       oss << Fmt::Conditional('|', notFirst) << "Text"; break;
        case EAccessPolicy::TextU8:     oss << Fmt::Conditional('|', notFirst) << "TextU8"; break;
        case EAccessPolicy::TextU16:    oss << Fmt::Conditional('|', notFirst) << "TextU16"; break;
        case EAccessPolicy::TextW:      oss << Fmt::Conditional('|', notFirst) << "TextW"; break;
        case EAccessPolicy::Create:     oss << Fmt::Conditional('|', notFirst) << "Create"; break;
        case EAccessPolicy::Append:     oss << Fmt::Conditional('|', notFirst) << "Append"; break;
        case EAccessPolicy::Truncate:   oss << Fmt::Conditional('|', notFirst) << "Truncate"; break;
        case EAccessPolicy::Random:     oss << Fmt::Conditional('|', notFirst) << "Random"; break;
        case EAccessPolicy::Sequential: oss << Fmt::Conditional('|', notFirst) << "Sequential"; break;
        case EAccessPolicy::ShortLived: oss << Fmt::Conditional('|', notFirst) << "ShortLived"; break;
        case EAccessPolicy::Temporary:  oss << Fmt::Conditional('|', notFirst) << "Temporary"; break;
        case EAccessPolicy::Exclusive:  oss << Fmt::Conditional('|', notFirst) << "Exclusive"; break;
        case EAccessPolicy::Compress:   oss << Fmt::Conditional('|', notFirst) << "Compress"; break;
        default:
            AssertNotImplemented();
            break;
        };
        notFirst = true;
    }
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EAccessPolicy policy) {
    bool notFirst = false;
    for (size_t i = 0; i < 32; ++i) {
        switch(EAccessPolicy(size_t(policy) & (size_t(1) << i))) {
        case EAccessPolicy::None: continue;
        case EAccessPolicy::Binary:     oss << Fmt::Conditional(L'|', notFirst) << L"Binary"; break;
        case EAccessPolicy::Text:       oss << Fmt::Conditional(L'|', notFirst) << L"Text"; break;
        case EAccessPolicy::TextU8:     oss << Fmt::Conditional(L'|', notFirst) << L"TextU8"; break;
        case EAccessPolicy::TextU16:    oss << Fmt::Conditional(L'|', notFirst) << L"TextU16"; break;
        case EAccessPolicy::TextW:      oss << Fmt::Conditional(L'|', notFirst) << L"TextW"; break;
        case EAccessPolicy::Create:     oss << Fmt::Conditional(L'|', notFirst) << L"Create"; break;
        case EAccessPolicy::Append:     oss << Fmt::Conditional(L'|', notFirst) << L"Append"; break;
        case EAccessPolicy::Truncate:   oss << Fmt::Conditional(L'|', notFirst) << L"Truncate"; break;
        case EAccessPolicy::Random:     oss << Fmt::Conditional(L'|', notFirst) << L"Random"; break;
        case EAccessPolicy::Sequential: oss << Fmt::Conditional(L'|', notFirst) << L"Sequential"; break;
        case EAccessPolicy::ShortLived: oss << Fmt::Conditional(L'|', notFirst) << L"ShortLived"; break;
        case EAccessPolicy::Temporary:  oss << Fmt::Conditional(L'|', notFirst) << L"Temporary"; break;
        case EAccessPolicy::Exclusive:  oss << Fmt::Conditional(L'|', notFirst) << L"Exclusive"; break;
        case EAccessPolicy::Compress:   oss << Fmt::Conditional(L'|', notFirst) << L"Compress"; break;
        default:
            AssertNotImplemented();
            break;
        };
        notFirst = true;
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core