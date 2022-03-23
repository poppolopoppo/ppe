#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanSpirvCompiler.h"

#include "Vulkan/Instance/VulkanInstance.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Pipeline/VulkanDebuggableShaderData.h"
#include "Vulkan/Pipeline/VulkanShaderCompilationFlags.h"

#include "RHI/EnumHelpers.h"

#include "Container/BitMask.h"
#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/Filename.h"
#include "Memory/PtrRef.h"
#include "Memory/UniquePtr.h"
#include "Meta/Functor.h"
#include "Misc/Guid.h"
#include "VirtualFileSystem_fwd.h"

#include "glslang-external.h"
#include "glsl_trace-external.h"
#include "spirv-tools-external.h"

#include "glslang/Public/ShaderLang.h"
#include "glslang/MachineIndependent/localintermediate.h"
#include "glslang/Include/intermediate.h"
#include "glslang/SPIRV/doc.h"
#include "glslang/SPIRV/disassemble.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/SPIRV/GLSL.std.450.h"
#include "glslang/SPIRV/SpvTools.h"

//#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/libspirv.h"

#include <string>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Used as seed for every shader fingerprint: invalidating it will recompile all shaders
static const FGuid GVulkanSpirvFingerprint{ {{
    0X3550B8BAull,
    0xA0674724ull,
    0xAAA5336Cull,
    0xB554D506ull,
}}};
//----------------------------------------------------------------------------
NODISCARD u32 GLSLangArraySize_(const glslang::TType& type) {
    const glslang::TArraySizes* const sizes = type.getArraySizes();

    if (not sizes or sizes->getNumDims() <= 0)
        return 1;

    AssertRelease(sizes->getNumDims() == 1);
    return checked_cast<u32>(sizes->getDimSize(0));
}
//----------------------------------------------------------------------------
NODISCARD EShLanguage EShaderType_ToShLanguage(EShaderType shaderType) {
    switch (shaderType) {
    case EShaderType::Vertex: return EShLangVertex;
    case EShaderType::TessControl: return EShLangTessControl;
    case EShaderType::TessEvaluation: return EShLangTessEvaluation;;
    case EShaderType::Geometry: return EShLangGeometry;
    case EShaderType::Fragment: return EShLangFragment;
    case EShaderType::Compute: return EShLangCompute;
    case EShaderType::MeshTask: return EShLangTaskNV;
    case EShaderType::Mesh: return EShLangMeshNV;
    case EShaderType::RayGen: return EShLangRayGenNV;
    case EShaderType::RayAnyHit: return EShLangAnyHitNV;
    case EShaderType::RayClosestHit: return EShLangClosestHitNV;
    case EShaderType::RayMiss: return EShLangMissNV;
    case EShaderType::RayIntersection: return EShLangIntersectNV;
    case EShaderType::RayCallable: return EShLangCallableNV;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
struct FGLSLangError {
    FWStringView Filename;
    FWStringView Description;
    u32 SourceIndex{ 0 };
    u32 Line{ 0 };
    bool IsError{ false };

    bool Parse(FWStringView in) {
        CONSTEXPR FWStringView c_error = L"error"_view;
        CONSTEXPR FWStringView c_warning = L"warning"_view;

        EatSpaces(in);

        if (StartsWithI(in, c_error)) {
            in = in.CutStartingAt(c_error.size());
            IsError = true;
        }
        else
        if (StartsWithI(in, c_warning)) {
            in = in.CutStartingAt(c_warning.size());
            IsError = false;
        }
        else {
            return false;
        }

        if (in.front() != L':')
            return false;
        in = in.ShiftFront();

        EatSpaces(in);

        const FWStringView id = EatAlnums(in);
        if (in.front() != L':')
            return false;
        in = in.ShiftFront();

        if (IsDigit(id)) {
            if (not Atoi(&SourceIndex, id, 10))
                return false;
        }
        else
            Filename = id;

        const FWStringView line = EatDigits(in);
        if (line.empty())
            return false;
        in = in.CutStartingAt(line.size());
        if (not Atoi(&Line, line, 10))
            return false;

        Description = Strip(in);
        return true;
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FVulkanSpirvCompiler::FCompilationContext
//----------------------------------------------------------------------------
struct FVulkanSpirvCompiler::FCompilationContext {
    TPtrRef<FWStringBuilder> Log;
    TPtrRef<FGLSLangResult> Glslang;
    TPtrRef<FIncludeResolver> Resolver;
    TPtrRef<FShaderReflection> Reflection;
    TPtrRef<glslang::TIntermediate> Intermediate;
    EShaderStages CurrentStage{ Default };
    spv_target_env SpirvTargetEnvironment{ SPV_ENV_MAX };
    bool TargetVulkan{ true };
};
//----------------------------------------------------------------------------
// FVulkanSpirvCompiler::FIncludeResolver
//----------------------------------------------------------------------------
class FVulkanSpirvCompiler::FIncludeResolver final : public glslang::TShader::Includer {
public:
    struct FIncludeResultImpl final : public IncludeResult {
        const FRawStorage RawData;

        FIncludeResultImpl(FRawStorage&& rawData, const std::string& headerName, void* userData = nullptr) NOEXCEPT
        :   IncludeResult{ headerName, nullptr, 0, userData }
        ,   RawData(std::move(rawData)) {
            const auto source = Source();
            const_cast<const char*&>(headerData) = source.data();
            const_cast<size_t&>(headerLength) = source.size();
        }

        FStringView Source() const {
            return RawData.MakeConstView().Cast<const char>();
        }
    };

    using UIncludeResult = TUniquePtr<FIncludeResultImpl>;
    using FIncludeResultRef = TPtrRef<FIncludeResultImpl>;
    using FIncludeResults = VECTORINSITU(PipelineCompiler, UIncludeResult, 1);
    using FIncludedFiles = HASHMAP(PipelineCompiler, FFilename, FIncludeResultRef);

    explicit FIncludeResolver(const FDirectories& directories)
    :   _directories(directories)
    {}

    ~FIncludeResolver() override {}

    const FIncludeResults& Results() const { return _results; }
    const FIncludedFiles& IncludedFiles() const { return _includedFiles; }

public: // glslang::TShader::Includer
    IncludeResult* includeSystem(const char*, const char*, size_t) override {
        return nullptr;
    }

    IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override {
        UNUSED(includerName);
        UNUSED(inclusionDepth);

        FRawStorage headerData;
        FWString relativeName = ToWString(MakeCStringView(headerName));
        for (const FDirpath& path : _directories) {
            FFilename absolute{ path, relativeName.MakeView() };

            if (_includedFiles.Contains(absolute)) {
                headerData.CopyFrom("// skipped recursive header include\n"_view.Cast<const u8>());
                _results.emplace_back(MakeUnique<FIncludeResultImpl>(
                    std::move(headerData), headerName ));
                return _results.back().get();
            }

            if (VFS_ReadAll(&headerData, absolute, EAccessPolicy::Text)) {
                _results.emplace_back(MakeUnique<FIncludeResultImpl>(
                    std::move(headerData), headerName ));
                _includedFiles.insert_or_assign({ absolute, _results.back().get() });
                return _results.back().get();
            }
        }

        return nullptr;
    }

    void releaseInclude(IncludeResult*) override {}

private:
    FIncludeResults _results;
    FIncludedFiles _includedFiles;
    const FDirectories& _directories;
};
//----------------------------------------------------------------------------
// FGLSLangResult
//----------------------------------------------------------------------------
struct FVulkanSpirvCompiler::FGLSLangResult {
    glslang::TProgram Program;
    TUniquePtr<glslang::TShader> Shader;
};
//----------------------------------------------------------------------------
// SPIRV compiler
//----------------------------------------------------------------------------
FVulkanSpirvCompiler::FVulkanSpirvCompiler(const FDirectories& directories)
:   _directories(directories) {

    glslang::InitializeProcess();

    GenerateDefaultResources_(&_builtInResources);
}
//----------------------------------------------------------------------------
FVulkanSpirvCompiler::~FVulkanSpirvCompiler() {
    glslang::FinalizeProcess();
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::SetCompilationFlags(EVulkanShaderCompilationFlags flags) {
    _compilationFlags = flags;
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanSpirvCompiler::SetShaderDebugFlags(EShaderLangFormat flags) {
    _debugFlags = flags;
}
#endif
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::SetShaderClockFeatures(bool shaderSubgroupClock, bool shaderDeviceClock) {
    _features.EnableShaderSubgroupClock = shaderSubgroupClock;
    _features.EnableShaderDeviceClock = shaderDeviceClock;
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::SetShaderFeatures(bool vertexPipelineStoresAndAtomics, bool fragmentStoresAndAtomic) {
    _features.EnableVertexPipelineStoresAndAtomics = vertexPipelineStoresAndAtomics;
    _features.EnableFragmentStoresAndAtomics = fragmentStoresAndAtomic;
}
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::CheckShaderFeatures_(EShaderType shaderType) const {
    switch (shaderType) {
    case EShaderType::Vertex:
    case EShaderType::TessControl:
    case EShaderType::TessEvaluation:
    case EShaderType::Geometry:
        return _features.EnableVertexPipelineStoresAndAtomics;
    case EShaderType::Fragment:
        return _features.EnableFragmentStoresAndAtomics;
    case EShaderType::Compute:
        return true;
    case EShaderType::MeshTask:
    case EShaderType::Mesh:
        return true;
    case EShaderType::RayGen:
    case EShaderType::RayAnyHit:
    case EShaderType::RayClosestHit:
    case EShaderType::RayMiss:
    case EShaderType::RayIntersection:
    case EShaderType::RayCallable:
        return true;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
// Compilation
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::Compile(
    FPipelineDesc::FShader* outShader,
    FShaderReflection* outReflection,
    FWStringBuilder* outLog,
    EShaderType shaderType,
    EShaderLangFormat srcShaderFormat,
    EShaderLangFormat dstShaderFormat,
    FConstChar entry,
    FConstChar source
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) const {
    outLog->clear();
    LOG_CHECK(PipelineCompiler, Meta::EnumAnd(dstShaderFormat, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::SPIRV);

    dstShaderFormat -= EShaderLangFormat::_DebugModeMask;

    FCompilationContext compilationContext;
    compilationContext.Log = outLog;
    compilationContext.CurrentStage = EShaderStages_FromShader(shaderType);
    compilationContext.Reflection = outReflection;

    FShaderDataFingerprint sourceFingerprint = Fingerprint128(source.MakeView(), GVulkanSpirvFingerprint);

    // compiler shader without debug info
    {
        FGLSLangResult glslang;
        FIncludeResolver resolver{ _directories };

        compilationContext.Glslang = glslang;
        compilationContext.Resolver = resolver;

        LOG_CHECK(PipelineCompiler, ParseGLSL_(
            compilationContext,
            shaderType, srcShaderFormat, dstShaderFormat,
            entry, { source.c_str() }, resolver ));

        for (const auto& file : resolver.Results())
            sourceFingerprint = Fingerprint128(file->Source(), sourceFingerprint);

        const FShaderDataFingerprint shaderFingerprint = Fingerprint128(MakeRawView(
            MakeTuple(
                entry.MakeView(),
                srcShaderFormat, dstShaderFormat,
                compilationContext.CurrentStage, _compilationFlags, _builtInResources)),
            sourceFingerprint );

        // #TODO: caching with shaderFingerprint?

        FRawData spirv;
        LOG_CHECK(PipelineCompiler, CompileSPIRV_(&spirv, compilationContext));
        LOG_CHECK(PipelineCompiler, BuildReflection_(compilationContext));

        if (_compilationFlags & EVulkanShaderCompilationFlags::ParseAnnotations) {
            LOG_CHECK(PipelineCompiler, ParseAnnotations_(compilationContext, source.MakeView()));

            for (const auto& file : resolver.Results())
                 LOG_CHECK(PipelineCompiler, ParseAnnotations_(compilationContext, file->Source()));
        }

        outShader->Specializations = outReflection->Specializations;
        outShader->AddShader(dstShaderFormat, entry, std::move(spirv), shaderFingerprint ARGS_IF_RHIDEBUG(debugName));
    }

#if USE_PPE_RHIDEBUG
    const EShaderLangFormat debugModes = (Meta::EnumAnd(srcShaderFormat, EShaderLangFormat::_DebugModeMask) + _debugFlags);

    if (debugModes != EShaderLangFormat::Unknown) {
        LOG_CHECK(PipelineCompiler, CheckShaderFeatures_(shaderType));

        for (auto allModes = MakeEnumBitMask(debugModes); allModes; ) {
            const auto mode = static_cast<EShaderLangFormat>(1u << allModes.PopFront_AssumeNotEmpty());
            Assert(debugModes & mode);

            FGLSLangResult glslang;
            FIncludeResolver resolver{ _directories };

            compilationContext.Glslang = glslang;
            compilationContext.Resolver = resolver;

            LOG_CHECK(PipelineCompiler, ParseGLSL_(
                compilationContext,
                shaderType, srcShaderFormat, dstShaderFormat,
                entry, { source.c_str() },
                resolver ));

            PVulkanSharedDebugUtils debugUtils = NEW_REF(PipelineCompiler, FVulkanSharedDebugUtils, std::in_place_type_t<ShaderTrace>{});

            ShaderTrace& trace = std::get<ShaderTrace>(*debugUtils);
            EShLanguage stage = glslang.Shader->getStage();
            glslang::TIntermediate& interm = *glslang.Program.getIntermediate(stage);

            trace.SetSource(source.c_str(), source.length());

            for (const auto& it : resolver.IncludedFiles())
                trace.IncludeSource(it.second->headerName.data(), it.second->Source().data(), it.second->Source().size());

            switch (mode) {
            case EShaderLangFormat::EnableDebugTrace:
                LOG_CHECK(PipelineCompiler, trace.InsertTraceRecording(interm, DebugDescriptorSet));
                break;
            case EShaderLangFormat::EnableProfiling:
                LOG_CHECK(PipelineCompiler, trace.InsertFunctionProfiler(interm, DebugDescriptorSet, _features.EnableShaderSubgroupClock, _features.EnableShaderDeviceClock));
                break;
            case EShaderLangFormat::EnableTimeMap:
                LOG_CHECK(PipelineCompiler, trace.InsertShaderClockHeatmap(interm, DebugDescriptorSet));
                break;

            default:
                LOG(PipelineCompiler, Error, L"unsupported shader debug mode: 0x{0:#8X}", Meta::EnumOrd(mode));
                break;
            }

            const FShaderDataFingerprint debugFingerprint = Fingerprint128(MakeRawView(
                MakeTuple(
                    mode,
                    entry.MakeView(),
                    srcShaderFormat, dstShaderFormat,
                    compilationContext.CurrentStage, _compilationFlags, _builtInResources)),
                sourceFingerprint );

            // #TODO: caching with debugFingerprint?

            FRawData debugSpirv;
            LOG_CHECK(PipelineCompiler, CompileSPIRV_(&debugSpirv, compilationContext));

            PShaderBinaryData debugShader{
                NEW_REF(PipelineCompiler, FVulkanDebuggableShaderSPIRV,
                    entry.MakeView(), std::move(debugSpirv), debugFingerprint ARGS_IF_RHIDEBUG(debugName.MakeView()) ) };

            outShader->AddShader(dstShaderFormat | mode, FShaderDataVariant{ std::move(debugShader) });
        }
    }
#endif

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::ParseGLSL_(
    FCompilationContext& ctx,
    EShaderType shaderType,
    EShaderLangFormat srcShaderFormat,
    EShaderLangFormat dstShaderFormat,
    FConstChar entry,
    TMemoryView<const FConstChar> source,
    FIncludeResolver& resolver ) const {
    Assert(ctx.Log);
    Assert(ctx.Glslang);

    using namespace ::glslang;

    EShClient client = EShClientOpenGL;
    EShTargetClientVersion clientVersion = EShTargetOpenGL_450;

    EShTargetLanguage target = EShTargetNone;
    EShTargetLanguageVersion targetVersion = EShTargetLanguageVersion(0);

    int version = 0;
    int shVersion = 460; // #TODO
    EProfile shProfile = ENoProfile;
    EShSource shSource = EShSourceCount;

    ctx.SpirvTargetEnvironment = SPV_ENV_UNIVERSAL_1_0;
    ctx.TargetVulkan = false;

    switch (Meta::EnumAnd(srcShaderFormat, EShaderLangFormat::_ApiMask)) {
    case EShaderLangFormat::OpenGL:
        shSource = EShSourceGlsl;
        version = EShaderLangFormat_Version(srcShaderFormat);
        shProfile = (version >= 330 ? ECoreProfile : ENoProfile);
        break;
    case EShaderLangFormat::OpenGLES:
        shSource = EShSourceGlsl;
        version = EShaderLangFormat_Version(srcShaderFormat);
        shProfile = EEsProfile;
        break;
    case EShaderLangFormat::DirectX:
        shSource = EShSourceHlsl;
        version = EShaderLangFormat_Version(srcShaderFormat);
        shProfile = ENoProfile; // #TODO: check
        break;
    case EShaderLangFormat::Vulkan:
        shSource = EShSourceGlsl;
        version = EShaderLangFormat_Version(srcShaderFormat);
        shProfile = ECoreProfile;
        break;
    default:
        LOG(PipelineCompiler, Error, L"unsupported source shader format");
        return false;
    }

    switch (Meta::EnumAnd(dstShaderFormat, EShaderLangFormat::_ApiMask)) {
    case EShaderLangFormat::Vulkan: {
        ctx.TargetVulkan = true;
        const u32 dstVersion = EShaderLangFormat_Version(dstShaderFormat);
        client = EShClientVulkan;
        target = EShTargetSpv;

        switch (dstVersion) {
        case 100:
            clientVersion = EShTargetVulkan_1_0;
            targetVersion = EShTargetSpv_1_0;
            ctx.SpirvTargetEnvironment = SPV_ENV_VULKAN_1_0;
            break;
        case 110:
            clientVersion = EShTargetVulkan_1_1;
            targetVersion = EShTargetSpv_1_3;
            ctx.SpirvTargetEnvironment = SPV_ENV_VULKAN_1_1;
            break;
        case 120:
            clientVersion = EShTargetVulkan_1_2;
            targetVersion = EShTargetSpv_1_4;
            ctx.SpirvTargetEnvironment = SPV_ENV_VULKAN_1_1_SPIRV_1_4;
            break;
        default:
            LOG(PipelineCompiler, Error, L"unsupported vulkan version: {0}", dstVersion);
            return false;
        }
        break;
    }
    case EShaderLangFormat::OpenGL: {
        if (Meta::EnumAnd(dstShaderFormat, EShaderLangFormat::_FormatMask) == EShaderLangFormat::SPIRV) {
            target = EShTargetSpv;
            targetVersion = EShTargetSpv_1_0;
            ctx.SpirvTargetEnvironment = SPV_ENV_OPENGL_4_5;
        }
        break;
    }
    default:
        LOG(PipelineCompiler, Error, L"unsupported source shader format");
        return false;
    }

    Assert(shSource < EShSourceCount);

    EShMessages messages = EShMsgDefault;
    EShLanguage stage = EShaderType_ToShLanguage(shaderType);
    auto& shader = ctx.Glslang->Shader;

    shader.reset<TShader>(stage);
    shader->setStrings(reinterpret_cast<const char* const*>(source.data()), checked_cast<int>(source.size()));
    shader->setEntryPoint(entry.c_str());
    shader->setEnvInput(shSource, stage, client, version);
    shader->setEnvClient(client, clientVersion);
    shader->setEnvTarget(target, targetVersion);

    if (not shader->parse(&_builtInResources, shVersion, shProfile, false, true, messages, resolver)) {
        *ctx.Log << MakeCStringView(shader->getInfoLog());
        OnCompilationFailed_(ctx, source);
        return false;
    }

    ctx.Glslang->Program.addShader(shader.get());

    if (not ctx.Glslang->Program.link(messages)) {
        *ctx.Log << MakeCStringView(shader->getInfoLog());
        OnCompilationFailed_(ctx, source);
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
#if defined(INCLUDE_SPIRV_TOOLS_OPTIMIZER_HPP_)
NODISCARD static bool OptimizeSPIRV_(
    FWStringBuilder& log,
    std::vector<unsigned>& spirv,
    spv_target_env targetEnv ) {

    spvtools::Optimizer optimizer{ targetEnv };
    optimizer.RegisterLegalizationPasses();
    optimizer.RegisterSizePasses();
    optimizer.RegisterPerformancePasses();

    optimizer.RegisterPass(spvtools::CreateCompactIdsPass());
    optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
    optimizer.RegisterPass(spvtools::CreateLocalAccessChainConvertPass());
    optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
    optimizer.RegisterPass(spvtools::CreateRemoveDuplicatesPass());
    optimizer.RegisterPass(spvtools::CreateCFGCleanupPass());

    optimizer.SetMessageConsumer(
        [&log](spv_message_level_t level, const char* source, const spv_position_t& position, const char* message) {
            switch (level) {
            case SPV_MSG_FATAL:
                log << L"fatal: ";
                break;
            case SPV_MSG_INTERNAL_ERROR:
                log << L"internal error: ";
                break;
            case SPV_MSG_ERROR:
                log << L"error: ";
                break;
            case SPV_MSG_WARNING:
                log << L"warning: ";
                break;
            case SPV_MSG_INFO:
                log << L"info: ";
                break;
            case SPV_MSG_DEBUG:
                log << L"debug: ";
                break;
            }

            if (source)
                log << MakeCStringView(source) << L':';

            log << position.line << L':' << position.column << L':' << position.index << L':';

            if (message)
                log << L' ' << MakeCStringView(message);
        });

    spvtools::OptimizerOptions options{};
    options.set_run_validator(false);

    optimizer.Run(spirv.data(), spirv.size(), &spirv, options);

    return true;
}
#endif
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::CompileSPIRV_(FRawData* outSPIRV, const FCompilationContext& ctx) const {
    Assert(outSPIRV);
    Assert(ctx.Log);
    Assert(ctx.Glslang);

    using namespace ::glslang;

    const TIntermediate* const intermediate = ctx.Glslang->Program.getIntermediate(
        ctx.Glslang->Shader->getStage() );
    LOG_CHECK(PipelineCompiler, !!intermediate);

    SpvOptions spvOptions{};
    spvOptions.generateDebugInfo = (_compilationFlags & EVulkanShaderCompilationFlags::GenerateDebug);
    spvOptions.disableOptimizer = not (_compilationFlags & EVulkanShaderCompilationFlags::Optimize);
    spvOptions.optimizeSize = (_compilationFlags & EVulkanShaderCompilationFlags::OptimizeSize);

    spv::SpvBuildLogger logger;
    std::vector<unsigned> spirvTmp;
    GlslangToSpv(*intermediate, spirvTmp, &logger, &spvOptions);

    const auto allMessages = logger.getAllMessages();
    *ctx.Log << FStringView{ allMessages.c_str(), allMessages.size() };

#ifdef INCLUDE_SPIRV_TOOLS_OPTIMIZER_HPP_
    if (_compilationFlags & EVulkanShaderCompilationFlags::StrongOptimization)
        LOG_CHECK(PipelineCompiler, OptimizeSPIRV_(*ctx.Log, spirvTmp, ctx.SpirvTargetEnvironment));
#endif

    *outSPIRV = FRawData{ MakeView(spirvTmp).Cast<const u8>() };
    return true;
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::OnCompilationFailed_(
    const FCompilationContext& ctx,
    TMemoryView<const FConstChar> source ) const {
    // glslang errors format:
    // pattern: <error/warning>: <number>:<line>: <description>
    // pattern: <error/warning>: <file>:<line>: <description>

    FWStringView line;
    FWStringView log{ ctx.Log->Written() };

    u32 lineNumber{ 0 };
    u32 prevErrorLine{ 0 };

    FWStringBuilder parsedLog;
    parsedLog.reserve(ctx.Log->capacity());

    while (Split(log, L'\n', line)) {
        ++lineNumber;
        EatSpaces(line);
        if (line.empty())
            continue;

        bool parsed = false;

        FGLSLangError error;
        if (error.Parse(line)) {
            if (error.Line == prevErrorLine) {
                parsedLog << line << Eol;
                continue;
            }

            prevErrorLine = error.Line;

            if (error.Filename.empty()) {
                FStringView sourceLine;
                FStringView in{ error.SourceIndex < source.size() ? source[error.SourceIndex].MakeView() : "" };

                if (SplitNth(in, L'\n', sourceLine, error.Line - 1)) {
                    parsed = true;
                    parsedLog << L"in source (" << error.SourceIndex << L':' << error.Line << L"): " << Strip(sourceLine) << Eol;
                }
            }
        }

        if (not parsed)
            parsedLog << ARG0_IF_ASSERT(L"<unknown> " <<) line << Eol;
    }

    size_t len;
    FAllocatorBlock blk = parsedLog.StealDataUnsafe(&len);
    Verify( ctx.Log->AcquireDataUnsafe(blk, len) );
}
//----------------------------------------------------------------------------
// Reflection
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::BuildReflection_(FCompilationContext& ctx) const {
    Assert(ctx.Log);
    Assert(ctx.Glslang);
    Assert(ctx.Reflection);
    Assert(not ctx.Intermediate);

    ctx.Intermediate = ctx.Glslang->Program.getIntermediate(
        ctx.Glslang->Shader->getStage() );
    LOG_CHECK(PipelineCompiler, !!ctx.Intermediate);

    // deserialize shader
    TIntermNode* const root = ctx.Intermediate->getTreeRoot();
    Assert(root);

    LOG_CHECK(PipelineCompiler, ProcessExternalObjects_(ctx, *root));
    LOG_CHECK(PipelineCompiler, ProcessShaderInfos_(ctx));

    ctx.Intermediate.reset();
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::ParseAnnotations_(const FCompilationContext& ctx, FStringView source) const {
    Assert(ctx.Log);
    Assert(ctx.Glslang);
    Assert(ctx.Reflection);

    u32 lineNumber{ 0 };
    bool commentSingleLine{ false };
    bool commentMultiLine{ false };
    EShaderAnnotation annotations{ Default };

    const auto parseSet = [&]() -> bool {
        FStringView it{ source };
        EatSpaces(it);

        const FStringView id = EatDigits(it);
        if (id.empty() || EatSpaces(it).empty())
            return false;

        const FStringView name = EatIdentifier(it);
        if (name.empty())
            return false;

        u32 index{ INDEX_NONE };
        if (not Atoi(&index, id, 10))
            return false;

        const TPtrRef<FPipelineDesc::FDescriptorSet> ds =
            ctx.Reflection->Layout.DescriptorSets.MakeView().Any(
                [index](const FPipelineDesc::FDescriptorSet& ds) NOEXCEPT -> bool {
                    return (ds.BindingIndex == index);
                });
        if (not ds)
            return false;

        source = it;
        ds->Id = FDescriptorSetID{ name };
        return true;
    };

    const auto parseUniform = [&]() -> bool {
        // ex:
        //  buffer <SSBO> { ...
        //  uniform <UBO> { ...
        //  uniform image* <name> ...

        FStringView it{ source };

        while (not it.empty()) {
            EatSpaces(it);

            const FStringView word = EatIdentifier(it);
            if (word.empty()) {
                if (Equals(it.Eat(1), "}"))
                    return false;
                continue;
            }

            bool isBuffer = false;
            bool isUniform = false;
            bool isUniformImage = false;
            bool isUniformSampler = false;

            if (Equals(word, "buffer")) {
                isBuffer = true;
            }
            else
            if (Equals(word, "uniform")) {
                isUniform = true;
                EatSpaces(it);
                isUniformImage = it.Eat("image"); // optional
                UNUSED(isUniformImage);
                isUniformSampler = it.StartsWith("sampler"); // optional
                if (isUniformSampler) {
                    EatIdentifier(it);
                }
            }

            EatSpaces(it);
            const FStringView name = EatIdentifier(it);
            if (name.empty())
                continue;

            EatSpaces(it);
            if (not it.Eat("{"))
                continue;

            if ((isBuffer || isUniform)) {
                const FUniformID id{ name };

                bool found = true;
                for (FPipelineDesc::FDescriptorSet& ds : ctx.Reflection->Layout.DescriptorSets) {
                    const auto jt = ds.Uniforms->find(id);
                    if (ds.Uniforms->end() == jt)
                        continue;

                    found = Meta::Visit(jt->second.Data,
                        [&](FPipelineDesc::FImage& image) {
                            if (annotations & EShaderAnnotation::DynamicOffset)
                                *ctx.Log << L"@dynamic-offset is only supported on buffers, but found on image <" << name << L">" << Eol;
                            if (annotations & EShaderAnnotation::WriteDiscard)
                                image.State |= EResourceState::InvalidateBefore;
                            return true;
                        },
                        [&](FPipelineDesc::FUniformBuffer& ubo) {
                            if (annotations & EShaderAnnotation::DynamicOffset)
                                ubo.State |= EResourceState::_BufferDynamicOffset;
                            if (annotations & EShaderAnnotation::WriteDiscard)
                                *ctx.Log << L"@write-discard is only supported on images or storage buffers, but found on uniform buffer <" << name << L">" << Eol;
                            return true;
                        },
                        [&](FPipelineDesc::FStorageBuffer& ssbo) {
                            if (annotations & EShaderAnnotation::DynamicOffset)
                                ssbo.State |= EResourceState::_BufferDynamicOffset;
                            if (annotations & EShaderAnnotation::WriteDiscard)
                                ssbo.State |= EResourceState::InvalidateBefore;
                            return true;
                        },
                        [&](FPipelineDesc::FTexture&) {
                            *ctx.Log << L"unsupported annotation found on texture <" << name << L">" << Eol;
                            return true;
                        },
                        [&](FPipelineDesc::FSampler&) {
                            *ctx.Log << L"unsupported annotation found on sampler <" << name << L">" << Eol;
                            return true;
                        },
                        [&](FPipelineDesc::FSubpassInput&) {
                            *ctx.Log << L"unsupported annotation found on subpass input <" << name << L">" << Eol;
                            return true;
                        },
                        [&](FPipelineDesc::FRayTracingScene&) {
                            *ctx.Log << L"unsupported annotation found on raytracing scene input <" << name << L">" << Eol;
                            return true;
                        },
                        [](std::monostate&) {
                            AssertNotReached();
                        });

                    break;
                }

                source = it;
                return true;
            }
        }

        return false;
    };

    while (not source.empty()) {
        const char c = source.front();
        const char n = (source.size() > 1 ? source[1] : 0);
        source.Eat(1);

        const bool newLine1 = (c == '\r') && (n == '\n'); // windows
        const bool newLine2 = (c == '\n') || (c == '\r'); // linux | mac
        if (newLine1 || newLine2) {
            if (newLine1)
                source.Eat(1);
            commentSingleLine = false;
            lineNumber++;

            if ((annotations ^ (EShaderAnnotation::DynamicOffset | EShaderAnnotation::WriteDiscard)) && not commentMultiLine) {
                LOG_CHECK(PipelineCompiler, parseUniform());
                annotations = Default;
            }
            continue;
        }

        if (commentMultiLine) {
            if ((c == '*') && (n == '/'))
                commentMultiLine = false;
        }
        else {
            if ((c == '/') && (n == '*'))
                commentMultiLine = true;
            if ((c == '/') && (n == '/'))
                commentSingleLine = true;
        }

        if (not ((commentSingleLine || commentMultiLine) && (c == '@')))
            continue;

        const FStringView id = EatIdentifier(source);

        if (Equals(id, "set")) {
            LOG_CHECK(PipelineCompiler, parseSet());
            annotations |= EShaderAnnotation::Set;
        }
        else
        if (Equals(id, "discard"))
            annotations |= EShaderAnnotation::WriteDiscard;
        else
        if (Equals(id, "dynamic-offset"))
            annotations |= EShaderAnnotation::DynamicOffset;

        source.Eat(",");
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::ProcessExternalObjects_(
    const FCompilationContext& ctx,
    TIntermNode& node ) const {
    using namespace glslang;

    if (TIntermAggregate* const aggr = node.getAsAggregate()) {
        switch (aggr->getOp()) {
            // continue deserializing
            case TOperator::EOpSequence:
                for (TIntermNode* interm : aggr->getSequence()) {
                    Assert(interm);
                    LOG_CHECK(PipelineCompiler, ProcessExternalObjects_(ctx, *interm));
                }
            break;
            // uniforms, buffers, ...
            case TOperator::EOpLinkerObjects:
                for (TIntermNode* interm : aggr->getSequence()) {
                    Assert(interm);
                    LOG_CHECK(PipelineCompiler, DeserializeExternalObjects_(ctx, *interm));
                }
            break;
        default: break;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
FBindingIndex FVulkanSpirvCompiler::ToBindingIndex_(const FCompilationContext& ctx, u32 index) {
    const FBindingIndex::index_t bindingIndex = (index != UMax
        ? checked_cast<FBindingIndex::index_t>(index)
        : UMax);
    if (ctx.TargetVulkan)
        return FBindingIndex{ UMax, bindingIndex };
    else
        return FBindingIndex{ bindingIndex, UMax };
}
//----------------------------------------------------------------------------
FPipelineDesc::FDescriptorSet& FVulkanSpirvCompiler::ToDescriptorSet_(const FCompilationContext& ctx, u32 index) {
    TPtrRef<FPipelineDesc::FDescriptorSet> ds = ctx.Reflection->Layout.DescriptorSets.MakeView().Any(
        [index](const FPipelineDesc::FDescriptorSet& ds) {
            return (ds.BindingIndex == index);
        });

    if (Unlikely(not ds)) {
        ds = ctx.Reflection->Layout.DescriptorSets.Push_Default();
        ds->BindingIndex = index;
        ds->Id = FDescriptorSetID{ ToString(index) };
        ds->Uniforms = NEW_REF(RHIPipeline, FPipelineDesc::FUniformMap);
    }

    return *ds;
}
//----------------------------------------------------------------------------
FStringView FVulkanSpirvCompiler::ExtractNodeName_(const TIntermNode* node) {
    LOG_CHECK(PipelineCompiler, node and node->getAsSymbolNode());

    const glslang::TString& str = node->getAsSymbolNode()->getName();
    const FStringView result{ str.c_str(), str.size() };
    return StartsWith(result, "anon@") ? Default : result;
}
//----------------------------------------------------------------------------
FUniformID FVulkanSpirvCompiler::ExtractBufferUniformID_(const glslang::TType& type) {
    const glslang::TString& name = type.getTypeName();
    return { FStringView{ name.c_str(), name.size() } };
}
//----------------------------------------------------------------------------
EImageSampler FVulkanSpirvCompiler::ExtractImageSampler_(const glslang::TType& type) {
    using namespace glslang;

    if (type.getBasicType() == TBasicType::EbtSampler and not type.isSubpass()) {
        EImageSampler resource = Zero;
        const TSampler& sampler = type.getSampler();

        if (sampler.isImage()) {

            switch (type.getQualifier().layoutFormat) {
            case TLayoutFormat::ElfRgba32f: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA32f); break;
            case TLayoutFormat::ElfRgba16f: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA16f); break;
            case TLayoutFormat::ElfR32f: resource = EImageSampler_FromPixelFormat(EPixelFormat::R32f); break;
            case TLayoutFormat::ElfRgba8: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA8_UNorm); break;
            case TLayoutFormat::ElfRgba8Snorm: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA8_SNorm); break;
            case TLayoutFormat::ElfRg32f: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG32f); break;
            case TLayoutFormat::ElfRg16f: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG16f); break;
            case TLayoutFormat::ElfR11fG11fB10f: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGB_11_11_10f); break;
            case TLayoutFormat::ElfR16f: resource = EImageSampler_FromPixelFormat(EPixelFormat::R16f); break;
            case TLayoutFormat::ElfRgba16: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA16_UNorm); break;
            case TLayoutFormat::ElfRgb10A2: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGB10_A2_UNorm); break;
            case TLayoutFormat::ElfRg16: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG16_UNorm); break;
            case TLayoutFormat::ElfRg8: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG8_UNorm); break;
            case TLayoutFormat::ElfR16: resource = EImageSampler_FromPixelFormat(EPixelFormat::R16_UNorm); break;
            case TLayoutFormat::ElfR8: resource = EImageSampler_FromPixelFormat(EPixelFormat::R8_UNorm); break;
            case TLayoutFormat::ElfRgba16Snorm: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA16_SNorm); break;
            case TLayoutFormat::ElfRg16Snorm: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG16_SNorm); break;
            case TLayoutFormat::ElfRg8Snorm: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG8_SNorm); break;
            case TLayoutFormat::ElfR16Snorm: resource = EImageSampler_FromPixelFormat(EPixelFormat::R16_SNorm); break;
            case TLayoutFormat::ElfR8Snorm: resource = EImageSampler_FromPixelFormat(EPixelFormat::R8_SNorm); break;
            case TLayoutFormat::ElfRgba32i: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA32i); break;
            case TLayoutFormat::ElfRgba16i: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA16i); break;
            case TLayoutFormat::ElfRgba8i: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA8i); break;
            case TLayoutFormat::ElfR32i: resource = EImageSampler_FromPixelFormat(EPixelFormat::R32i); break;
            case TLayoutFormat::ElfRg32i: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG32i); break;
            case TLayoutFormat::ElfRg16i: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG16i); break;
            case TLayoutFormat::ElfRg8i: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG8i); break;
            case TLayoutFormat::ElfR16i: resource = EImageSampler_FromPixelFormat(EPixelFormat::R16i); break;
            case TLayoutFormat::ElfR8i: resource = EImageSampler_FromPixelFormat(EPixelFormat::R8i); break;
            case TLayoutFormat::ElfRgba32ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA32u); break;
            case TLayoutFormat::ElfRgba16ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA16u); break;
            case TLayoutFormat::ElfRgba8ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGBA8_UNorm); break;
            case TLayoutFormat::ElfR32ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::R32u); break;
            case TLayoutFormat::ElfRg32ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG32u); break;
            case TLayoutFormat::ElfRg16ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG16u); break;
            case TLayoutFormat::ElfRgb10a2ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::RGB10_A2u); break;
            case TLayoutFormat::ElfRg8ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::RG8u); break;
            case TLayoutFormat::ElfR16ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::R16u); break;
            case TLayoutFormat::ElfR8ui: resource = EImageSampler_FromPixelFormat(EPixelFormat::R8u); break;

            case TLayoutFormat::ElfNone: break;

            case TLayoutFormat::ElfEsFloatGuard:
            case TLayoutFormat::ElfFloatGuard:
            case TLayoutFormat::ElfEsIntGuard:
            case TLayoutFormat::ElfIntGuard:
            case TLayoutFormat::ElfEsUintGuard:
            case TLayoutFormat::ElfCount:
            default: AssertNotImplemented();
            }
        }

        if (sampler.type == TBasicType::EbtFloat) {
            resource |= EImageSampler::_Float;
        }
        else
        if (sampler.type == TBasicType::EbtUint) {
            resource |= EImageSampler::_UInt;
        }
        else
        if (sampler.type == TBasicType::EbtInt) {
            resource |= EImageSampler::_Int;
        }
        else {
            LOG(PipelineCompiler, Error, L"unsupported image value type");
            return Zero;
        }

        switch (sampler.dim) {
        case TSamplerDim::Esd1D :
            if (sampler.isShadow() and sampler.isArrayed())
                return resource | EImageSampler::_1DArray | EImageSampler::_Shadow;
            if (sampler.isShadow())
                return resource | EImageSampler::_1D | EImageSampler::_Shadow;
            if (sampler.isArrayed())
                return resource | EImageSampler::_1DArray;
            return resource | EImageSampler::_1D;

        case TSamplerDim::Esd2D :
            if (sampler.isShadow() and sampler.isArrayed() )
                return resource | EImageSampler::_2DArray | EImageSampler::_Shadow;
            if (sampler.isShadow())
                return resource | EImageSampler::_2D | EImageSampler::_Shadow;
            if (sampler.isMultiSample() and sampler.isArrayed() )
                return resource | EImageSampler::_2DMSArray;
            if (sampler.isArrayed())
                return resource | EImageSampler::_2DArray;
            if (sampler.isMultiSample())
                return resource | EImageSampler::_2DMS;
            return resource | EImageSampler::_2D;

        case TSamplerDim::Esd3D :
            return resource | EImageSampler::_3D;

        case TSamplerDim::EsdCube :
            if (sampler.isShadow())
                return resource | EImageSampler::_Cube | EImageSampler::_Shadow;
            if (sampler.isArrayed())
                return resource | EImageSampler::_CubeArray;
            return resource | EImageSampler::_Cube;

        case TSamplerDim::EsdBuffer:
        case TSamplerDim::EsdNone: // to shutup warnings
        case TSamplerDim::EsdRect:
        case TSamplerDim::EsdSubpass:
        case TSamplerDim::EsdNumDims:
        default: AssertNotImplemented();
        }
    }

    AssertNotReached();
}
//----------------------------------------------------------------------------
EResourceState FVulkanSpirvCompiler::ExtractShaderAccessType_(const glslang::TQualifier& q) {
    if (q.isWriteOnly())
        return EResourceState::ShaderWrite;

    if (q.isReadOnly())
        return EResourceState::ShaderRead;

    if (q.coherent or
        q.devicecoherent or
        q.queuefamilycoherent or
        q.workgroupcoherent or
        q.subgroupcoherent or
        q.volatil or
        q.restrict )
        return EResourceState::ShaderReadWrite;

    return EResourceState::ShaderReadWrite;
}
//----------------------------------------------------------------------------
EVertexFormat FVulkanSpirvCompiler::ExtractVertexFormat_(const glslang::TType& type) {
    using namespace glslang;

    EVertexFormat result = Zero;

    switch (type.getBasicType()) {
    case TBasicType::EbtFloat: result |= EVertexFormat::_Float; break;
    case TBasicType::EbtDouble: result |= EVertexFormat::_Double; break;
    case TBasicType::EbtFloat16: result |= EVertexFormat::_Half; break;
    case TBasicType::EbtInt8: result |= EVertexFormat::_Byte; break;
    case TBasicType::EbtUint8: result |= EVertexFormat::_UByte; break;
    case TBasicType::EbtInt16: result |= EVertexFormat::_Short; break;
    case TBasicType::EbtUint16: result |= EVertexFormat::_UShort; break;
    case TBasicType::EbtInt: result |= EVertexFormat::_Int; break;
    case TBasicType::EbtUint: result |= EVertexFormat::_UInt; break;
    case TBasicType::EbtInt64: result |= EVertexFormat::_Long; break;
    case TBasicType::EbtUint64: result |= EVertexFormat::_ULong; break;
    //case TBasicType::EbtBool: result |= EVertexType::_Bool; break;
    default: AssertNotImplemented();
    }

    if (type.isScalarOrVec1())
        return result | EVertexFormat::_Vec1;

    if (type.isVector()) {
        switch (type.getVectorSize()) {
        case 1: return result | EVertexFormat::_Vec1;
        case 2: return result | EVertexFormat::_Vec2;
        case 3: return result | EVertexFormat::_Vec3;
        case 4: return result | EVertexFormat::_Vec4;
        default: AssertNotImplemented();
        }
    }

    if (type.isMatrix()) {
        LOG(PipelineCompiler, Error, L"missing support for matrices!!!");
        AssertNotImplemented(); // #TODO: add support for matrices
    }

    AssertNotReached();
}
//----------------------------------------------------------------------------
EFragmentOutput FVulkanSpirvCompiler::ExtractFragmentOutput_(const glslang::TType& type) {
    using namespace glslang;

    LOG_CHECK(PipelineCompiler, type.getVectorSize() == 4);

    switch (type.getBasicType()) {
    case TBasicType::EbtFloat: return EFragmentOutput::Float4;
    case TBasicType::EbtInt: return EFragmentOutput::Int4;
    case TBasicType::EbtUint: return EFragmentOutput::UInt4;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
// based on TParseContext::fixBlockUniformOffsets:
bool FVulkanSpirvCompiler::CalculateStructSize_(
    u32* outStaticSize, u32* outArrayStride, u32* outMinOffset,
    const FCompilationContext& ctx,
    const glslang::TType& bufferType ) const {
    Assert(outStaticSize);
    Assert(outArrayStride);
    Assert(outMinOffset);

    using namespace glslang;

    *outStaticSize = *outArrayStride = 0;
    *outMinOffset = ~0u;

    LOG_CHECK(PipelineCompiler, bufferType.isStruct());
    LOG_CHECK(PipelineCompiler, (bufferType.getQualifier().isUniformOrBuffer() or
                    bufferType.getQualifier().layoutPushConstant ));
    LOG_CHECK(PipelineCompiler, (bufferType.getQualifier().layoutPacking == ElpStd140 or
                    bufferType.getQualifier().layoutPacking == ElpStd430 ));

    int memberSize{ 0 };
    int offset{ 0 };
    const TTypeList& structFields{ *bufferType.getStruct() };

    forrange(member, 0, structFields.size()) {
        const TType& memberType = *structFields[member].type;
        const TQualifier& memberQualifier = memberType.getQualifier();
        TLayoutMatrix subLayoutMatrix = memberQualifier.layoutMatrix;

        int dummyStride;
        int memberAlignment = ctx.Intermediate->getBaseAlignment(
            memberType, memberSize, dummyStride,
            bufferType.getQualifier().layoutPacking,
            subLayoutMatrix != ElmNone
                ? subLayoutMatrix == ElmRowMajor
                : bufferType.getQualifier().layoutMatrix == ElmRowMajor );

        if (memberQualifier.hasOffset()) {
            Assert(Meta::IsPow2OrZero(memberQualifier.layoutOffset));
            Assert(Meta::IsPow2(memberAlignment));

            if (ctx.Intermediate->getSpv().spv == 0) {
                Assert(memberQualifier.layoutOffset >= offset);
                offset = Max(offset, memberQualifier.layoutOffset);
            }
            else {
                offset = memberQualifier.layoutOffset;
            }
        }

        if (memberQualifier.hasAlign())
            memberAlignment = Max(memberAlignment, memberQualifier.layoutAlign);

        glslang::RoundToPow2(offset, memberAlignment);

        *outMinOffset = Min(*outMinOffset, checked_cast<u32>(offset));
        offset += memberSize;

        if (member + 1 == structFields.size() and memberType.isUnsizedArray()) {
            Assert(0 == memberSize);
            *outArrayStride = checked_cast<u32>(dummyStride);
        }
    }

    *outStaticSize = checked_cast<u32>(offset);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::DeserializeExternalObjects_(const FCompilationContext& ctx, TIntermNode& node) const {
    using namespace glslang;

    TIntermTyped* const tnode = node.getAsTyped();
    const TType& type = tnode->getType();
    const TQualifier& qualifier = tnode->getQualifier();

    // skip builtin
    if (type.isBuiltIn())
        return true;

    // shared variables
    if (qualifier.storage == EvqShared)
        return true;

    // shader input
    if (qualifier.storage == EvqVaryingIn) {
        if (ctx.CurrentStage != EShaderStages::Vertex)
            return true; // skip

        FVertexAttribute& attrib = *ctx.Reflection->Vertex.VertexAttributes.Push_Default();
        attrib.Id = ExtractVertexID_(&node);
        attrib.Index = (qualifier.hasLocation() ? checked_cast<u32>(qualifier.layoutLocation) : UMax);
        attrib.Format = ExtractVertexFormat_(type);
        return true;
    }

    // shader output
    if (qualifier.storage == EvqVaryingOut) {
        if (ctx.CurrentStage != EShaderStages::Fragment)
            return true; // skip

        FGraphicsPipelineDesc::FFragmentOutput& fragment = *ctx.Reflection->Fragment.FragmentOutputs.Push_Default();
        fragment.Index = (qualifier.hasLocation() ? checked_cast<u32>(qualifier.layoutLocation) : UMax);
        fragment.Type = ExtractFragmentOutput_(type);
        return true;
    }

    // specialization constant
    if (qualifier.storage == EvqConst and qualifier.layoutSpecConstantId != TQualifier::layoutSpecConstantIdEnd) {
        ctx.Reflection->Specializations.insert({
            ExtractSpecializationID_(&node),
            qualifier.layoutSpecConstantId });
        return true;
    }

    // global variable or global constant
    if (qualifier.storage == EvqGlobal or qualifier.storage == EvqConst)
        return true;

    FPipelineDesc::FDescriptorSet& descriptorSet = ToDescriptorSet_(
        ctx, qualifier.hasSet() ? checked_cast<u32>(qualifier.layoutSet) : 0);
    FPipelineDesc::FUniformMap& uniforms = *descriptorSet.Uniforms;

    const auto insertUniform = [&](FUniformID&& uniformId, auto&& resource) {
        FPipelineDesc::FVariantUniform un;
        un.Index = ToBindingIndex_(ctx, (qualifier.hasBinding()
            ? checked_cast<u32>(qualifier.layoutBinding)
            : UMax ));
        un.StageFlags = ctx.CurrentStage;
        un.Data = std::move(resource);
        un.ArraySize = GLSLangArraySize_(type);

        uniforms.insert({ uniformId, std::move(un) });
    };

    if (type.getBasicType() == TBasicType::EbtSampler) {
        // image
        if (type.getSampler().isImage()) {
            FPipelineDesc::FImage image{};
            image.Type = ExtractImageSampler_(type);
            image.State = ExtractShaderAccessType_(qualifier) | EResourceState_FromShaders(ctx.CurrentStage);

            insertUniform(ExtractUniformID_(&node), std::move(image));
            return true;
        }

        // subpass
        if (type.getSampler().isSubpass()) {
            FPipelineDesc::FSubpassInput subpass{};
            subpass.AttachmentIndex = (qualifier.hasAttachment() ? checked_cast<u32>(qualifier.layoutAttachment) : UMax);
            subpass.IsMultiSample = false; // #TODO
            subpass.State = EResourceState::InputAttachment | EResourceState_FromShaders(ctx.CurrentStage);

            insertUniform(ExtractUniformID_(&node), std::move(subpass));
            return true;
        }

        // sampler
        if (type.getSampler().isPureSampler()) {
            insertUniform(ExtractUniformID_(&node), FPipelineDesc::FSampler{});
            return true;
        }

        // texture
        {
            FPipelineDesc::FTexture texture{};
            texture.Type = ExtractImageSampler_(type);
            texture.State = EResourceState::ShaderSample | EResourceState_FromShaders(ctx.CurrentStage);

            insertUniform(ExtractUniformID_(&node), std::move(texture));
            return true;
        }
    }

    // push constants
    if (qualifier.layoutPushConstant) {
        u32 staticSize{}, arrayStride{}, minOffset{};
        LOG_CHECK(PipelineCompiler, CalculateStructSize_(&staticSize, &arrayStride, &minOffset, ctx, type));

        Assert(staticSize >= minOffset);
        staticSize -= minOffset;
        const TString& name = type.getTypeName();

        FPushConstantID id{{ name.c_str(), name.size() }};
        FPipelineDesc::FPushConstant pushConstant{ id, ctx.CurrentStage, minOffset, staticSize };

        ctx.Reflection->Layout.PushConstants.insert({ std::move(id), std::move(pushConstant) });
        return true;
    }

    // uniform buffer or storage buffer
    if (type.getBasicType() == TBasicType::EbtBlock and (
        qualifier.storage == TStorageQualifier::EvqUniform or
        qualifier.storage == TStorageQualifier::EvqBuffer )) {
        LOG_CHECK(PipelineCompiler, type.isStruct());

        if (qualifier.layoutShaderRecord)
            return true;

        // uniform block
        if (qualifier.storage == TStorageQualifier::EvqUniform) {
            FPipelineDesc::FUniformBuffer ubuf{};
            ubuf.State = EResourceState::UniformRead | EResourceState_FromShaders(ctx.CurrentStage);

            u32 stride{}, offset{};
            LOG_CHECK(PipelineCompiler, CalculateStructSize_(&ubuf.Size, &stride, &offset, ctx, type));
            LOG_CHECK(PipelineCompiler, 0 == offset);

            insertUniform(ExtractBufferUniformID_(type), std::move(ubuf));
            return true;
        }

        // storage block
        if (qualifier.storage == TStorageQualifier::EvqBuffer) {
            FPipelineDesc::FStorageBuffer sbuf{};
            sbuf.State = ExtractShaderAccessType_(qualifier) | EResourceState_FromShaders(ctx.CurrentStage);

            u32 offset{};
            LOG_CHECK(PipelineCompiler, CalculateStructSize_(&sbuf.StaticSize, &sbuf.ArrayStride, &offset, ctx, type));
            LOG_CHECK(PipelineCompiler, 0 == offset);

            insertUniform(ExtractBufferUniformID_(type), std::move(sbuf));
            return true;
        }
    }

    // acceleration structure
    if (type.getBasicType() == TBasicType::EbtAccStruct) {
        FPipelineDesc::FRayTracingScene rtScene{};
        rtScene.State = EResourceState::_RayTracingShader | EResourceState::ShaderRead;

        insertUniform(ExtractUniformID_(&node), std::move(rtScene));
        return true;
    }

    // uniform
    if (qualifier.storage == TStorageQualifier::EvqUniform) {
        LOG(PipelineCompiler, Error, L"uniform is not supported for Vulkan!");
        return false;
    }

    // #TODO
    if (qualifier.storage == TStorageQualifier::EvqPayload or
        qualifier.storage == TStorageQualifier::EvqPayloadIn or
        qualifier.storage == TStorageQualifier::EvqHitAttr or
        qualifier.storage == TStorageQualifier::EvqCallableData or
        qualifier.storage == TStorageQualifier::EvqCallableDataIn ) {
        LOG(PipelineCompiler, Error, L"#TODO: support is still back for this type!");
        return false;
    }

    LOG(PipelineCompiler, Error, L"uniform external type!");
    return false;
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::MergeWithGeometryInputPrimitive_(
    FShaderReflection::FTopologyBits* inoutTopology,
    glslang::TLayoutGeometry geometryType ) const {
    Assert(inoutTopology);

    using namespace glslang;

    switch (geometryType) {
    case TLayoutGeometry::ElgPoints:
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::Point));
        return;
    case TLayoutGeometry::ElgLines:
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::LineList));
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::LineStrip));
        return;
    case TLayoutGeometry::ElgLinesAdjacency:
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::LineListAdjacency));
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::LineStripAdjacency));
        return;
    case TLayoutGeometry::ElgTriangles:
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::TriangleFan));
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::TriangleList));
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::TriangleStrip));
        return;
    case TLayoutGeometry::ElgTrianglesAdjacency:
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::TriangleListAdjacency));
        inoutTopology->SetTrue(Meta::EnumOrd(EPrimitiveTopology::TriangleStripAdjacency));
        return;
    case TLayoutGeometry::ElgLineStrip:
    case TLayoutGeometry::ElgTriangleStrip:
    case TLayoutGeometry::ElgQuads:
    case TLayoutGeometry::ElgIsolines:
    case TLayoutGeometry::ElgNone:
        LOG(PipelineCompiler, Error, L"invalid geometry input primitive type!");
        break;
    }
}
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::ProcessShaderInfos_(const FCompilationContext& ctx) const {
    Assert(ctx.Intermediate);

    using namespace glslang;

    switch (ctx.Intermediate->getStage()) {
    case EShLangVertex: break;
    case EShLangTessControl:
        ctx.Reflection->Vertex.SupportedTopology.SetTrue(Meta::EnumOrd(EPrimitiveTopology::Patch));
        ctx.Reflection->Tesselation.PatchControlPoints = checked_cast<u32>(ctx.Intermediate->getVertices());
        break;
    case EShLangTessEvaluation: break;
    case EShLangGeometry:
        MergeWithGeometryInputPrimitive_(&ctx.Reflection->Vertex.SupportedTopology, ctx.Intermediate->getInputPrimitive());
        break;
    case EShLangFragment:
        ctx.Reflection->Fragment.EarlyFragmentTests = (
            ctx.Intermediate->getEarlyFragmentTests() or
            not ctx.Intermediate->isDepthReplacing() );
        break;
    case EShLangCompute:
        ctx.Reflection->Compute.LocalGroupSize = {
            ctx.Intermediate->getLocalSize(0),
            ctx.Intermediate->getLocalSize(1),
            ctx.Intermediate->getLocalSize(2)
        };
        ctx.Reflection->Compute.LocalGroupSpecialization = {
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(0)),
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(1)),
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(2))
        };
        break;
    case EShLangRayGen/*NV*/: break;
    case EShLangIntersect/*NV*/: break;
    case EShLangAnyHit/*NV*/: break;
    case EShLangClosestHit/*NV*/: break;
    case EShLangMiss/*NV*/: break;
    case EShLangCallable/*NV*/: break;
    case EShLangTaskNV:
        ctx.Reflection->Mesh.TaskGroupSize = {
            ctx.Intermediate->getLocalSize(0),
            ctx.Intermediate->getLocalSize(1),
            ctx.Intermediate->getLocalSize(2)
        };
        ctx.Reflection->Mesh.TaskGroupSpecialization = {
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(0)),
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(1)),
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(2))
        };
        break;
    case EShLangMeshNV:
        ctx.Reflection->Mesh.MaxVertices = checked_cast<u32>(ctx.Intermediate->getVertices());
        ctx.Reflection->Mesh.MaxPrimitives = checked_cast<u32>(ctx.Intermediate->getPrimitives());
        ctx.Reflection->Mesh.MaxIndices = ctx.Reflection->Mesh.MaxPrimitives;

        switch (ctx.Intermediate->getOutputPrimitive()) {
        case ElgPoints:
            ctx.Reflection->Mesh.Topology = EPrimitiveTopology::Point;
            ctx.Reflection->Mesh.MaxIndices *= 1;
            break;
        case ElgLines:
            ctx.Reflection->Mesh.Topology = EPrimitiveTopology::LineList;
            ctx.Reflection->Mesh.MaxIndices *= 2;
            break;
        case ElgTriangles:
            ctx.Reflection->Mesh.Topology = EPrimitiveTopology::TriangleList;
            ctx.Reflection->Mesh.MaxIndices *= 3;
            break;
        default: AssertNotReached();
        }

        ctx.Reflection->Mesh.MeshGroupSize = {
            ctx.Intermediate->getLocalSize(0),
            ctx.Intermediate->getLocalSize(1),
            ctx.Intermediate->getLocalSize(2)
        };
        ctx.Reflection->Mesh.MeshGroupSpecialization = {
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(0)),
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(1)),
            static_cast<u32>(ctx.Intermediate->getLocalSizeSpecId(2))
        };
        break;
    case EShLangCount:
    default: AssertNotImplemented();
    }

    return true;
}
//----------------------------------------------------------------------------
// Resource limits
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::SetDefaultResourceLimits() {
    GenerateDefaultResources_(&_builtInResources);
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::GenerateDefaultResources_(TBuiltInResource* outResources) noexcept {
    Assert(outResources);

    *outResources = {};

    outResources->maxLights = 0;
    outResources->maxClipPlanes = 6;
    outResources->maxTextureUnits = 32;
    outResources->maxTextureCoords = 32;
    outResources->maxVertexAttribs = MaxVertexAttribs;
    outResources->maxVertexUniformComponents = 4096;
    outResources->maxVaryingFloats = 64;
    outResources->maxVertexTextureImageUnits = 32;
    outResources->maxCombinedTextureImageUnits = 80;
    outResources->maxTextureImageUnits = 32;
    outResources->maxFragmentUniformComponents = 4096;
    outResources->maxDrawBuffers = MaxColorBuffers;
    outResources->maxVertexUniformVectors = 128;
    outResources->maxVaryingVectors = 8;
    outResources->maxFragmentUniformVectors = 16;
    outResources->maxVertexOutputVectors = 16;
    outResources->maxFragmentInputVectors = 15;
    outResources->minProgramTexelOffset = -8;
    outResources->maxProgramTexelOffset = 7;
    outResources->maxClipDistances = 8;
    outResources->maxComputeWorkGroupCountX = 65535;
    outResources->maxComputeWorkGroupCountY = 65535;
    outResources->maxComputeWorkGroupCountZ = 65535;
    outResources->maxComputeWorkGroupSizeX = 1024;
    outResources->maxComputeWorkGroupSizeY = 1024;
    outResources->maxComputeWorkGroupSizeZ = 64;
    outResources->maxComputeUniformComponents = 1024;
    outResources->maxComputeTextureImageUnits = 16;
    outResources->maxComputeImageUniforms = 8;
    outResources->maxComputeAtomicCounters = 0;
    outResources->maxComputeAtomicCounterBuffers = 0;
    outResources->maxVaryingComponents = 60;
    outResources->maxVertexOutputComponents = 64;
    outResources->maxGeometryInputComponents = 64;
    outResources->maxGeometryOutputComponents = 128;
    outResources->maxFragmentInputComponents = 128;
    outResources->maxImageUnits = 8;
    outResources->maxCombinedImageUnitsAndFragmentOutputs = 8;
    outResources->maxImageSamples = 0;
    outResources->maxVertexImageUniforms = 0;
    outResources->maxTessControlImageUniforms = 0;
    outResources->maxTessEvaluationImageUniforms = 0;
    outResources->maxGeometryImageUniforms = 0;
    outResources->maxFragmentImageUniforms = 8;
    outResources->maxCombinedImageUniforms = 8;
    outResources->maxGeometryTextureImageUnits = 16;
    outResources->maxGeometryOutputVertices = 256;
    outResources->maxGeometryTotalOutputComponents = 1024;
    outResources->maxGeometryUniformComponents = 1024;
    outResources->maxGeometryVaryingComponents = 64;
    outResources->maxTessControlInputComponents = 128;
    outResources->maxTessControlOutputComponents = 128;
    outResources->maxTessControlTextureImageUnits = 16;
    outResources->maxTessControlUniformComponents = 1024;
    outResources->maxTessControlTotalOutputComponents = 4096;
    outResources->maxTessEvaluationInputComponents = 128;
    outResources->maxTessEvaluationOutputComponents = 128;
    outResources->maxTessEvaluationTextureImageUnits = 16;
    outResources->maxTessEvaluationUniformComponents = 1024;
    outResources->maxTessPatchComponents = 120;
    outResources->maxPatchVertices = 32;
    outResources->maxTessGenLevel = 64;
    outResources->maxViewports = MaxViewports;
    outResources->maxVertexAtomicCounters = 0;
    outResources->maxTessControlAtomicCounters = 0;
    outResources->maxTessEvaluationAtomicCounters = 0;
    outResources->maxGeometryAtomicCounters = 0;
    outResources->maxFragmentAtomicCounters = 0;
    outResources->maxCombinedAtomicCounters = 0;
    outResources->maxAtomicCounterBindings = 0;
    outResources->maxVertexAtomicCounterBuffers = 0;
    outResources->maxTessControlAtomicCounterBuffers = 0;
    outResources->maxTessEvaluationAtomicCounterBuffers = 0;
    outResources->maxGeometryAtomicCounterBuffers = 0;
    outResources->maxFragmentAtomicCounterBuffers = 0;
    outResources->maxCombinedAtomicCounterBuffers = 0;
    outResources->maxAtomicCounterBufferSize = 0;
    outResources->maxTransformFeedbackBuffers = 0;
    outResources->maxTransformFeedbackInterleavedComponents = 0;
    outResources->maxCullDistances = 8;
    outResources->maxCombinedClipAndCullDistances = 8;
    outResources->maxSamples = 4;
    outResources->maxMeshOutputVerticesNV = 256;
    outResources->maxMeshOutputPrimitivesNV = 512;
    outResources->maxMeshWorkGroupSizeX_NV = 32;
    outResources->maxMeshWorkGroupSizeY_NV = 1;
    outResources->maxMeshWorkGroupSizeZ_NV = 1;
    outResources->maxTaskWorkGroupSizeX_NV = 32;
    outResources->maxTaskWorkGroupSizeY_NV = 1;
    outResources->maxTaskWorkGroupSizeZ_NV = 1;
    outResources->maxMeshViewCountNV = 4;

    outResources->limits.nonInductiveForLoops = 1;
    outResources->limits.whileLoops = 1;
    outResources->limits.doWhileLoops = 1;
    outResources->limits.generalUniformIndexing = 1;
    outResources->limits.generalAttributeMatrixVectorIndexing = 1;
    outResources->limits.generalVaryingIndexing = 1;
    outResources->limits.generalSamplerIndexing = 1;
    outResources->limits.generalVariableIndexing = 1;
    outResources->limits.generalConstantMatrixVectorIndexing = 1;
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::SetCurrentResourceLimits(const FVulkanDeviceInfo& deviceInfo) {
    Assert(VK_NULL_HANDLE != deviceInfo.vkPhysicalDevice);

    GenerateDefaultResources_(&_builtInResources);

    VkPhysicalDeviceProperties properties{};

#ifdef VK_NV_mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeatures = {};
    VkPhysicalDeviceMeshShaderPropertiesNV meshShaderProperties = {};
#endif

    deviceInfo.API.instance_api_->vkGetPhysicalDeviceProperties(deviceInfo.vkPhysicalDevice, &properties);

    if (VK_VERSION_MAJOR(properties.apiVersion) >= 1 and
        VK_VERSION_MINOR(properties.apiVersion) >  0 ) {
        VkPhysicalDeviceFeatures2 features2{};
        VkPhysicalDeviceProperties2 properties2{};

        void** nextFeatures{ nullptr };
        void** nextProperties{ nullptr };

        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        nextProperties = &properties2.pNext;

        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        nextFeatures = &features2.pNext;

#if VK_NV_mesh_shader
        *nextFeatures = &meshShaderFeatures;
        nextFeatures = &meshShaderFeatures.pNext;
        meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;

        *nextProperties = &meshShaderProperties;
        nextProperties = &meshShaderProperties.pNext;
        meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV;
#endif

        deviceInfo.API.instance_api_->vkGetPhysicalDeviceFeatures2(deviceInfo.vkPhysicalDevice, &features2);
        deviceInfo.API.instance_api_->vkGetPhysicalDeviceProperties2(deviceInfo.vkPhysicalDevice, &properties2);
    }

    _builtInResources.maxVertexAttribs = Min( MaxVertexAttribs, properties.limits.maxVertexInputAttributes );
    _builtInResources.maxDrawBuffers = Min( MaxColorBuffers, properties.limits.maxColorAttachments );
    _builtInResources.minProgramTexelOffset = properties.limits.minTexelOffset;
    _builtInResources.maxProgramTexelOffset = properties.limits.maxTexelOffset;

    _builtInResources.maxComputeWorkGroupCountX = properties.limits.maxComputeWorkGroupCount[0];
    _builtInResources.maxComputeWorkGroupCountY = properties.limits.maxComputeWorkGroupCount[1];
    _builtInResources.maxComputeWorkGroupCountZ = properties.limits.maxComputeWorkGroupCount[2];
    _builtInResources.maxComputeWorkGroupSizeX = properties.limits.maxComputeWorkGroupSize[0];
    _builtInResources.maxComputeWorkGroupSizeY = properties.limits.maxComputeWorkGroupSize[1];
    _builtInResources.maxComputeWorkGroupSizeZ = properties.limits.maxComputeWorkGroupSize[2];

    _builtInResources.maxVertexOutputComponents = properties.limits.maxVertexOutputComponents;
    _builtInResources.maxGeometryInputComponents = properties.limits.maxGeometryInputComponents;
    _builtInResources.maxGeometryOutputComponents = properties.limits.maxGeometryOutputComponents;
    _builtInResources.maxFragmentInputComponents = properties.limits.maxFragmentInputComponents;

    _builtInResources.maxCombinedImageUnitsAndFragmentOutputs = properties.limits.maxFragmentCombinedOutputResources;
    _builtInResources.maxGeometryOutputVertices = properties.limits.maxGeometryOutputVertices;
    _builtInResources.maxGeometryTotalOutputComponents = properties.limits.maxGeometryTotalOutputComponents;

    _builtInResources.maxTessControlInputComponents = properties.limits.maxTessellationControlPerVertexInputComponents;
    _builtInResources.maxTessControlOutputComponents = properties.limits.maxTessellationControlPerVertexOutputComponents;
    _builtInResources.maxTessControlTotalOutputComponents = properties.limits.maxTessellationControlTotalOutputComponents;
    _builtInResources.maxTessEvaluationInputComponents = properties.limits.maxTessellationEvaluationInputComponents;
    _builtInResources.maxTessEvaluationOutputComponents = properties.limits.maxTessellationEvaluationOutputComponents;
    _builtInResources.maxTessPatchComponents = properties.limits.maxTessellationControlPerPatchOutputComponents;
    _builtInResources.maxPatchVertices = properties.limits.maxTessellationPatchSize;
    _builtInResources.maxTessGenLevel = properties.limits.maxTessellationGenerationLevel;

    _builtInResources.maxViewports = Min(MaxViewports, properties.limits.maxViewports);
    _builtInResources.maxClipDistances = properties.limits.maxClipDistances;
    _builtInResources.maxCullDistances = properties.limits.maxCullDistances;
    _builtInResources.maxCombinedClipAndCullDistances = properties.limits.maxCombinedClipAndCullDistances;

#if VK_NV_mesh_shader
    if (meshShaderFeatures.meshShader and meshShaderFeatures.taskShader)
    {
        _builtInResources.maxMeshOutputVerticesNV = meshShaderProperties.maxMeshOutputVertices;
        _builtInResources.maxMeshOutputPrimitivesNV = meshShaderProperties.maxMeshOutputPrimitives;
        _builtInResources.maxMeshWorkGroupSizeX_NV = meshShaderProperties.maxMeshWorkGroupSize[0];
        _builtInResources.maxMeshWorkGroupSizeY_NV = meshShaderProperties.maxMeshWorkGroupSize[1];
        _builtInResources.maxMeshWorkGroupSizeZ_NV = meshShaderProperties.maxMeshWorkGroupSize[2];
        _builtInResources.maxTaskWorkGroupSizeX_NV = meshShaderProperties.maxTaskWorkGroupSize[0];
        _builtInResources.maxTaskWorkGroupSizeY_NV = meshShaderProperties.maxTaskWorkGroupSize[1];
        _builtInResources.maxTaskWorkGroupSizeZ_NV = meshShaderProperties.maxTaskWorkGroupSize[2];
        _builtInResources.maxMeshViewCountNV = meshShaderProperties.maxMeshMultiviewViewCount;
    }
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
