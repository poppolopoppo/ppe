
#include "stdafx.h"

#include "Vulkan/Debugger/VulkanDebugger.h"

#if USE_PPE_RHIDEBUG

#include "Diagnostic/Logger.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDebugger::FVulkanDebugger() {
    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->FullDump.reserve(8);
    exclusiveData->Graphs.reserve(8);
}
//----------------------------------------------------------------------------
void FVulkanDebugger::AddBatchDump(const FStringView& name, FString&& dump) {
    AssertRelease(not name.empty());

    if (not dump.empty()) {
        const auto exclusiveData = _data.LockExclusive();
        exclusiveData->FullDump.Emplace_Overwrite(ToString(name), std::move(dump));
    }
}
//----------------------------------------------------------------------------
bool FVulkanDebugger::FrameDump(FStringBuilder* pout) {
    Assert(pout);

    const auto exclusiveData = _data.LockExclusive();
    if (exclusiveData->FullDump.empty())
        return false;

    std::sort(
        exclusiveData->FullDump.begin(),
        exclusiveData->FullDump.end(),
        [](const auto& lhs, const auto& rhs) NOEXCEPT {
            return Meta::TLess<>{}(lhs.first, rhs.first);
        });

    for (auto& [name, dump] : exclusiveData->FullDump)
        *pout << dump;

    exclusiveData->FullDump.clear();
    return true;
}
//----------------------------------------------------------------------------
void FVulkanDebugger::AddBatchGraph(FBatchGraph&& in) {
    if (not in.Body.empty()) {
        const auto exclusiveData = _data.LockExclusive();
        exclusiveData->Graphs.push_back(std::move(in));
    }
}
//----------------------------------------------------------------------------
bool FVulkanDebugger::GraphDump(FStringBuilder* pout) {
    Assert(pout);

    const auto exclusiveData = _data.LockExclusive();
    if (exclusiveData->Graphs.empty())
        return false;

    pout->clear();

    *pout
        << "digraph Framegraph {" << Eol
        << "    rankdir = LR;" << Eol
        << "    bgcolor = black;" << Eol
        << "    labelloc = top;" << Eol
        //<< "    concentrate = true;" << Eol
        << "    compound = true;" << Eol
        << "    node [shape=rectangle, margin=\"0.1,0.1\" fontname=\"helvetica\", style=filled, layer=all, penwidth=0.0];" << Eol
        << "    edge [fontname=\"helvetica\", fontsize=8, fontcolor=white, layer=all];" << Eol << Eol;

    for (FBatchGraph& graph : exclusiveData->Graphs)
        *pout << graph.Body;

    exclusiveData->Graphs.clear();

    *pout << "}" << Eol;
    return true;
}
//----------------------------------------------------------------------------
void FVulkanDebugger::LogDump() {
    const auto exclusiveData = _data.LockExclusive();
    if (exclusiveData->FullDump.empty())
        return;

#if USE_PPE_LOGGER
    std::sort(
        exclusiveData->FullDump.begin(),
        exclusiveData->FullDump.end(),
        [](const auto& lhs, const auto& rhs) NOEXCEPT{
            return Meta::TLess<>{}(lhs.first, rhs.first);
        });

    for (auto& [name, dump] : exclusiveData->FullDump) {
        FWStringBuilder wchar;
        wchar << L"--- frame dump <" << name << L"> ---" << Eol;
        wchar << dump;
        LOG_DIRECT(RHI, Debug, wchar.Written());
    }
#endif

    exclusiveData->FullDump.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
