#include "stdafx.h"

#include "VirtualFileSystemTrie.h"

#include "FileSystem.h"

#include "VirtualFileSystemNode.h"
#include "VirtualFileSystemComponent.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool EachComponent_(
    const VirtualFileSystemNode* node,
    const std::function<bool(VirtualFileSystemComponent*)>& foreach) {
    Assert(node);

    for (VirtualFileSystemComponent* component : node->Components())
        if (foreach(component))
            return true;

    for (const Pair<Dirname, PVirtualFileSystemNode>& child : node->Children())
        if (EachComponent_(child.second.get(), foreach))
            return true;

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemTrie::VirtualFileSystemTrie() {}
//----------------------------------------------------------------------------
VirtualFileSystemTrie::~VirtualFileSystemTrie() {}
//----------------------------------------------------------------------------
bool VirtualFileSystemTrie::EachComponent(const std::function<bool(VirtualFileSystemComponent*)>& foreach) const {
    for (const Pair<MountingPoint, PVirtualFileSystemNode>& root : _nodes)
        if (EachComponent_(root.second.get(), foreach))
            return true;

    return false;
}
//----------------------------------------------------------------------------
VirtualFileSystemNode* VirtualFileSystemTrie::AddComponent(VirtualFileSystemComponent* component) {
    Assert(component);

    VirtualFileSystemNode* node = GetNode(component->Alias());
    Assert(node);

    node->AddComponent(component);
    return node;
}
//----------------------------------------------------------------------------
void VirtualFileSystemTrie::RemoveComponent(VirtualFileSystemComponent* component) {
    Assert(component);

    VirtualFileSystemNode* node = GetNodeIFP(component->Alias());
    Assert(node);

    node->RemoveComponent(component);
}
//----------------------------------------------------------------------------
VirtualFileSystemNode* VirtualFileSystemTrie::GetNode(const Dirpath& dirpath) {
    const MountingPoint& mountingPoint = dirpath.MountingPoint();
    Assert(!mountingPoint.empty());

    PVirtualFileSystemNode& node = _nodes.Get(mountingPoint);
    if (!node)
        node.reset(new VirtualFileSystemNode());

    VirtualFileSystemNode* result = node.get();
    for (const Dirname& dirname : dirpath.Path())
        result = result->GetNode(dirname);

    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
VirtualFileSystemNode* VirtualFileSystemTrie::GetNodeIFP(const Dirpath& dirpath) const {
    if (dirpath.MountingPoint().empty())
        return nullptr;

    VirtualFileSystemNode* result = GetNodeIFP(dirpath.MountingPoint());
    Assert(result);

    for (const Dirname& dirname : dirpath.Path())
        if (!(result = result->GetNodeIFP(dirname)))
            return nullptr;

    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
VirtualFileSystemNode* VirtualFileSystemTrie::GetNodeIFP(const MountingPoint& mountingPoint) const {
    if (mountingPoint.empty())
        return nullptr;

    const auto it = _nodes.Find(mountingPoint);
    if (_nodes.end() == it)
        return nullptr;

    Assert(it->second);
    return it->second.get();
}
//----------------------------------------------------------------------------
void VirtualFileSystemTrie::Clear() {
    _nodes.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
