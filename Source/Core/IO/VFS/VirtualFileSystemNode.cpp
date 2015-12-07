#include "stdafx.h"

#include "VirtualFileSystemNode.h"

#include "VirtualFileSystemComponent.h"
#include "VirtualFileSystemTrie.h"

#include "Allocator/PoolAllocator-impl.h"
#include "IO/VirtualFileSystem.h"

#include <algorithm>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(VirtualFileSystem, VirtualFileSystemNode, );
//----------------------------------------------------------------------------
VirtualFileSystemNode::VirtualFileSystemNode() {}
//----------------------------------------------------------------------------
VirtualFileSystemNode::~VirtualFileSystemNode() {}
//----------------------------------------------------------------------------
void VirtualFileSystemNode::AddComponent(VirtualFileSystemComponent* component) {
    Assert(component);
    _components.emplace_back(component);
}
//----------------------------------------------------------------------------
void VirtualFileSystemNode::RemoveComponent(VirtualFileSystemComponent* component) {
    Assert(component);
    const components_type::const_iterator it = std::find(_components.begin(), _components.end(), component);
    Assert(_components.end() != it);
    Erase_DontPreserveOrder(_components, it);
}
//----------------------------------------------------------------------------
auto VirtualFileSystemNode::FindComponent(VirtualFileSystemComponent* component) const -> components_type::const_iterator {
    return std::find(_components.begin(), _components.end(), component);
}
//----------------------------------------------------------------------------
void VirtualFileSystemNode::RemoveComponent(const components_type::const_iterator it) {
    Erase_DontPreserveOrder(_components, it);
}
//----------------------------------------------------------------------------
VirtualFileSystemNode* VirtualFileSystemNode::GetNode(const Core::Dirname& dirname) {
    Assert(!dirname.empty());
    PVirtualFileSystemNode& node = _children.Get(dirname);
    if (!node)
        node.reset(new VirtualFileSystemNode());
    Assert(node);
    return node.get();
}
//----------------------------------------------------------------------------
VirtualFileSystemNode* VirtualFileSystemNode::GetNodeIFP(const Core::Dirname& dirname) const {
    Assert(!dirname.empty());
    const auto it = _children.Find(dirname);
    if (_children.end() == it)
        return nullptr;
    Assert(it->second);
    return it->second.get();
}
//----------------------------------------------------------------------------
void VirtualFileSystemNode::Clear() {
    _components.clear();
    _children.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
