#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Name.h"
#include "Core.Graphics/ValueBlock.h"
#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResource.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Stack.h"
#include "Core/Container/Vector.h"
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
using VertexFormat = ValueType;
//----------------------------------------------------------------------------
class VertexSemantic : public Graphics::Name {
public:
    static const VertexSemantic Position;
    static const VertexSemantic TexCoord;
    static const VertexSemantic Color;
    static const VertexSemantic Normal;
    static const VertexSemantic Tangent;
    static const VertexSemantic Binormal;

    static VertexSemantic Invalid() { return VertexSemantic(); }

    VertexSemantic() {}

    VertexSemantic(const VertexSemantic& ) = default;
    VertexSemantic& operator =(const VertexSemantic& ) = default;

private:
    friend class VertexDeclaration;

    VertexSemantic(const Graphics::Name& name) : Name(name) {}
};
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

    size_t size() const { return _block.size(); }
    bool empty() const { return _block.empty(); }

    size_t SizeInBytes() const { return _block.SizeInBytes(); }

    const ValueBlock& Block() const { return _block; }

    MemoryView<const ValueBlock::Field> SubParts() const { return _block.MakeView(); }

    void AddSubPart(const VertexSemantic& semantic, size_t index, ValueType type, size_t offset);

    template <typename _Class, typename T>
    void AddTypedSubPart(const VertexSemantic& semantic, size_t index, T _Class:: *member);

    const ValueBlock::Field& SubPartByIndex(size_t index) const { return _block[index]; }
    const ValueBlock::Field& SubPartBySemantic(const VertexSemantic& semantic, size_t index) const;
    const ValueBlock::Field* SubPartBySemanticIFP(const VertexSemantic& semantic, size_t index) const;

    void CopyVertex(const MemoryView<u8>& dst, const MemoryView<const u8>& src) const;

    virtual void FillSubstitutions(VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>)& substitutions) const;

    static void Start();
    static void Shutdown();

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void OnDeviceCreate(DeviceEncapsulator *device);
    static void OnDeviceDestroy(DeviceEncapsulator *device);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ValueBlock _block;
    PDeviceAPIDependantVertexDeclaration _deviceAPIDependantDeclaration;
};
//----------------------------------------------------------------------------
template <typename _Class, typename T>
void VertexDeclaration::AddTypedSubPart(const VertexSemantic& semantic, size_t index, T _Class:: *member) {
    const size_t offset = (size_t)&(((_Class *)nullptr)->*member);
    Assert(0 == offset % sizeof(u32));
    AddSubPart(semantic, index, ValueTraits<T>::TypeId, offset);
}
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
} //!namespace Graphics
} //!namespace Core
