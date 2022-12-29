#pragma once

#include "PipelineCompiler_fwd.h"

#include "RHIVulkan_fwd.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

#include "Diagnostic/Logger.h"

#if USE_PPE_RHIDEBUG
#   include "glsl_trace-external.h"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
using FVulkanShaderDebugUtils = std::variant<std::monostate, ShaderTrace>;
using FVulkanSharedDebugUtils = TRefCountable<FVulkanShaderDebugUtils>;
using PVulkanSharedDebugUtils = TRefPtr<FVulkanSharedDebugUtils>;
#endif
//----------------------------------------------------------------------------
template <>
class TVulkanDebuggableShaderData<FRawData> final : public IShaderData<FRawData> {
public:
    template <typename>
    friend class TVulkanDebuggableShaderData;

    using IShaderData<FRawData>::FDataRef;
    using IShaderData<FRawData>::FFingerprint;

    TVulkanDebuggableShaderData(
        const FStringView& entryPoint, FRawData&& rdata, const FFingerprint& fingerprint
        ARGS_IF_RHIDEBUG(const FStringView& debugName) ) NOEXCEPT
    :   _data(std::move(rdata))
    ,   _fingerprint(fingerprint)
    ,   _entryPoint(entryPoint)
#if USE_PPE_RHIDEBUG
    ,   _debugName(debugName)
#endif
    {}

#if USE_PPE_RHIDEBUG
    TVulkanDebuggableShaderData(
        const FStringView& entryPoint, FRawData&& rdata, const FFingerprint& fingerprint,
        const FStringView& debugName, PVulkanSharedDebugUtils&& debugInfo ) NOEXCEPT
    :   _data(std::move(rdata))
    ,   _fingerprint(fingerprint)
    ,   _entryPoint(entryPoint)
    ,   _debugName(debugName)
    ,   _debugInfo(std::move(debugInfo))
    {}
#endif

    FDataRef Data() const NOEXCEPT override { return FDataRef{ std::addressof(_data) }; }
    FConstChar EntryPoint() const NOEXCEPT override { return _entryPoint.c_str(); }
    FFingerprint Fingerprint() const NOEXCEPT override { return _fingerprint; }

#if USE_PPE_RHIDEBUG
    FConstChar DebugName() const NOEXCEPT override { return _debugName; }
    const PVulkanSharedDebugUtils& DebugInfo() const NOEXCEPT { return _debugInfo; }

    bool ParseDebugOutput(TAppendable<FString> outp, EShaderDebugMode mode, FRawMemoryConst trace) override {
        LOG_CHECK(PipelineCompiler, EShaderDebugMode::Trace == mode || EShaderDebugMode::Profiling == mode);
        if (not _debugInfo)
            return false;

        std::vector<std::string> tmp; // #TODO : port glsl_trace as an internal project
        const bool result = std::get<ShaderTrace>(*_debugInfo).ParseShaderTrace(trace.data(), trace.size(), tmp);

        for (const std::string& log : tmp)
            outp.emplace_back(FStringView{log.c_str(), log.size()});

        return result;
    }
#endif

    void TearDown(const FVulkanDeviceInfo& ) {}

private:
    FRawData _data{};
    FFingerprint _fingerprint{};
    TStaticString<64> _entryPoint;

#if USE_PPE_RHIDEBUG
    TStaticString<64> _debugName;
    PVulkanSharedDebugUtils _debugInfo;
#endif
};
//----------------------------------------------------------------------------
template <>
class TVulkanDebuggableShaderData<FShaderModule> final : public FVulkanShaderModule {
public:
    template <typename>
    friend class TVulkanDebuggableShaderData;

    using FVulkanShaderModule::FDataRef;
    using FVulkanShaderModule::FFingerprint;
    using FVulkanShaderModule::vkShaderModule;
    using FVulkanShaderModule::Data;
    using FVulkanShaderModule::EntryPoint;
    using FVulkanShaderModule::Fingerprint;
#if USE_PPE_RHIDEBUG
    using FVulkanShaderModule::DebugName;
#endif

    TVulkanDebuggableShaderData(const FStringView& entryPoint, VkShaderModule vkShaderModule, const FFingerprint& fingerprint) NOEXCEPT
    :   FVulkanShaderModule(vkShaderModule, fingerprint, entryPoint)
    {}

    TVulkanDebuggableShaderData(VkShaderModule vkShaderModule, const PShaderBinaryData& compiledSpirv) NOEXCEPT
    :   FVulkanShaderModule(vkShaderModule, compiledSpirv->Fingerprint(), compiledSpirv->EntryPoint().MakeView()) {
        Assert(VK_NULL_HANDLE != vkShaderModule);
    }

#if USE_PPE_RHIDEBUG
    TVulkanDebuggableShaderData(VkShaderModule vkShaderModule, const FVulkanDebuggableShaderSPIRV& debuggableSpirv) NOEXCEPT
    :   FVulkanShaderModule(vkShaderModule, debuggableSpirv.Fingerprint(), debuggableSpirv.EntryPoint().MakeView()) {
        Assert(VK_NULL_HANDLE != vkShaderModule);
    }

    NODISCARD bool Construct(const FVulkanDevice& device, FStringView debugName, const PVulkanSharedDebugUtils& debugInfo) {
        _debugInfo = debugInfo;
        return FVulkanShaderModule::Construct(device, debugName);
    }

    bool ParseDebugOutput(TAppendable<FString> outp, EShaderDebugMode mode, FRawMemoryConst trace) override final {
        LOG_CHECK(PipelineCompiler, EShaderDebugMode::Trace == mode || EShaderDebugMode::Profiling == mode);
        if (not _debugInfo)
            return false;

        std::vector<std::string> tmp; // #TODO : port glsl_trace as an internal project
        const bool result = std::get<ShaderTrace>(*_debugInfo).ParseShaderTrace(trace.data(), trace.size(), tmp);

        for (const std::string& log : tmp)
            outp.emplace_back(FStringView{log.c_str(), log.size()});

        return result;
    }
#else
    using FVulkanShaderModule::Construct;
#endif
    using FVulkanShaderModule::TearDown;

private:
#if USE_PPE_RHIDEBUG
    PVulkanSharedDebugUtils _debugInfo;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
