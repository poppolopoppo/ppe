#include "stdafx.h"

#include "VertexDeclaration.h"

#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

#include "Device/DeviceEncapsulator.h"
#include "VertexTypes.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VertexDeclaration::VertexDeclaration()
:   _sizeInBytes(0) {}
//----------------------------------------------------------------------------
VertexDeclaration::~VertexDeclaration() {
    Assert(!_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
MemoryView<const Pair<VertexSubPartKey, VertexSubPartPOD>> VertexDeclaration::SubParts() const {
    return _subParts.Cast<const vertexsubpartentry_type>();
}
//----------------------------------------------------------------------------
Pair<const VertexSubPartKey *, const AbstractVertexSubPart *> VertexDeclaration::SubPartByIndex(size_t index) const {
    Assert(index < _subParts.size());

    const vertexsubpartentry_type& it = _subParts.at(index);
    return MakePair(&it.first, reinterpret_cast<const AbstractVertexSubPart *>(&it.second));
}
//----------------------------------------------------------------------------
Pair<const VertexSubPartKey *, const AbstractVertexSubPart *> VertexDeclaration::SubPartBySemantic(const VertexSubPartSemantic semantic, size_t index) const {
    for (const vertexsubpartentry_type& it : _subParts) {
        if (it.first.Semantic() == semantic &&
            it.first.Index() == index) {
            return MakePair(&it.first, reinterpret_cast<const AbstractVertexSubPart *>(&it.second));
        }
    }

    AssertNotReached();
    return MakePair(static_cast<const VertexSubPartKey *>(nullptr), static_cast<const AbstractVertexSubPart *>(nullptr));
}
//----------------------------------------------------------------------------
Pair<const VertexSubPartKey *, const AbstractVertexSubPart *> VertexDeclaration::SubPartBySemanticIFP(const VertexSubPartSemantic semantic, size_t index) const {
    for (const vertexsubpartentry_type& it : _subParts) {
        if (it.first.Semantic() == semantic &&
            it.first.Index() == index) {
            return MakePair(&it.first, reinterpret_cast<const AbstractVertexSubPart *>(&it.second));
        }
    }

    return MakePair(static_cast<const VertexSubPartKey *>(nullptr), static_cast<const AbstractVertexSubPart *>(nullptr));
}
//----------------------------------------------------------------------------
void VertexDeclaration::CopyVertex(void *const dst, const void *src, size_t size) const {
    Assert(Frozen());
    Assert(dst);
    Assert(src);
    Assert(SizeInBytes() <= size);

    for (const vertexsubpartentry_type& it : _subParts) {
        const AbstractVertexSubPart *subpart = reinterpret_cast<const AbstractVertexSubPart *>(&it.second);
        subpart->Copy(dst, src, size);
    }
}
//----------------------------------------------------------------------------
String VertexDeclaration::ToString() const {
    OStringStream oss;

    oss << "Vertex";
    for (const vertexsubpartentry_type& subPart : _subParts)
        Format( oss, "__{0}{1}_{2}",
                VertexSubPartSemanticToCStr(subPart.first.Semantic()),
                subPart.first.Index(),
                VertexSubPartFormatToCStr(subPart.first.Format()) );

    return oss.str();
}
//----------------------------------------------------------------------------
void VertexDeclaration::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantDeclaration);

    _deviceAPIDependantDeclaration = device->CreateVertexDeclaration(this);

    Assert(_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
void VertexDeclaration::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantDeclaration);

    device->DestroyVertexDeclaration(this, _deviceAPIDependantDeclaration);

    Assert(!_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration::DeviceAPIDependantVertexDeclaration(IDeviceAPIEncapsulator *device, VertexDeclaration *owner)
:   DeviceAPIDependantEntity(device)
,   _owner(owner) {
    Assert(owner);
}
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration::~DeviceAPIDependantVertexDeclaration() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexDeclaration::Start() {
    VertexTypes_Start();
}
//----------------------------------------------------------------------------
void VertexDeclaration::Shutdown() {
    VertexTypes_Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexDeclaration::OnDeviceCreate(DeviceEncapsulator *device) {
    VertexTypes_OnDeviceCreate(device);
}
//----------------------------------------------------------------------------
void VertexDeclaration::OnDeviceDestroy(DeviceEncapsulator *device) {
    VertexTypes_OnDeviceDestroy(device);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
