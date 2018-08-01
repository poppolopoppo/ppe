#pragma once

#include "Graphics.h"

#include "Device/DeviceAPI_fwd.h"
#include "VertexTypes.h"

#include "Color/Color.h"
#include "Container/Vector.h"
#include "Maths/ScalarBoundingBox_fwd.h"
#include "Meta/Delegate.h"
#include "Meta/ThreadResource.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(PrimitiveBatch);
class IPrimitiveBatch : public FRefCountable {
public:
    typedef u32 index_type;
    typedef Vertex::FPosition0_Float3__Color0_UByte4N__Color1_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N vertex_type;
    typedef TDelegate<void(*)(IDeviceAPIContext* context)> context_delegate;

    virtual ~IPrimitiveBatch() {}

    virtual void BeginBatch(context_delegate setContext) = 0;
    virtual void AddTriangles(const TMemoryView<vertex_type>& v) = 0;
    virtual void EndBatch() = 0;
};
//----------------------------------------------------------------------------
class FPrimitiveBatch : public IPrimitiveBatch, public Meta::FThreadResource {
public:
    using IPrimitiveBatch::index_type;
    using IPrimitiveBatch::vertex_type;
    using IPrimitiveBatch::context_delegate;

    FPrimitiveBatch(IDeviceAPIEncapsulator* device, size_t maxBatchSize);
    virtual ~FPrimitiveBatch();

    FPrimitiveBatch(const FPrimitiveBatch&) = delete;
    FPrimitiveBatch& operator =(const FPrimitiveBatch&) = delete;

    virtual void BeginBatch(context_delegate setContext) override;
    virtual void AddTriangles(const TMemoryView<vertex_type>& v) override;
    virtual void EndBatch() override;

private:
    IDeviceAPIEncapsulator* _device;
    PVertexBuffer _vertices;
    size_t _isOpen : 1;
    size_t _primitiveCount : sizeof(size_t) - 1;
    context_delegate _setContext;
};
//----------------------------------------------------------------------------
class FDeferredPrimitiveBatch : public IPrimitiveBatch {
public:
    using IPrimitiveBatch::index_type;
    using IPrimitiveBatch::vertex_type;
    using IPrimitiveBatch::context_delegate;

    FDeferredPrimitiveBatch();
    virtual ~FDeferredPrimitiveBatch();

    virtual void BeginBatch(context_delegate setContext) override;
    virtual void AddTriangles(const TMemoryView<vertex_type>& v) override;
    virtual void EndBatch() override;

    void Flush(IPrimitiveBatch* other);

private:
    VECTOR(Vertex, index_type) _indices;
    VECTOR(Vertex, vertex_type) _vertices;
};
//----------------------------------------------------------------------------
struct FPrimitiveStyle {
    float Thickness         = 1.f;
    FLinearColor Fill       = FLinearColor::PaperWhite;
    FLinearColor Stroke     = FLinearColor::PaperWhite;

    static const FPrimitiveStyle Default;
};
//----------------------------------------------------------------------------
class FPrimitiveRenderToolbox {
public:
    typedef IPrimitiveBatch::context_delegate context_delegate;

    explicit FPrimitiveRenderToolbox(IPrimitiveBatch* batch)
        : FPrimitiveRenderToolbox(batch, context_delegate{}) {}

    FPrimitiveRenderToolbox(IPrimitiveBatch* batch, context_delegate setContext)
        : _batch(batch) {
        Assert(_batch);
        _batch->BeginBatch(std::move(setContext));
    }

    ~FPrimitiveRenderToolbox() {
        _batch->EndBatch();
    }

    FPrimitiveRenderToolbox(const FPrimitiveRenderToolbox&) = delete;
    FPrimitiveRenderToolbox& operator =(const FPrimitiveRenderToolbox&) = delete;

    void Line(const float3& a, const float3& b, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Triangle(const float3& a, const float3& b, const float3& c, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Quad(const float3& a, const float3& b, const float3& c, const float3& d, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Cube(const FAabb3f& box, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Sphere(const float3& center, float radius, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Elypse(const float3& center, const float3& normal, const float3& tangent, const float3& extent, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Pyramid(const float3& center, const float3& normal, const float3& tangent, const float3& extent, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Octahedron(const float3& center, const float3& normal, const float3& tangent, const float3& extent, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Icosahedron(const float3& center, const float3& normal, const float3& tangent, const float3& extent, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void ContellatedTetraHedron(const float3& center, const float3& normal, const float3& tangent, const float3& extent, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void HemiContellatedTetraHedron(const float3& center, const float3& normal, const float3& tangent, const float3& extent, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Geosphere(const float3& center, const float3& normal, const float3& tangent, const float3& extent, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void HemiGeosphere(const float3& center, const float3& normal, const float3& tangent, const float3& extent, const FPrimitiveStyle = FPrimitiveStyle::Default);
    void Polyline(const TMemoryView<float3>& point, const FPrimitiveStyle = FPrimitiveStyle::Default);

private:
    SPrimitiveBatch _batch;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
