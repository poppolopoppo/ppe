#pragma once

#include "Graphics.h"

#include "DeviceResource.h"
#include "DeviceAPIDependantEntity.h"
#include "VertexSubPart.h"
#include "VertexTypes.h"

#include "Core/MemoryStack.h"
#include "Core/Pair.h"
#include "Core/RefPtr.h"
#include "Core/String.h"

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
class VertexDeclaration : public TypedDeviceResource<DeviceResourceType::VertexDeclaration> {
public:
    STATIC_CONST_INTEGRAL(u32, MaxSubPartCount, 6);

    VertexDeclaration();
    virtual ~VertexDeclaration();

    virtual bool Available() const override { return nullptr != _deviceAPIDependantDeclaration; }
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

private:
    size_t _sizeInBytes;

    typedef std::pair<VertexSubPartKey, VertexSubPartPOD> vertexsubpartentry_type;
    StaticStack<vertexsubpartentry_type, MaxSubPartCount> _subParts;

    PDeviceAPIDependantVertexDeclaration _deviceAPIDependantDeclaration;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantVertexDeclaration : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantVertexDeclaration(IDeviceAPIEncapsulator *device, VertexDeclaration *owner);
    virtual ~DeviceAPIDependantVertexDeclaration();

    const VertexDeclaration *Owner() const { return _owner; }

private:
    VertexDeclaration *_owner;
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

#include "VertexDeclaration-inl.h"
