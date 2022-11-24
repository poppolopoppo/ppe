// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/StreamPolicies.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EOpenPolicy policy) {
    switch (policy) {
    case PPE::EOpenPolicy::ReadWritable:
        return oss << "ReadWritable";
    case PPE::EOpenPolicy::Readable:
        return oss << "Readable";
    case PPE::EOpenPolicy::Writable:
        return oss << "Writable";
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EOpenPolicy policy) {
    switch (policy) {
    case PPE::EOpenPolicy::ReadWritable:
        return oss << L"ReadWritable";
    case PPE::EOpenPolicy::Readable:
        return oss << L"Readable";
    case PPE::EOpenPolicy::Writable:
        return oss << L"Writable";
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EAccessPolicy policy) {
    bool notFirst = false;
    for (size_t i = 0; i < 32; ++i) {
        switch(EAccessPolicy(size_t(policy) & (size_t(1) << i))) {
        case EAccessPolicy::None:       continue;
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
        case EAccessPolicy::ShareRead:  oss << Fmt::Conditional('|', notFirst) << "ShareRead"; break;
        case EAccessPolicy::Roll:       oss << Fmt::Conditional('|', notFirst) << "Roll"; break;
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
        case EAccessPolicy::None:       continue;
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
        case EAccessPolicy::ShareRead:  oss << Fmt::Conditional(L'|', notFirst) << L"ShareRead"; break;
        case EAccessPolicy::Roll:       oss << Fmt::Conditional(L'|', notFirst) << L"Roll"; break;
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
} //!namespace PPE
