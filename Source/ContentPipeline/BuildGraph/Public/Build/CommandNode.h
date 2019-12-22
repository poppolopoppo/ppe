#pragma once

#include "BuildGraph_fwd.h"

#include "FileNode.h"

#include "Container/Vector.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/String.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(CommandNode);
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FCommandNode : public FFileNode {
    RTTI_CLASS_HEADER(PPE_BUILDGRAPH_API, FCommandNode, FFileNode);
public:
    using FParameters = VECTORINSITU(BuildGraph, FWString, 3);

    FCommandNode() NOEXCEPT;

    FCommandNode(
        FBuildNode* input,
        const FFilename& outputFile,
        const FFilename& executable,
        const FDirpath& workingDirIFN,
        FParameters&& parameters ) NOEXCEPT;

    const PBuildNode& Input() const { return _input; }
    const FFilename& Executable() const { return _executable; }
    const FDirpath& WorkingDir() const { return _workingDir; }
    const FParameters& Parameters() const { return _parameters; }

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const PPE_THROW();
#endif

    // special argument tokens
    static const FWStringView Token_Input;
    static const FWStringView Token_Output;

public: // FBuildNode:
    virtual EBuildResult Scan(FScanContext& ctx) override;
    virtual EBuildResult Import(FBuildContext& ctx) override;
    virtual EBuildResult Process(FBuildContext& ctx) override;
    virtual EBuildResult Clean(FCleanContext& ctx) override;

protected:
    void SetInput(FBuildNode* input) NOEXCEPT;
    void SetExecutable(
        const FFilename& executable,
        const FDirpath& workingDir,
        FParameters&& parameters) NOEXCEPT;

private:
    PBuildNode _input;
    FFilename _executable;
    FDirpath _workingDir;
    FParameters _parameters;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
