#include "stdafx.h"

#include "BuildEnums.h"

#include "IO/FormatHelpers.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ContentPipeline::EBuildFlags flags) {
    using ContentPipeline::EBuildFlags;

    auto sep = Fmt::NotFirstTime('|');

    if (flags & EBuildFlags::Rebuild) { oss << sep << "Rebuild"; }
    if (flags & EBuildFlags::CacheRead) { oss << sep << "CacheRead"; }
    if (flags & EBuildFlags::CacheWrite) { oss << sep << "CacheWrite"; }
    if (flags & EBuildFlags::DryRun) { oss << sep << "DryRun"; }
    if (flags & EBuildFlags::StopOnError) { oss << sep << "StopOnError"; }
    if (flags & EBuildFlags::Verbose) { oss << sep << "Verbose"; }

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ContentPipeline::EBuildFlags flags) {
    using ContentPipeline::EBuildFlags;

    auto sep = Fmt::NotFirstTime(L'|');

    if (flags & EBuildFlags::Rebuild) { oss << sep << L"Rebuild"; }
    if (flags & EBuildFlags::CacheRead) { oss << sep << L"CacheRead"; }
    if (flags & EBuildFlags::CacheWrite) { oss << sep << L"CacheWrite"; }
    if (flags & EBuildFlags::DryRun) { oss << sep << L"DryRun"; }
    if (flags & EBuildFlags::StopOnError) { oss << sep << L"StopOnError"; }
    if (flags & EBuildFlags::Verbose) { oss << sep << L"Verbose"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ContentPipeline::EBuildResult result) {
    using ContentPipeline::EBuildResult;

    switch (result) {
    case EBuildResult::Unbuilt:
        return oss << "Unbuilt";
    case EBuildResult::UpToDate:
        return oss << "UpToDate";
    case EBuildResult::Built:
        return oss << "Built";
    case EBuildResult::Failed:
        return oss << "Failed";
    default:
        AssertNotReached();
    }
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ContentPipeline::EBuildResult result) {
    using ContentPipeline::EBuildResult;

    switch (result) {
    case EBuildResult::Unbuilt:
        return oss << L"Unbuilt";
    case EBuildResult::UpToDate:
        return oss << L"UpToDate";
    case EBuildResult::Built:
        return oss << L"Built";
    case EBuildResult::Failed:
        return oss << L"Failed";
    default:
        AssertNotReached();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE