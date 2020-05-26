#include "stdafx.h"

#include "DX11ShaderEffect.h"

#include "DX11ShaderProgram.h"
#include "DirectX11/DX11DeviceAPIEncapsulator.h"
#include "DirectX11/Geometry/DX11VertexDeclaration.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Shader/ShaderCompiled.h"

#include "Allocator/PoolAllocator-impl.h"
#include "Memory/UniqueView.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static HRESULT DX11CreateInputLayout_(
    ::ID3D11Device *device,
    const FShaderCompiled* compiled,
    const FDX11VertexDeclaration *vertexDeclaration,
    TComPtr<::ID3D11InputLayout>& inputLayout ) {
    const TMemoryView<const ::D3D11_INPUT_ELEMENT_DESC> layout = vertexDeclaration->Layout();

    return device->CreateInputLayout(
        layout.Pointer(),
        checked_cast<UINT>(layout.size()),
        compiled->Blob().Pointer(),
        compiled->Blob().SizeInBytes(),
        inputLayout.GetAddressOf()
        );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11ShaderEffect, );
//----------------------------------------------------------------------------
FDX11ShaderEffect::FDX11ShaderEffect(IDeviceAPIEncapsulator *device, FShaderEffect *owner)
:   FDeviceAPIDependantShaderEffect(device, owner) {
    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    const PCShaderProgram& vertexProgram = owner->StageProgram(EShaderProgramType::Vertex);
    if (vertexProgram) {
        Assert(vertexProgram->Available());
        Assert(owner->VertexDeclaration());
        Assert(owner->VertexDeclaration()->Available());

        const FDX11VertexDeclaration *dx11VertexDeclaration =
            checked_cast<FDX11VertexDeclaration *>(owner->VertexDeclaration()->DeviceAPIDependantDeclaration().get());

        const FDX11ShaderProgram *dx11ShaderProgram =
            checked_cast<FDX11ShaderProgram *>(vertexProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            DX11CreateInputLayout_( wrapper->Device(), dx11ShaderProgram->Compiled(), dx11VertexDeclaration, _inputLayout)
            ));

        Assert(_inputLayout);
        DX11SetDeviceResourceNameIFP(_inputLayout.Get(), owner->VertexDeclaration().get());

        _vertexShader.Reset(dx11ShaderProgram->VertexShader());
    }

    const PCShaderProgram& domainProgram = owner->StageProgram(EShaderProgramType::Domain);
    if (domainProgram) {
        Assert(domainProgram->Available());

        const FDX11ShaderProgram *dx11ShaderProgram =
            checked_cast<FDX11ShaderProgram *>(domainProgram->DeviceAPIDependantProgram().get());

        _domainShader.Reset(dx11ShaderProgram->DomainShader());
    }

    const PCShaderProgram& hullProgram = owner->StageProgram(EShaderProgramType::Hull);
    if (hullProgram) {
        Assert(hullProgram->Available());

        const FDX11ShaderProgram *dx11ShaderProgram =
            checked_cast<FDX11ShaderProgram *>(hullProgram->DeviceAPIDependantProgram().get());

        _hullShader.Reset(dx11ShaderProgram->HullShader());
    }

    const PCShaderProgram& geometryProgram = owner->StageProgram(EShaderProgramType::Geometry);
    if (geometryProgram) {
        Assert(geometryProgram->Available());

        const FDX11ShaderProgram *dx11ShaderProgram =
            checked_cast<FDX11ShaderProgram *>(geometryProgram->DeviceAPIDependantProgram().get());

        _geometryShader.Reset(dx11ShaderProgram->GeometryShader());
    }

    const PCShaderProgram& pixelProgram = owner->StageProgram(EShaderProgramType::Pixel);
    if (pixelProgram) {
        Assert(pixelProgram->Available());

        const FDX11ShaderProgram *dx11ShaderProgram =
            checked_cast<FDX11ShaderProgram *>(pixelProgram->DeviceAPIDependantProgram().get());

        _pixelShader.Reset(dx11ShaderProgram->PixelShader());
    }

    const PCShaderProgram& computeProgram = owner->StageProgram(EShaderProgramType::Compute);
    if (computeProgram) {
        Assert(computeProgram->Available());

        const FDX11ShaderProgram *dx11ShaderProgram =
            checked_cast<FDX11ShaderProgram *>(computeProgram->DeviceAPIDependantProgram().get());

        _computeShader.Reset(dx11ShaderProgram->ComputeShader());
    }
}
//----------------------------------------------------------------------------
FDX11ShaderEffect::~FDX11ShaderEffect() {
    ReleaseComRef(_inputLayout);
    ReleaseComRef(_vertexShader);
    ReleaseComRef(_domainShader);
    ReleaseComRef(_hullShader);
    ReleaseComRef(_geometryShader);
    ReleaseComRef(_domainShader);
    ReleaseComRef(_pixelShader);
    ReleaseComRef(_computeShader);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
