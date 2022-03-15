#include "stdafx.h"

#include "Build/CommandNode.h"

#include "BuildContext.h"
#include "BuildGraph.h"
#include "BuildLog.h"
#include "Build/FileHelpers.h"
#include "Build/FileNode.h"

#include "RTTI/Macros-impl.h"

#include "IO/FileSystemProperties.h"
#include "IO/FormatHelpers.h" // Quoted
#include "IO/StringBuilder.h"
#include "HAL/PlatformProcess.h"
#include "Memory/MemoryStream.h"
#include "Misc/Process.h"
#include "VirtualFileSystem_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void FormatParameters_(
    FWStringBuilder& outp,
    const FFilename& destination,
    const FCommandNode::FDependenciesView& sources,
    const FCommandNode::FParameters& parameters ) {
    auto prmFormat = [&](const FWStringView& prm) {
        if (not outp.Written().empty())
            outp << Fmt::Space;

        if (prm.empty() || HasSpace(prm))
            outp << Fmt::Quoted(prm, Fmt::DoubleQuote);
        else
            outp << prm;
    };

    for (const FWString& prm : parameters) {
        if (prm.front() == L'$') {
            if (Equals(prm, FCommandNode::Token_Input)) {
                // assumes dynamic deps <=> input files to avoid calling again FillFileNodeList()
                for (const PBuildNode& input : sources) {
                    const FFileNode* const file = RTTI::CastChecked<FFileNode>(input.get());
                    prmFormat(VFS_Unalias(file->Filename()));
                }
                continue;
            }
            else if (Equals(prm, FCommandNode::Token_Output)) {
                prmFormat(VFS_Unalias(destination));
                continue;
            }
        }

        prmFormat(prm);
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(BuildGraph, FCommandNode, Concrete)
RTTI_PROPERTY_PRIVATE_READONLY(_input)
RTTI_PROPERTY_PRIVATE_READONLY(_executable)
RTTI_PROPERTY_PRIVATE_READONLY(_workingDir)
RTTI_PROPERTY_PRIVATE_READONLY(_parameters)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FCommandNode::RTTI_VerifyPredicates() const PPE_THROW() {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(_input);
    RTTI_VerifyPredicate(not _executable.empty());
}
#endif
//----------------------------------------------------------------------------
// Command-line format:
// --------------------
//
// The format string must contain at least of those 3 selectors:
//  - ${input}   : input file(s), as provided via Input
//  - ${output}  : output file, as provided by Filename
//
const FWStringView FCommandNode::Token_Input = L"${input}";
const FWStringView FCommandNode::Token_Output = L"${output}";
//----------------------------------------------------------------------------
FCommandNode::FCommandNode() NOEXCEPT
{}
//----------------------------------------------------------------------------
FCommandNode::FCommandNode(
    FBuildNode* input,
    const FFilename& outputFile,
    const FFilename& executable,
    const FDirpath& workingDirIFN,
    FParameters&& parameters ) NOEXCEPT {
    SetInput(input);
    SetFilename(outputFile);
    SetExecutable(executable, workingDirIFN, std::move(parameters));
}
//----------------------------------------------------------------------------
EBuildResult FCommandNode::Scan(FScanContext& ctx) {
    AddStaticDep(ctx.GetOrCreateFileNode(_executable));
    AddStaticDep(_input.get());

    ctx.AddOutputFile(this, Filename());

    return FFileNode::Scan(ctx);
}
//----------------------------------------------------------------------------
EBuildResult FCommandNode::Import(FBuildContext& ctx) {
    FFileNodeList inputFiles;
    FillFileNodeList(&inputFiles, *_input);

    if (inputFiles.empty()) {
        ctx.Log().TraceError(L"can't find any input files for command node '{0}'", Filename());
        return EBuildResult::Failed;
    }

    AddDynamicDeps(inputFiles.begin(), inputFiles.end());

    return FFileNode::Import(ctx);
}
//----------------------------------------------------------------------------
EBuildResult FCommandNode::Process(FBuildContext& ctx) {
    FWStringBuilder cmdline;
    FormatParameters_(cmdline, Filename(), DynamicDeps(), _parameters);

    MEMORYSTREAM(BuildGraph) outp;
    const int exitCode = FProcess::CaptureOutput(
        &outp, &outp,
        VFS_Unalias(_executable),
        cmdline.ToString(),
        VFS_Unalias(_workingDir),
        FProcess::NoWindow );

    if (0 == exitCode) {
        return EBuildResult::Built;
    }
    else {
        ctx.Log().TraceError(ToWString(outp.MakeView().Cast<const char>()));
        return EBuildResult::Failed;
    }
}
//----------------------------------------------------------------------------
EBuildResult FCommandNode::Clean(FCleanContext& ctx) {
    const FFilename& filename = Filename();

    if (VFS_RemoveFile(filename)) {
        ctx.Log().TraceInfo(L"deleted command node output file '{0}'", filename);
        return EBuildResult::Built;
    }
    else {
        return EBuildResult::UpToDate;
    }
}
//----------------------------------------------------------------------------
void FCommandNode::SetInput(FBuildNode* input) NOEXCEPT {
    Assert(input);

    _input = input;
}
//----------------------------------------------------------------------------
void FCommandNode::SetExecutable(
    const FFilename& executable,
    const FDirpath& workingDir,
    FParameters&& parameters) NOEXCEPT {
    Assert(not executable.empty());

    _executable = executable;
    _workingDir = workingDir;
    _parameters = std::move(parameters);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
