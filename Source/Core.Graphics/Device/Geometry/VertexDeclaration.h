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
    class TMemoryView;
}

namespace Core {
namespace Graphics {
class FDeviceEncapsulator;
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantVertexDeclaration);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FVertexFormat = EValueType;
//----------------------------------------------------------------------------
class FVertexSemantic : public Graphics::FName {
public:
    static const FVertexSemantic Position;
    static const FVertexSemantic TexCoord;
    static const FVertexSemantic Color;
    static const FVertexSemantic Normal;
    static const FVertexSemantic Tangent;
    static const FVertexSemantic Binormal;

    static FVertexSemantic Invalid() { return FVertexSemantic(); }
    static FVertexSemantic FromName(const FName& name);

    FVertexSemantic() {}

    FVertexSemantic(const FVertexSemantic& ) = default;
    FVertexSemantic& operator =(const FVertexSemantic& ) = default;

private:
    friend class FVertexDeclaration;

    FVertexSemantic(const Graphics::FName& name) : FName(name) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(VertexDeclaration);
class FVertexDeclaration : public FDeviceResource {
public:
    STATIC_CONST_INTEGRAL(u32, MaxSubPartCount, 6);

    FVertexDeclaration();
    virtual ~FVertexDeclaration();

    virtual bool Available() const override;
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantVertexDeclaration& DeviceAPIDependantDeclaration() const {
        Assert(Frozen()); return _deviceAPIDependantDeclaration;
    }

    size_t size() const { return _block.size(); }
    bool empty() const { return _block.empty(); }

    size_t SizeInBytes() const { return _block.SizeInBytes(); }

    const FValueBlock& FBlock() const { return _block; }

    TMemoryView<const FValueField> SubParts() const { return _block.MakeView(); }

    void AddSubPart(const FVertexSemantic& semantic, size_t index, EValueType type, size_t offset);

    template <typename _Class, typename T>
    void AddTypedSubPart(const FVertexSemantic& semantic, size_t index, T _Class:: *member);

    const FValueField& SubPartByIndex(size_t index) const { return _block[index]; }
    const FValueField& SubPartBySemantic(const FVertexSemantic& semantic, size_t index) const;
    const FValueField* SubPartBySemanticIFP(const FVertexSemantic& semantic, size_t index) const;

    void CopyVertex(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src) const;

    virtual void FillSubstitutions(VECTOR_THREAD_LOCAL(Shader, TPair<FString COMMA FString>)& substitutions) const;

    static void Start();
    static void Shutdown();

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void OnDeviceCreate(FDeviceEncapsulator *device);
    static void OnDeviceDestroy(FDeviceEncapsulator *device);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FValueBlock _block;
    PDeviceAPIDependantVertexDeclaration _deviceAPIDependantDeclaration;
};
//----------------------------------------------------------------------------
template <typename _Class, typename T>
void FVertexDeclaration::AddTypedSubPart(const FVertexSemantic& semantic, size_t index, T _Class:: *member) {
    const size_t offset = (size_t)&(((_Class *)nullptr)->*member);
    Assert(0 == offset % sizeof(u32));
    AddSubPart(semantic, index, TValueTraits<T>::TypeId, offset);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantVertexDeclaration : public TTypedDeviceAPIDependantEntity<FVertexDeclaration> {
public:
    FDeviceAPIDependantVertexDeclaration(IDeviceAPIEncapsulator *device, const FVertexDeclaration *resource);
    virtual ~FDeviceAPIDependantVertexDeclaration();

    virtual size_t VideoMemorySizeInBytes() const override { return 0; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
