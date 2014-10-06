#pragma once

#include "Core.h"
#include "AssociativeVector.h"
#include "Dirname.h"
#include "RefPtr.h"
#include "Vector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(VirtualFileSystemComponent);
FWD_REFPTR(VirtualFileSystemNode);
//----------------------------------------------------------------------------
class VirtualFileSystemNode : public RefCountable {
public:
    typedef VECTOR(FileSystem, PVirtualFileSystemComponent) components_type;
    typedef ASSOCIATIVE_VECTOR(FileSystem, Core::Dirname, PVirtualFileSystemNode) children_type;

    VirtualFileSystemNode();
    ~VirtualFileSystemNode();

    VirtualFileSystemNode(const VirtualFileSystemNode& other) = delete;
    VirtualFileSystemNode& operator =(const VirtualFileSystemNode& other) = delete;

    const components_type& Components() const { return _components; }
    const children_type& Children() const { return _children; }

    void AddComponent(VirtualFileSystemComponent* component);
    void RemoveComponent(VirtualFileSystemComponent* component);

    components_type::const_iterator FindComponent(VirtualFileSystemComponent* component) const;
    void RemoveComponent(const components_type::const_iterator it);

    VirtualFileSystemNode* GetNode(const Core::Dirname& dirname);
    VirtualFileSystemNode* GetNodeIFP(const Core::Dirname& dirname) const;

    void Clear();

private:
    components_type _components;
    children_type _children;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
