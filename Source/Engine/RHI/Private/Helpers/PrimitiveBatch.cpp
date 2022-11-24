// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include <stddef.h>

#include "PrimitiveBatch.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceResourceBuffer.h"
#include "Device/Geometry/IndexBuffer.h"
#include "Device/Geometry/VertexBuffer.h"
#include "Device/Geometry/PrimitiveType.h"

#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPrimitiveBatch::FPrimitiveBatch(IDeviceAPIEncapsulator* device, size_t maxBatchSize)
    : _device(device)
    , _vertices(new FVertexBuffer(vertex_type::Declaration, maxBatchSize, EBufferMode::WriteNoOverwrite, EBufferUsage::Dynamic, false))
    , _primitiveCount(0)
    , _isOpen(0) {
    Assert(_device);

    _vertices->Freeze();
    _vertices->Create(_device);
}
//----------------------------------------------------------------------------
FPrimitiveBatch::~FPrimitiveBatch() {
    Assert(0 == _isOpen);

    _vertices->Destroy(_device);
}
//----------------------------------------------------------------------------
void FPrimitiveBatch::BeginBatch(context_delegate setContext) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(0 == _isOpen);
    Assert(0 == _primitiveCount);

    _isOpen = true;
    _setContext = setContext;
}
//----------------------------------------------------------------------------
void FPrimitiveBatch::AddTriangles(const TMemoryView<vertex_type>& v) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(0 != _isOpen);
    Assert(_primitiveCount + v.size() <= _vertices->VertexCount());

    _vertices->SetData(_device, _primitiveCount * sizeof(vertex_type), MakeRawView(v));
    _primitiveCount += v.size();
}
//----------------------------------------------------------------------------
void FPrimitiveBatch::EndBatch() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(0 != _isOpen);
    Assert(_primitiveCount <= _vertices->VertexCount());

    IDeviceAPIContext* immediate = _device->Immediate();

    _setContext.InvokeIFP(_device->Immediate());
    _device->Immediate()->DrawPrimitives(EPrimitiveType::TriangleList, 0, _primitiveCount);

    _isOpen = false;
    _primitiveCount = 0;
    _setContext = context_delegate{};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
