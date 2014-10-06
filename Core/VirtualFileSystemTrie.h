#pragma once

#include "Core.h"
#include "AssociativeVector.h"
#include "MountingPoint.h"
#include "RefPtr.h"

#include <functional>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Dirpath;
FWD_REFPTR(VirtualFileSystemNode);
FWD_REFPTR(VirtualFileSystemComponent);
//----------------------------------------------------------------------------
class VirtualFileSystemTrie {
public:
    typedef ASSOCIATIVE_VECTOR(FileSystem, MountingPoint, PVirtualFileSystemNode) nodes_type;

    VirtualFileSystemTrie();
    ~VirtualFileSystemTrie();

    VirtualFileSystemTrie(const VirtualFileSystemTrie& other) = delete;
    VirtualFileSystemTrie& operator =(const VirtualFileSystemTrie& other) = delete;

    const nodes_type& Nodes() const { return _nodes; }

    bool EachComponent(const std::function<bool(VirtualFileSystemComponent*)>& foreach) const;

    VirtualFileSystemNode* AddComponent(VirtualFileSystemComponent* component);
    void RemoveComponent(VirtualFileSystemComponent* component);

    VirtualFileSystemNode* GetNode(const Dirpath& dirpath);
    VirtualFileSystemNode* GetNodeIFP(const Dirpath& dirpath) const;
    VirtualFileSystemNode* GetNodeIFP(const MountingPoint& mountingPoint) const;

    void Clear();

private:
    nodes_type _nodes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
