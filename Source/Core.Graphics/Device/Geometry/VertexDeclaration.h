#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResource.h"
#include "Core.Graphics/Device/Geometry/VertexSubPart.h"
#include "Core.Graphics/Device/Geometry/VertexTypes.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Stack.h"
#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
    template <typename T>
    class MemoryView;
}

namespace Core {
namespace Graphics {
class DeviceEncapsulator;
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantVertexDeclaration);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(VertexDeclaration);
class VertexDeclaration : public DeviceResource {
public:
    STATIC_CONST_INTEGRAL(u32, MaxSubPartCount, 6);

    VertexDeclaration();
    virtual ~VertexDeclaration();

    virtual bool Available() const override;
    virtual DeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantVertexDeclaration& DeviceAPIDependantDeclaration() const {
        Assert(Frozen()); return _deviceAPIDependantDeclaration;
    }

    size_t size() const { return _subParts.size(); }
    bool empty() const { return _subParts.empty(); }

    size_t SizeInBytes() const { return _sizeInBytes; }

    MemoryView<const Pair<VertexSubPartKey, VertexSubPartPOD>> SubParts() const;

    template <VertexSubPartFormat _Format, VertexSubPartSemantic _Semantic>
    void AddSubPart(size_t index);

    template <VertexSubPartSemantic _Semantic, typename _Class, typename T>
    void AddTypedSubPart(T _Class:: *member, size_t index);

    Pair<const VertexSubPartKey *, const AbstractVertexSubPart *> SubPartByIndex(size_t index) const;
    Pair<const VertexSubPartKey *, const AbstractVertexSubPart *> SubPartBySemantic(const VertexSubPartSemantic semantic, size_t index) const;
    Pair<const VertexSubPartKey *, const AbstractVertexSubPart *> SubPartBySemanticIFP(const VertexSubPartSemantic semantic, size_t index) const;

    template <typename T>
    const VertexSubPart<T> *SubPart(const VertexSubPartKey& key) const;
    template <typename T>
    const VertexSubPart<T> *SubPartIFP(const VertexSubPartKey& key) const;

    template <VertexSubPartFormat _Format>
    const TypedVertexSubPart<_Format> *TypedSubPart(const VertexSubPartSemantic semantic, size_t index) const;
    template <VertexSubPartFormat _Format>
    const TypedVertexSubPart<_Format> *TypedSubPartIFP(const VertexSubPartSemantic semantic, size_t index) const;

    void CopyVertex(void *const dst, const void *src, size_t size) const;

    String ToString() const;

    static void Start();
    static void Shutdown();

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void OnDeviceCreate(DeviceEncapsulator *device);
    static void OnDeviceDestroy(DeviceEncapsulator *device);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    size_t _sizeInBytes;

    typedef std::pair<VertexSubPartKey, VertexSubPartPOD> vertexsubpartentry_type;
    FixedSizeStack<vertexsubpartentry_type, MaxSubPartCount> _subParts;

    PDeviceAPIDependantVertexDeclaration _deviceAPIDependantDeclaration;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantVertexDeclaration : public TypedDeviceAPIDependantEntity<VertexDeclaration> {
public:
    DeviceAPIDependantVertexDeclaration(IDeviceAPIEncapsulator *device, const VertexDeclaration *resource);
    virtual ~DeviceAPIDependantVertexDeclaration();

    virtual size_t VideoMemorySizeInBytes() const override { return 0; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class VertexDeclarator {
public:
    VertexDeclarator(VertexDeclaration *vdecl);
    ~VertexDeclarator();

    VertexDeclarator(const VertexDeclarator&) = delete;
    VertexDeclarator& operator =(const VertexDeclarator&) = delete;

    template <VertexSubPartSemantic _Semantic, typename _Value>
    void AddTypedSubPart(_Value T:: *member, size_t index) const;

    void SetResourceName(String&& name);

private:
    VertexDeclaration *_vdecl;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core

#include "Core.Graphics/Device/Geometry/VertexDeclaration-inl.h"
