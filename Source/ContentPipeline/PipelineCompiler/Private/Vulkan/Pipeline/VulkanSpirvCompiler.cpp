// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Pipeline/VulkanSpirvCompiler.h"

#include "Vulkan/Instance/VulkanInstance.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Pipeline/VulkanDebuggableShaderData.h"

#include "RHI/EnumHelpers.h"
#include "RHI/PipelineCompiler.h"

#include "Container/BitMask.h"
#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/FileSystemProperties.h"
#include "IO/Filename.h"
#include "IO/FormatHelpers.h"
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

#include "Memory/SharedBuffer.h"

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
template <typename... _Args>
NODISCARD FShaderDataFingerprint MakeShaderFingerprint_(const FShaderDataFingerprint& seed, _Args&&... args) {
    constexpr size_t capacityInBytes = (sizeof(_Args) + ... + 0);

    VECTORINSITU(PipelineCompiler, u8, capacityInBytes) blob;
    FOLD_EXPR(Append(blob, MakePodConstView(args)));

    return Fingerprint128(blob.MakeView(), seed);
}
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
    TStaticString<FileSystem::MaxPathLength> Filename;
    FStringView Description;
    u32 SourceIndex{ 0 };
    u32 Line{ 0 };
    bool IsError{ false };

    bool Parse(FStringView in) {
        CONSTEXPR FStringView c_error = "error"_view;
        CONSTEXPR FStringView c_warning = "warning"_view;

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

        if (in.front() != ':')
            return false;
        in = in.ShiftFront();

        in = Strip(in);
        if (not TryParseSite(in))
            Description = in;

        return true;
    }

    bool TryParseSite(FStringView in) {
        FStringView id = EatUntil(in, ':');
        if (in.empty() || in.front() != ':')
            return false;
        in = in.ShiftFront();

        if (IsDigit(id)) {
            if (not Atoi(&SourceIndex, id, 10))
                return false;
            id = FStringView{};
        }

        const FStringView line = EatDigits(in);
        if (line.empty())
            return false;
        in = in.CutStartingAt(line.size());
        if (not Atoi(&Line, line, 10))
            return false;

        Filename = id;
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
    TPtrRef<const FCompilationLogger> Log;
    TPtrRef<FGLSLangResult> Glslang;
    TPtrRef<FIncludeResolver> Resolver;
    TPtrRef<FShaderReflection> Reflection;
    TPtrRef<glslang::TIntermediate> Intermediate;
    EShaderStages CurrentStage{ Default };
    spv_target_env SpirvTargetEnvironment{ SPV_ENV_MAX };
    FConstChar SourceFile;
    int SourceLine{ 0 };
    bool TargetVulkan{ true };
};
//----------------------------------------------------------------------------
// FVulkanSpirvCompiler::FIncludeResolver
//----------------------------------------------------------------------------
class FVulkanSpirvCompiler::FIncludeResolver final : public glslang::TShader::Includer {
public:
    struct FIncludeResultImpl final : public IncludeResult {
        const FSharedBuffer RawData;

        FIncludeResultImpl(FSharedBuffer&& rawData, const std::string& headerName_, void* userData_ = nullptr) NOEXCEPT
        :   IncludeResult{ headerName_, nullptr, 0, userData_ }
        ,   RawData(std::move(rawData)) {
            const auto source = Source();
            const_cast<const char*&>(headerData) = source.data();
            const_cast<size_t&>(headerLength) = source.size();
        }

        FStringView Source() const {
            return RawData.MakeView().Cast<const char>();
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
        Unused(includerName);
        Unused(inclusionDepth);

        FWString relativeName = ToWString(MakeCStringView(headerName));
        for (const FDirpath& path : _directories) {
            FFilename absolute{ path, relativeName.MakeView() };

            if (_includedFiles.Contains(absolute)) {
                _results.emplace_back(MakeUnique<FIncludeResultImpl>(
                    FSharedBuffer::MakeView("// skipped recursive header include\n"_view.Cast<const u8>()),
                    headerName ));
                return _results.back().get();
            }

            if (FUniqueBuffer headerData = VFS_ReadAll(absolute, EAccessPolicy::Text)) {
                _results.emplace_back(MakeUnique<FIncludeResultImpl>(
                    headerData.MoveToShared(),
                    headerName ));
                _includedFiles.insert_or_assign({ absolute, _results.back().get() });
                return _results.back().get();
            }
        }

        return nullptr;
    }

    void releaseInclude(IncludeResult*) override {}

private:
    const FDirectories& _directories;
    FIncludeResults _results;
    FIncludedFiles _includedFiles;
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

    // precompute fingeprint for glslang toolchain
    const glslang::Version glslangVer = glslang::Version();
    _glslangFingerprint = MakeShaderFingerprint_(
        Fingerprint128(MakePodView(GVulkanSpirvFingerprint)),
        glslang::GetKhronosToolId(),
        glslangVer.major, glslangVer.minor,
        glslang::GetSpirvGeneratorVersion() );
}
//----------------------------------------------------------------------------
FVulkanSpirvCompiler::~FVulkanSpirvCompiler() {

    glslang::FinalizeProcess();
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::SetCompilationFlags(EShaderCompilationFlags flags) {
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
    const FCompilationLogger& log,
    EShaderType shaderType,
    EShaderLangFormat srcShaderFormat,
    EShaderLangFormat dstShaderFormat,
    FConstChar entry,
    const FSharedBuffer& content,
    FConstChar sourceFile ) const {
    PPE_LOG_CHECK(PipelineCompiler, Meta::EnumAnd(dstShaderFormat, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::SPIRV);

    dstShaderFormat -= EShaderLangFormat::_DebugModeMask;

    FCompilationContext compilationContext;
    compilationContext.Log = log;
    compilationContext.CurrentStage = EShaderStages_FromShader(shaderType);
    compilationContext.Reflection = outReflection;
    compilationContext.SourceFile = sourceFile;

    const FStringView sourceData = content.MakeView().Cast<const char>();

    const FShaderDataFingerprint sourceFingerprint = Fingerprint128(sourceData,
        Fingerprint128(entry.MakeView(), _glslangFingerprint));

    // compiler shader without debug info
    {
        FGLSLangResult glslang;
        FIncludeResolver resolver{ _directories };

        compilationContext.Glslang = glslang;
        compilationContext.Resolver = resolver;

        PPE_LOG_CHECK(PipelineCompiler, ParseGLSL_(
            compilationContext,
            shaderType, srcShaderFormat, dstShaderFormat,
            entry, content, sourceFile,
            resolver ));

        FShaderDataFingerprint shaderFingerprint = MakeShaderFingerprint_(
            sourceFingerprint,
            EDebugFlags::Unknown,
            srcShaderFormat,
            dstShaderFormat,
            compilationContext.CurrentStage,
            _compilationFlags,
            _builtInResources );

        for (const auto& file : resolver.Results())
            shaderFingerprint = Fingerprint128(file->Source(), shaderFingerprint);

        for (const auto& process : glslang.Shader->getIntermediate()->getProcesses())
            shaderFingerprint = Fingerprint128(process.data(), process.size() * sizeof(*process.data()), shaderFingerprint);

        // #TODO: caching with shaderFingerprint?

        FRawData spirv;
        PPE_LOG_CHECK(PipelineCompiler, CompileSPIRV_(&spirv, compilationContext));
        PPE_LOG_CHECK(PipelineCompiler, BuildReflection_(compilationContext));

        if (_compilationFlags & EShaderCompilationFlags::ParseAnnotations) {
            PPE_LOG_CHECK(PipelineCompiler, ParseAnnotations_(compilationContext, sourceData, sourceFile));

            for (const auto& file : resolver.Results())
                PPE_LOG_CHECK(PipelineCompiler, ParseAnnotations_(compilationContext, file->Source(), file->headerName.c_str()));
        }

        outShader->Specializations = outReflection->Specializations;
        outShader->AddShader(dstShaderFormat, entry, std::move(spirv), shaderFingerprint ARGS_IF_RHIDEBUG(sourceFile));
    }

#if USE_PPE_RHIDEBUG
    const EShaderLangFormat debugModes = (Meta::EnumAnd(srcShaderFormat, EShaderLangFormat::_DebugModeMask) + _debugFlags);

    if (debugModes != EShaderLangFormat::Unknown) {
        PPE_LOG_CHECK(PipelineCompiler, CheckShaderFeatures_(shaderType));

        for (auto allModes = MakeEnumBitMask(debugModes); allModes; ) {
            const auto mode = static_cast<EShaderLangFormat>(1u << allModes.PopFront_AssumeNotEmpty());
            Assert(debugModes & mode);

            FGLSLangResult glslang;
            FIncludeResolver resolver{ _directories };

            compilationContext.Glslang = glslang;
            compilationContext.Resolver = resolver;

            PPE_LOG_CHECK(PipelineCompiler, ParseGLSL_(
                compilationContext,
                shaderType, srcShaderFormat, dstShaderFormat,
                entry, content, sourceFile,
                resolver ));

            PVulkanSharedDebugUtils debugUtils = NEW_REF(PipelineCompiler, FVulkanSharedDebugUtils, std::in_place_type_t<ShaderTrace>{});

            const EShLanguage stage = glslang.Shader->getStage();
            ShaderTrace& trace = std::get<ShaderTrace>(*debugUtils);
            glslang::TIntermediate& interm = *glslang.Program.getIntermediate(stage);

            trace.SetSource(sourceData.data(), sourceData.size());

            for (const auto& it : resolver.IncludedFiles())
                trace.IncludeSource(it.second->headerName.data(), it.second->Source().data(), it.second->Source().size());

            switch (mode) {
            case EShaderLangFormat::EnableDebugTrace:
                PPE_LOG_CHECK(PipelineCompiler, trace.InsertTraceRecording(interm, DebugDescriptorSet));
                break;
            case EShaderLangFormat::EnableProfiling:
                PPE_LOG_CHECK(PipelineCompiler, trace.InsertFunctionProfiler(interm, DebugDescriptorSet, _features.EnableShaderSubgroupClock, _features.EnableShaderDeviceClock));
                break;
            case EShaderLangFormat::EnableTimeMap:
                PPE_LOG_CHECK(PipelineCompiler, trace.InsertShaderClockHeatmap(interm, DebugDescriptorSet));
                break;

            default:
                compilationContext.Log->Invoke(ELoggerVerbosity::Error, compilationContext.SourceFile, 0, "unsupported shader debug mode"_view, {
                    {"DebugMode", Meta::EnumOrd(mode)}
                });
                break;
            }

            FShaderDataFingerprint debugFingerprint = MakeShaderFingerprint_(
                sourceFingerprint,
                mode,
                srcShaderFormat,
                dstShaderFormat,
                compilationContext.CurrentStage,
                _compilationFlags,
                _builtInResources );

            for (const auto& file : resolver.Results())
                debugFingerprint = Fingerprint128(file->Source(), debugFingerprint);

            for (const auto& process : interm.getProcesses())
                debugFingerprint = Fingerprint128(process.data(), process.size() * sizeof(*process.data()), debugFingerprint);

            // #TODO: caching with debugFingerprint?

            FRawData debugSpirv;
            PPE_LOG_CHECK(PipelineCompiler, CompileSPIRV_(&debugSpirv, compilationContext));

            PShaderBinaryData debugShader{
                NEW_REF(PipelineCompiler, FVulkanDebuggableShaderSPIRV,
                    entry.MakeView(), std::move(debugSpirv), debugFingerprint, sourceFile.MakeView(), std::move(debugUtils))};

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
    const FSharedBuffer& content,
    FConstChar sourceFile,
    FIncludeResolver& resolver ) const {
    Assert(ctx.Log);
    Assert(ctx.Glslang);

    using namespace ::glslang;

    ctx.SpirvTargetEnvironment = SPV_ENV_UNIVERSAL_1_0;
    ctx.TargetVulkan = false;

    EShClient client = EShClientOpenGL;
    EShTargetClientVersion clientVersion = EShTargetOpenGL_450;

    EShTargetLanguage target = EShTargetNone;
    EShTargetLanguageVersion targetVersion = EShTargetLanguageVersion(0);

    const int shVersion = 460; // #TODO
    const int version = checked_cast<int>(EShaderLangFormat_Version(srcShaderFormat));

    EProfile shProfile = ENoProfile;
    EShSource shSource = EShSourceCount;

    switch (Meta::EnumAnd(srcShaderFormat, EShaderLangFormat::_ApiMask)) {
    case EShaderLangFormat::OpenGL:
        shSource = EShSourceGlsl;
        shProfile = (version >= 330 ? ECoreProfile : ENoProfile);
        break;
    case EShaderLangFormat::OpenGLES:
        shSource = EShSourceGlsl;
        shProfile = EEsProfile;
        break;
    case EShaderLangFormat::DirectX:
        shSource = EShSourceHlsl;
        shProfile = ENoProfile; // #TODO: check
        break;
    case EShaderLangFormat::Vulkan:
        shSource = EShSourceGlsl;
        shProfile = ECoreProfile;
        break;
    default:
        ctx.Log->Invoke(ELoggerVerbosity::Error, ctx.SourceFile, 0, "unsupported source shader format"_view, {
            {"Format", Meta::EnumOrd(Meta::EnumAnd(srcShaderFormat, EShaderLangFormat::_ApiMask))}
        });
        return false;
    }

    switch (Meta::EnumAnd(dstShaderFormat, EShaderLangFormat::_ApiMask)) {
    case EShaderLangFormat::Vulkan: {
        ctx.TargetVulkan = true;
        client = EShClientVulkan;
        target = EShTargetSpv;

        const u32 dstVersion = EShaderLangFormat_Version(dstShaderFormat);
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
        case 130:
            clientVersion = EShTargetVulkan_1_3;
            targetVersion = EShTargetSpv_1_6;
            ctx.SpirvTargetEnvironment = SPV_ENV_VULKAN_1_3;
            break;
        default:
            ctx.Log->Invoke(ELoggerVerbosity::Error, ctx.SourceFile, 0, "unsupported vulkan version"_view, {
                {"Version", dstVersion}
            });
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
        ctx.Log->Invoke(ELoggerVerbosity::Error, ctx.SourceFile, 0, "unsupported source shader format"_view, {
            {"Format", Meta::EnumOrd(Meta::EnumAnd(srcShaderFormat, EShaderLangFormat::_ApiMask))}
        });
        return false;
    }

    Assert(shSource < EShSourceCount);

    EShMessages messages = EShMsgDefault;
    EShLanguage stage = EShaderType_ToShLanguage(shaderType);
    auto& shader = ctx.Glslang->Shader;

    const FStringView stringData = content.MakeView().Cast<const char>();
    const char* strings[] = { stringData.data() };
    const int lengths[] = { checked_cast<int>(stringData.size()) };

    shader.create<TShader>(stage);
    shader->setStringsWithLengths(strings, lengths, 1);
    shader->setEntryPoint(entry.c_str());
    shader->setEnvInput(shSource, stage, client, version);
    shader->setEnvClient(client, clientVersion);
    shader->setEnvTarget(target, targetVersion);
    // shader->setPreamble(); // #TODO: use to pass optional macro definitions with a generated header

    if (not shader->parse(&_builtInResources, shVersion, shProfile, false, true, messages, resolver)) {
        OnCompilationFailed_(ctx, shader->getInfoLog(), { sourceFile });
        OnCompilationFailed_(ctx, shader->getInfoDebugLog(), { sourceFile });
        return false;
    }

    ctx.Glslang->Program.addShader(shader.get());

    if (not ctx.Glslang->Program.link(messages)) {
        OnCompilationFailed_(ctx, shader->getInfoLog(), { sourceFile });
        OnCompilationFailed_(ctx, shader->getInfoDebugLog(), { sourceFile });
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
                log << "fatal: ";
                break;
            case SPV_MSG_INTERNAL_ERROR:
                log << "internal error: ";
                break;
            case SPV_MSG_ERROR:
                log << "error: ";
                break;
            case SPV_MSG_WARNING:
                log << "warning: ";
                break;
            case SPV_MSG_INFO:
                log << "info: ";
                break;
            case SPV_MSG_DEBUG:
                log << "debug: ";
                break;
            }

            if (source)
                log << MakeCStringView(source) << ':';

            log << position.line << ':' << position.column << ':' << position.index << ':';

            if (message)
                log << ' ' << MakeCStringView(message);
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
    PPE_LOG_CHECK(PipelineCompiler, !!intermediate);

    SpvOptions spvOptions{};
    spvOptions.generateDebugInfo = (_compilationFlags & EShaderCompilationFlags::GenerateDebug);
    spvOptions.disableOptimizer = not (_compilationFlags & EShaderCompilationFlags::Optimize);
    spvOptions.optimizeSize = (_compilationFlags & EShaderCompilationFlags::OptimizeSize);
    spvOptions.validate = (_compilationFlags & EShaderCompilationFlags::Validate);

    spv::SpvBuildLogger logger;
    std::vector<unsigned> spirvTmp;
    GlslangToSpv(*intermediate, spirvTmp, &logger, &spvOptions);

    // Poor interface, have to parse log messages...
    // https://github.com/KhronosGroup/glslang/blob/main/SPIRV/Logger.cpp#L55
    const std::string allMessages = logger.getAllMessages();

    FStringView messageLine;
    FStringView allMessagesView{ allMessages.c_str(), allMessages.size() };
    while (Split(allMessagesView, '\n', messageLine)) {
        ELoggerVerbosity verbosity = ELoggerVerbosity::Error;

        if (messageLine.Eat("warning: "_view)) {
            verbosity = ELoggerVerbosity::Warning;
        }
        else
        if (messageLine.Eat("error: "_view)) {
            verbosity = ELoggerVerbosity::Error;
        }

        ctx.Log->Invoke(verbosity, ctx.SourceFile, 0, messageLine, {});
    }

#ifdef INCLUDE_SPIRV_TOOLS_OPTIMIZER_HPP_
    if (_compilationFlags & EShaderCompilationFlags::StrongOptimization)
        PPE_LOG_CHECK(PipelineCompiler, OptimizeSPIRV_(*ctx.Log, spirvTmp, ctx.SpirvTargetEnvironment));
#endif

    *outSPIRV = FRawData{ MakeView(spirvTmp).Cast<const u8>() };
    return true;
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::OnCompilationFailed_(
    const FCompilationContext& ctx,
    const FConstChar compilerLog,
    const TMemoryView<const FConstChar> sourceFiles) const {
    // glslang errors format:
    // pattern: <error/warning>: <number>:<line>: <description>
    // pattern: <error/warning>: <file>:<line>: <description>

    FStringView line;
    FStringView log{ compilerLog.MakeView() };

    while (Split(log, '\n', line)) {
        line = Strip(Chomp(line));
        EatSpaces(line);
        if (line.empty())
            continue;

        FGLSLangError error;
        if (error.Parse(line)) {
            FConstChar sourceFile = error.Filename;
            if (error.Filename.empty() and error.SourceIndex < sourceFiles.size())
                sourceFile = sourceFiles[error.SourceIndex];

            ctx.Log->Invoke(
                error.IsError ? ELoggerVerbosity::Error : ELoggerVerbosity::Warning,
                sourceFile, error.Line,
                error.Description, {});
        }
        else {
            ctx.Log->Invoke(ELoggerVerbosity::Info, sourceFiles[0], 0, line, {});
        }
    }
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
    PPE_LOG_CHECK(PipelineCompiler, !!ctx.Intermediate);

    // deserialize shader
    TIntermNode* const root = ctx.Intermediate->getTreeRoot();
    Assert(root);

    PPE_LOG_CHECK(PipelineCompiler, ProcessExternalObjects_(ctx, *root));
    PPE_LOG_CHECK(PipelineCompiler, ProcessShaderInfos_(ctx));

    ctx.Intermediate.reset();
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSpirvCompiler::ParseAnnotations_(const FCompilationContext& ctx, FStringView content, FConstChar sourceFile) const {
    Assert(ctx.Log);
    Assert(ctx.Glslang);
    Assert(ctx.Reflection);

    int sourceLine = 1;
    bool commentSingleLine{ false };
    bool commentMultiLine{ false };
    EShaderAnnotation annotations{ Default };

    const auto parseSet = [&]() -> bool {
        FStringView it{ content };
        EatSpaces(it);

        const FStringView id = EatDigits(it);
        if (id.empty() || EatSpaces(it).empty()) {
            ctx.Log->Invoke(ELoggerVerbosity::Error, sourceFile, sourceLine, "expected a set index"_view, {});
            return false;
        }

        FStringView name;
        if (it.StartsWith('"')) { // identifiers can be quoted
            it.Eat(1);
            name = EatUntil(it, '"');
            if (name.empty() || not Equals(it.Eat(1), "\"")) {
                ctx.Log->Invoke(ELoggerVerbosity::Error, sourceFile, sourceLine, "unterminated string quote found in set identifier"_view, {});
                return false;
            }
        }
        else {
            name = EatIdentifier(it);
            if (name.empty()) {
                ctx.Log->Invoke(ELoggerVerbosity::Error, sourceFile, sourceLine, "empty set identifier"_view, {});
                return false;
            }
        }

        u32 index{ INDEX_NONE };
        if (not Atoi(&index, id, 10)) {
            ctx.Log->Invoke(ELoggerVerbosity::Error, sourceFile, sourceLine, "invalid number for set index"_view, {
                {"Index", id}
            });
            return false;
        }

        const TPtrRef<FPipelineDesc::FDescriptorSet> ds =
            ctx.Reflection->Layout.DescriptorSets.MakeView().Any(
                [index](const FPipelineDesc::FDescriptorSet& ds) NOEXCEPT -> bool {
                    return (ds.BindingIndex == index);
                });
        if (not ds) {
            ctx.Log->Invoke(ELoggerVerbosity::Error, sourceFile, sourceLine, "no descriptor set description found with this binding index"_view, {
                {"Index", index},
            });
            return false;
        }

        content = it;
        ds->Id = FDescriptorSetID{ name };
        return true;
    };

    const auto parseUniform = [&]() -> bool {
        // ex:
        //  buffer <SSBO> { ...
        //  uniform <UBO> { ...
        //  uniform image* <name> ...

        FStringView it{ content };

        while (not it.empty()) {
            EatSpaces(it);

            const FStringView word = EatIdentifier(it);
            if (word.empty()) {
                if (Equals(it.Eat(1), "}")) {
                    ctx.Log->Invoke(ELoggerVerbosity::Error, sourceFile, sourceLine, "unterminated uniform declaration scope, expected '}'"_view, {});
                    return false;
                }
                continue;
            }

            bool isBuffer = false;
            bool isUniform = false;
            bool isUniformImage = false;
            bool isUniformSampler = false;

            FStringView name;
            if (Equals(word, "buffer")) {
                isBuffer = true;

                EatSpaces(it);
                name = EatIdentifier(it);
            }
            else
            if (Equals(word, "uniform")) {
                isUniform = true;
                EatSpaces(it);
                isUniformImage = it.StartsWith("image"_view); // optional
                isUniformSampler = it.StartsWith("sampler"_view); // optional
                if (isUniformImage || isUniformSampler)
                    EatIdentifier(it);

                EatSpaces(it);
                name = EatIdentifier(it);
            }
            else {
                name = word;
            }

            if (name.empty()) {
                ctx.Log->Invoke(ELoggerVerbosity::Error, sourceFile, sourceLine, "found an empty uniform name"_view, {});
                return false;
            }

            EatSpaces(it);
            if (not (it.Eat("{") || it.Eat(";")))
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
                                ctx.Log->Invoke(ELoggerVerbosity::Warning, sourceFile, sourceLine, "@dynamic-offset is only supported on buffers, but found on image"_view, {
                                    {"ResourceName", name},
                                });
                            if (annotations & EShaderAnnotation::WriteDiscard)
                                image.State |= EResourceFlags::InvalidateBefore;
                            return true;
                        },
                        [&](FPipelineDesc::FUniformBuffer& ubo) {
                            if (annotations & EShaderAnnotation::DynamicOffset)
                                ubo.State |= EResourceFlags::BufferDynamicOffset;
                            if (annotations & EShaderAnnotation::WriteDiscard)
                                ctx.Log->Invoke(ELoggerVerbosity::Warning, sourceFile, sourceLine, "@write-discard is only supported on images or storage buffers, but found on uniform buffer"_view, {
                                    {"ResourceName", name},
                                });
                            return true;
                        },
                        [&](FPipelineDesc::FStorageBuffer& ssbo) {
                            if (annotations & EShaderAnnotation::DynamicOffset)
                                ssbo.State |= EResourceFlags::BufferDynamicOffset;
                            if (annotations & EShaderAnnotation::WriteDiscard)
                                ssbo.State |= EResourceFlags::InvalidateBefore;
                            return true;
                        },
                        [&](FPipelineDesc::FTexture&) {
                            ctx.Log->Invoke(ELoggerVerbosity::Warning, sourceFile, sourceLine, "unsupported annotations found on texture"_view, {
                                {"ResourceName", name},
                                {"Annotations", ToString(annotations)},
                            });
                            return false;
                        },
                        [&](FPipelineDesc::FSampler&) {
                            ctx.Log->Invoke(ELoggerVerbosity::Warning, sourceFile, sourceLine, "unsupported annotations found on sampler"_view, {
                                {"ResourceName", name},
                                {"Annotations", ToString(annotations)},
                            });
                            return false;
                        },
                        [&](FPipelineDesc::FSubpassInput&) {
                            ctx.Log->Invoke(ELoggerVerbosity::Warning, sourceFile, sourceLine, "unsupported annotations found on sub-pass input"_view, {
                                {"ResourceName", name},
                                {"Annotations", ToString(annotations)},
                            });
                            return false;
                        },
                        [&](FPipelineDesc::FRayTracingScene&) {
                            ctx.Log->Invoke(ELoggerVerbosity::Warning, sourceFile, sourceLine, "unsupported annotations found on ray-tracing scene"_view, {
                                {"ResourceName", name},
                                {"Annotations", ToString(annotations)},
                            });
                            return false;
                        },
                        [](std::monostate&) {
                            AssertNotReached();
                        });

                    break;
                }

                content = it;
                return found;
            }
        }

        return false;
    };

    while (not content.empty()) {
        const char c = content.front();
        const char n = (content.size() > 1 ? content[1] : 0_i8);
        content.Eat(1);

        const bool newLine1 = (c == '\r') && (n == '\n'); // windows
        const bool newLine2 = (c == '\n') || (c == '\r'); // linux | mac
        if (newLine1 || newLine2) {
            sourceLine++;

            if (newLine1)
                content.Eat(1);
            commentSingleLine = false;

            if ((annotations ^ (EShaderAnnotation::DynamicOffset | EShaderAnnotation::WriteDiscard)) && not commentMultiLine) {
                PPE_LOG_CHECK(PipelineCompiler, parseUniform());
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

        const FStringView id = EatWhile(content, [](char ch) {
            return (IsIdentifier(ch) || ch == '-');
        });

        if (Equals(id, "set")) {
            PPE_LOG_CHECK(PipelineCompiler, parseSet());
            annotations |= EShaderAnnotation::Set;
        }
        else
        if (Equals(id, "discard"))
            annotations |= EShaderAnnotation::WriteDiscard;
        else
        if (Equals(id, "dynamic-offset"))
            annotations |= EShaderAnnotation::DynamicOffset;
        else {
            ctx.Log->Invoke(ELoggerVerbosity::Error, sourceFile, sourceLine, "unknown shader annotation"_view, {
                {"Annotation", id},
            });
            return false;
        }

        content.Eat(",");
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
                    PPE_LOG_CHECK(PipelineCompiler, ProcessExternalObjects_(ctx, *interm));
                }
            break;
            // uniforms, buffers, ...
            case TOperator::EOpLinkerObjects:
                for (TIntermNode* interm : aggr->getSequence()) {
                    Assert(interm);
                    PPE_LOG_CHECK(PipelineCompiler, DeserializeExternalObjects_(ctx, *interm));
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
    PPE_LOG_CHECK(PipelineCompiler, node and node->getAsSymbolNode());

    const glslang::TString& str = node->getAsSymbolNode()->getName();
    const FStringView result{ str.c_str(), str.size() };
    return StartsWith(result, "anon@"_view) ? Default : result;
}
//----------------------------------------------------------------------------
FUniformID FVulkanSpirvCompiler::ExtractBufferUniformID_(const glslang::TType& type) {
    const glslang::TString& name = type.getTypeName();
    return FUniformID{ FStringView{ name.c_str(), name.size() } };
}
//----------------------------------------------------------------------------
EImageSampler FVulkanSpirvCompiler::ExtractImageSampler_(const FCompilationContext& ctx, const glslang::TType& type) {
    using namespace glslang;

    if (type.getBasicType() == TBasicType::EbtSampler and not type.isSubpass()) {
        const TSampler& sampler = type.getSampler();

        EPixelFormat pixelFormat{ Zero };

        if (sampler.isImage()) {
            switch (type.getQualifier().layoutFormat) {
            case TLayoutFormat::ElfRgba32f: pixelFormat = EPixelFormat::RGBA32f; break;
            case TLayoutFormat::ElfRgba16f: pixelFormat = EPixelFormat::RGBA16f; break;
            case TLayoutFormat::ElfR32f: pixelFormat = EPixelFormat::R32f; break;
            case TLayoutFormat::ElfRgba8: pixelFormat = EPixelFormat::RGBA8_UNorm; break;
            case TLayoutFormat::ElfRgba8Snorm: pixelFormat = EPixelFormat::RGBA8_SNorm; break;
            case TLayoutFormat::ElfRg32f: pixelFormat = EPixelFormat::RG32f; break;
            case TLayoutFormat::ElfRg16f: pixelFormat = EPixelFormat::RG16f; break;
            case TLayoutFormat::ElfR11fG11fB10f: pixelFormat = EPixelFormat::RGB_11_11_10f; break;
            case TLayoutFormat::ElfR16f: pixelFormat = EPixelFormat::R16f; break;
            case TLayoutFormat::ElfRgba16: pixelFormat = EPixelFormat::RGBA16_UNorm; break;
            case TLayoutFormat::ElfRgb10A2: pixelFormat = EPixelFormat::RGB10_A2_UNorm; break;
            case TLayoutFormat::ElfRg16: pixelFormat = EPixelFormat::RG16_UNorm; break;
            case TLayoutFormat::ElfRg8: pixelFormat = EPixelFormat::RG8_UNorm; break;
            case TLayoutFormat::ElfR16: pixelFormat = EPixelFormat::R16_UNorm; break;
            case TLayoutFormat::ElfR8: pixelFormat = EPixelFormat::R8_UNorm; break;
            case TLayoutFormat::ElfRgba16Snorm: pixelFormat = EPixelFormat::RGBA16_SNorm; break;
            case TLayoutFormat::ElfRg16Snorm: pixelFormat = EPixelFormat::RG16_SNorm; break;
            case TLayoutFormat::ElfRg8Snorm: pixelFormat = EPixelFormat::RG8_SNorm; break;
            case TLayoutFormat::ElfR16Snorm: pixelFormat = EPixelFormat::R16_SNorm; break;
            case TLayoutFormat::ElfR8Snorm: pixelFormat = EPixelFormat::R8_SNorm; break;
            case TLayoutFormat::ElfRgba32i: pixelFormat = EPixelFormat::RGBA32i; break;
            case TLayoutFormat::ElfRgba16i: pixelFormat = EPixelFormat::RGBA16i; break;
            case TLayoutFormat::ElfRgba8i: pixelFormat = EPixelFormat::RGBA8i; break;
            case TLayoutFormat::ElfR32i: pixelFormat = EPixelFormat::R32i; break;
            case TLayoutFormat::ElfRg32i: pixelFormat = EPixelFormat::RG32i; break;
            case TLayoutFormat::ElfRg16i: pixelFormat = EPixelFormat::RG16i; break;
            case TLayoutFormat::ElfRg8i: pixelFormat = EPixelFormat::RG8i; break;
            case TLayoutFormat::ElfR16i: pixelFormat = EPixelFormat::R16i; break;
            case TLayoutFormat::ElfR8i: pixelFormat = EPixelFormat::R8i; break;
            case TLayoutFormat::ElfRgba32ui: pixelFormat = EPixelFormat::RGBA32u; break;
            case TLayoutFormat::ElfRgba16ui: pixelFormat = EPixelFormat::RGBA16u; break;
            case TLayoutFormat::ElfRgba8ui: pixelFormat = EPixelFormat::RGBA8_UNorm; break;
            case TLayoutFormat::ElfR32ui: pixelFormat = EPixelFormat::R32u; break;
            case TLayoutFormat::ElfRg32ui: pixelFormat = EPixelFormat::RG32u; break;
            case TLayoutFormat::ElfRg16ui: pixelFormat = EPixelFormat::RG16u; break;
            case TLayoutFormat::ElfRgb10a2ui: pixelFormat = EPixelFormat::RGB10_A2u; break;
            case TLayoutFormat::ElfRg8ui: pixelFormat = EPixelFormat::RG8u; break;
            case TLayoutFormat::ElfR16ui: pixelFormat = EPixelFormat::R16u; break;
            case TLayoutFormat::ElfR8ui: pixelFormat = EPixelFormat::R8u; break;

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

        EImageSampler::EType resourceType{ Zero };
        if (sampler.type == TBasicType::EbtFloat) {
            resourceType = EImageSampler::_Float;
        }
        else
        if (sampler.type == TBasicType::EbtUint) {
            resourceType = EImageSampler::_UInt;
        }
        else
        if (sampler.type == TBasicType::EbtInt) {
            resourceType = EImageSampler::_Int;
        }
        else {
            const TString samplerDesc = sampler.getString();
            ctx.Log->Invoke(ELoggerVerbosity::Error, ctx.SourceFile, 0, "unsupported image value type"_view, {
                {"Sampler", FStringView{ samplerDesc.c_str(), samplerDesc.size() }}
            });
            return Default;
        }

        switch (sampler.dim) {
        case TSamplerDim::Esd1D :
            if (sampler.isShadow() and sampler.isArrayed())
                return { pixelFormat, EImageSampler::_1DArray, resourceType, EImageSampler::_Shadow };
            if (sampler.isShadow())
                return { pixelFormat, EImageSampler::_1D, resourceType, EImageSampler::_Shadow };
            if (sampler.isArrayed())
                return { pixelFormat, EImageSampler::_1DArray, resourceType, Zero };
            return { pixelFormat, EImageSampler::_1D, resourceType, Zero };

        case TSamplerDim::Esd2D :
            if (sampler.isShadow() and sampler.isArrayed() )
                return { pixelFormat, EImageSampler::_2DArray, resourceType, EImageSampler::_Shadow };
            if (sampler.isShadow())
                return { pixelFormat, EImageSampler::_2D, resourceType, EImageSampler::_Shadow };
            if (sampler.isMultiSample() and sampler.isArrayed() )
                return { pixelFormat, EImageSampler::_2DMSArray, resourceType, Zero };
            if (sampler.isArrayed())
                return { pixelFormat, EImageSampler::_2DArray, resourceType, Zero };
            if (sampler.isMultiSample())
                return { pixelFormat, EImageSampler::_2DMS, resourceType, Zero };
            return { pixelFormat, EImageSampler::_2D, resourceType, Zero };

        case TSamplerDim::Esd3D :
            return { pixelFormat, EImageSampler::_3D, resourceType, Zero };

        case TSamplerDim::EsdCube :
            if (sampler.isShadow())
                return { pixelFormat, EImageSampler::_Cube, resourceType, EImageSampler::_Shadow };
            if (sampler.isArrayed())
                return { pixelFormat, EImageSampler::_CubeArray, resourceType, Zero };
            return { pixelFormat, EImageSampler::_Cube, resourceType, Zero };

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
        return EResourceState_ShaderWrite;

    if (q.isReadOnly())
        return EResourceState_ShaderRead;

    if (q.coherent or
        q.devicecoherent or
        q.queuefamilycoherent or
        q.workgroupcoherent or
        q.subgroupcoherent or
        q.volatil or
        q.restrict )
        return EResourceState_ShaderReadWrite;

    return EResourceState_ShaderReadWrite;
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
        PPE_LOG(PipelineCompiler, Error, "missing support for matrices!!!");
        AssertNotImplemented(); // #TODO: add support for matrices
    }

    AssertNotReached();
}
//----------------------------------------------------------------------------
EFragmentOutput FVulkanSpirvCompiler::ExtractFragmentOutput_(const glslang::TType& type) {
    using namespace glslang;

    PPE_LOG_CHECK(PipelineCompiler, type.getVectorSize() == 4);

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

    PPE_LOG_CHECK(PipelineCompiler, bufferType.isStruct());
    PPE_LOG_CHECK(PipelineCompiler, bufferType.getQualifier().isUniformOrBuffer() or
                                bufferType.getQualifier().layoutPushConstant );
    PPE_LOG_CHECK(PipelineCompiler, bufferType.getQualifier().layoutPacking == ElpStd140 or
                                bufferType.getQualifier().layoutPacking == ElpStd430 );

    int memberSize{ 0 };
    int offset{ 0 };
    const TTypeList& structFields{ *bufferType.getStruct() };

    forrange(member, 0, structFields.size()) {
        const TType& memberType = *structFields[member].type;
        const TQualifier& memberQualifier = memberType.getQualifier();
        const TLayoutMatrix subMatrixLayout = memberQualifier.layoutMatrix;

        int dummyStride;
        int memberAlignment = ctx.Intermediate->getBaseAlignment(
            memberType, memberSize, dummyStride,
            bufferType.getQualifier().layoutPacking,
            subMatrixLayout != ElmNone
                ? subMatrixLayout == ElmRowMajor
                : bufferType.getQualifier().layoutMatrix == ElmRowMajor );
        Assert(Meta::IsPow2(memberAlignment));

        if (memberQualifier.hasOffset()) {
            Assert(glslang::IsMultipleOfPow2(memberQualifier.layoutOffset, memberAlignment));

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

        // for last member:
        if (member + 1 == structFields.size() and memberType.isUnsizedArray())
            *outArrayStride = checked_cast<u32>(dummyStride);
        else
            offset += memberSize;
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
        Assert(!!uniformId);

        FPipelineDesc::FVariantUniform un;
        un.Index = ToBindingIndex_(ctx, (qualifier.hasBinding()
            ? checked_cast<u32>(qualifier.layoutBinding)
            : UMax ));
        un.StageFlags = ctx.CurrentStage;
        un.Data = std::move(resource);
        un.ArraySize = GLSLangArraySize_(type);

        uniforms.insert({ std::move(uniformId), std::move(un) });
    };

    if (type.getBasicType() == TBasicType::EbtSampler) {
        // image
        if (type.getSampler().isImage()) {
            FPipelineDesc::FImage image{};
            image.Type = ExtractImageSampler_(ctx, type);
            image.State = ExtractShaderAccessType_(qualifier) | EResourceShaderStages_FromShaders(ctx.CurrentStage);

            insertUniform(ExtractUniformID_(&node), std::move(image));
            return true;
        }

        // subpass
        if (type.getSampler().isSubpass()) {
            FPipelineDesc::FSubpassInput subpass{};
            subpass.AttachmentIndex = (qualifier.hasAttachment() ? checked_cast<u32>(qualifier.layoutAttachment) : UMax);
            subpass.IsMultiSample = false; // #TODO
            subpass.State = EResourceState_InputAttachment | EResourceShaderStages_FromShaders(ctx.CurrentStage);

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
            texture.Type = ExtractImageSampler_(ctx, type);
            texture.State = EResourceState_ShaderSample | EResourceShaderStages_FromShaders(ctx.CurrentStage);

            insertUniform(ExtractUniformID_(&node), std::move(texture));
            return true;
        }
    }

    // push constants
    if (qualifier.layoutPushConstant) {
        u32 staticSize{}, arrayStride{}, minOffset{};
        PPE_LOG_CHECK(PipelineCompiler, CalculateStructSize_(&staticSize, &arrayStride, &minOffset, ctx, type));

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
        PPE_LOG_CHECK(PipelineCompiler, type.isStruct());

        if (qualifier.layoutShaderRecord)
            return true;

        // uniform block
        if (qualifier.storage == TStorageQualifier::EvqUniform) {
            FPipelineDesc::FUniformBuffer ubuf{};
            ubuf.State = EResourceState_UniformRead | EResourceShaderStages_FromShaders(ctx.CurrentStage);

            u32 stride{}, offset{};
            PPE_LOG_CHECK(PipelineCompiler, CalculateStructSize_(&ubuf.Size, &stride, &offset, ctx, type));
            PPE_LOG_CHECK(PipelineCompiler, 0 == offset);

            insertUniform(ExtractBufferUniformID_(type), std::move(ubuf));
            return true;
        }

        // storage block
        if (qualifier.storage == TStorageQualifier::EvqBuffer) {
            FPipelineDesc::FStorageBuffer sbuf{};
            sbuf.State = ExtractShaderAccessType_(qualifier) | EResourceShaderStages_FromShaders(ctx.CurrentStage);

            u32 offset{};
            PPE_LOG_CHECK(PipelineCompiler, CalculateStructSize_(&sbuf.StaticSize, &sbuf.ArrayStride, &offset, ctx, type));
            PPE_LOG_CHECK(PipelineCompiler, 0 == offset);

            insertUniform(ExtractBufferUniformID_(type), std::move(sbuf));
            return true;
        }
    }

    // acceleration structure
    if (type.getBasicType() == TBasicType::EbtAccStruct) {
        FPipelineDesc::FRayTracingScene rtScene{};
        rtScene.State = EResourceState_RayTracingShaderRead;

        insertUniform(ExtractUniformID_(&node), std::move(rtScene));
        return true;
    }

    if (qualifier.storage == TStorageQualifier::EvqPayload or
        qualifier.storage == TStorageQualifier::EvqPayloadIn or
        qualifier.storage == TStorageQualifier::EvqHitAttr or
        qualifier.storage == TStorageQualifier::EvqCallableData or
        qualifier.storage == TStorageQualifier::EvqCallableDataIn ) {
        return true; // #TODO
    }

    // uniform
    if (qualifier.storage == TStorageQualifier::EvqUniform) {
        PPE_LOG(PipelineCompiler, Error, "uniform is not supported for Vulkan!");
        return false;
    }

    PPE_LOG(PipelineCompiler, Error, "uniform external type!");
    return false;
}
//----------------------------------------------------------------------------
void FVulkanSpirvCompiler::MergeWithGeometryInputPrimitive_(
    const FCompilationContext& ctx,
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
        ctx.Log->Invoke(ELoggerVerbosity::Error, ctx.SourceFile, 0, "invalid geometry input primitive type!"_view, {
            {"GeometryType", Meta::EnumOrd(geometryType)},
        });
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
        MergeWithGeometryInputPrimitive_(ctx, &ctx.Reflection->Vertex.SupportedTopology, ctx.Intermediate->getInputPrimitive());
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

    FPlatformMemory::Memset(outResources, 0, sizeof(*outResources)); // reset eventual padding

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
void FVulkanSpirvCompiler::SetCurrentResourceLimits(const FVulkanDevice& device) {
    Assert(VK_NULL_HANDLE != device.vkPhysicalDevice());

    GenerateDefaultResources_(&_builtInResources);

    VkPhysicalDeviceProperties properties{};

#ifdef VK_NV_mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeatures = {};
    VkPhysicalDeviceMeshShaderPropertiesNV meshShaderProperties = {};
#endif

    device.vkGetPhysicalDeviceProperties(device.vkPhysicalDevice(), &properties);

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

        device.vkGetPhysicalDeviceFeatures2(device.vkPhysicalDevice(), &features2);
        device.vkGetPhysicalDeviceProperties2(device.vkPhysicalDevice(), &properties2);
    }

    _builtInResources.maxVertexAttribs = checked_cast<int>(Min( MaxVertexAttribs, properties.limits.maxVertexInputAttributes ));
    _builtInResources.maxDrawBuffers = checked_cast<int>(Min( MaxColorBuffers, properties.limits.maxColorAttachments ));
    _builtInResources.minProgramTexelOffset = checked_cast<int>(properties.limits.minTexelOffset);
    _builtInResources.maxProgramTexelOffset = checked_cast<int>(properties.limits.maxTexelOffset);

    _builtInResources.maxComputeWorkGroupCountX = checked_cast<int>(properties.limits.maxComputeWorkGroupCount[0]);
    _builtInResources.maxComputeWorkGroupCountY = checked_cast<int>(properties.limits.maxComputeWorkGroupCount[1]);
    _builtInResources.maxComputeWorkGroupCountZ = checked_cast<int>(properties.limits.maxComputeWorkGroupCount[2]);
    _builtInResources.maxComputeWorkGroupSizeX = checked_cast<int>(properties.limits.maxComputeWorkGroupSize[0]);
    _builtInResources.maxComputeWorkGroupSizeY = checked_cast<int>(properties.limits.maxComputeWorkGroupSize[1]);
    _builtInResources.maxComputeWorkGroupSizeZ = checked_cast<int>(properties.limits.maxComputeWorkGroupSize[2]);

    _builtInResources.maxVertexOutputComponents = checked_cast<int>(properties.limits.maxVertexOutputComponents);
    _builtInResources.maxGeometryInputComponents = checked_cast<int>(properties.limits.maxGeometryInputComponents);
    _builtInResources.maxGeometryOutputComponents = checked_cast<int>(properties.limits.maxGeometryOutputComponents);
    _builtInResources.maxFragmentInputComponents = checked_cast<int>(properties.limits.maxFragmentInputComponents);

    _builtInResources.maxCombinedImageUnitsAndFragmentOutputs = checked_cast<int>(properties.limits.maxFragmentCombinedOutputResources);
    _builtInResources.maxGeometryOutputVertices = checked_cast<int>(properties.limits.maxGeometryOutputVertices);
    _builtInResources.maxGeometryTotalOutputComponents = checked_cast<int>(properties.limits.maxGeometryTotalOutputComponents);

    _builtInResources.maxTessControlInputComponents = checked_cast<int>(properties.limits.maxTessellationControlPerVertexInputComponents);
    _builtInResources.maxTessControlOutputComponents = checked_cast<int>(properties.limits.maxTessellationControlPerVertexOutputComponents);
    _builtInResources.maxTessControlTotalOutputComponents = checked_cast<int>(properties.limits.maxTessellationControlTotalOutputComponents);
    _builtInResources.maxTessEvaluationInputComponents = checked_cast<int>(properties.limits.maxTessellationEvaluationInputComponents);
    _builtInResources.maxTessEvaluationOutputComponents = checked_cast<int>(properties.limits.maxTessellationEvaluationOutputComponents);
    _builtInResources.maxTessPatchComponents = checked_cast<int>(properties.limits.maxTessellationControlPerPatchOutputComponents);
    _builtInResources.maxPatchVertices = checked_cast<int>(properties.limits.maxTessellationPatchSize);
    _builtInResources.maxTessGenLevel = checked_cast<int>(properties.limits.maxTessellationGenerationLevel);

    _builtInResources.maxViewports = checked_cast<int>(Min(MaxViewports, properties.limits.maxViewports));
    _builtInResources.maxClipDistances = checked_cast<int>(properties.limits.maxClipDistances);
    _builtInResources.maxCullDistances = checked_cast<int>(properties.limits.maxCullDistances);
    _builtInResources.maxCombinedClipAndCullDistances = checked_cast<int>(properties.limits.maxCombinedClipAndCullDistances);

#if VK_NV_mesh_shader
    if (meshShaderFeatures.meshShader and meshShaderFeatures.taskShader) {
        _builtInResources.maxMeshOutputVerticesNV = checked_cast<int>(meshShaderProperties.maxMeshOutputVertices);
        _builtInResources.maxMeshOutputPrimitivesNV = checked_cast<int>(meshShaderProperties.maxMeshOutputPrimitives);
        _builtInResources.maxMeshWorkGroupSizeX_NV = checked_cast<int>(meshShaderProperties.maxMeshWorkGroupSize[0]);
        _builtInResources.maxMeshWorkGroupSizeY_NV = checked_cast<int>(meshShaderProperties.maxMeshWorkGroupSize[1]);
        _builtInResources.maxMeshWorkGroupSizeZ_NV = checked_cast<int>(meshShaderProperties.maxMeshWorkGroupSize[2]);
        _builtInResources.maxTaskWorkGroupSizeX_NV = checked_cast<int>(meshShaderProperties.maxTaskWorkGroupSize[0]);
        _builtInResources.maxTaskWorkGroupSizeY_NV = checked_cast<int>(meshShaderProperties.maxTaskWorkGroupSize[1]);
        _builtInResources.maxTaskWorkGroupSizeZ_NV = checked_cast<int>(meshShaderProperties.maxTaskWorkGroupSize[2]);
        _builtInResources.maxMeshViewCountNV = checked_cast<int>(meshShaderProperties.maxMeshMultiviewViewCount);
    }
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& WriteShaderAnnotations_(
    TBasicTextWriter<_Char>& oss,
    FVulkanSpirvCompiler::EShaderAnnotation annotations) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    if (annotations == Zero)
        return oss << STRING_LITERAL(_Char, "0");

    if (annotations & FVulkanSpirvCompiler::DynamicOffset) oss << sep << STRING_LITERAL(_Char, "DynamicOffset");
    if (annotations & FVulkanSpirvCompiler::Set) oss << sep << STRING_LITERAL(_Char, "Set");
    if (annotations & FVulkanSpirvCompiler::WriteDiscard) oss << sep << STRING_LITERAL(_Char, "WriteDiscard");

    return oss;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, FVulkanSpirvCompiler::EShaderAnnotation annotations) {
    return WriteShaderAnnotations_(oss, annotations);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, FVulkanSpirvCompiler::EShaderAnnotation annotations) {
    return WriteShaderAnnotations_(oss, annotations);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
