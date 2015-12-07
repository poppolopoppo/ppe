#include "stdafx.h"

#include "DX11ShaderEffect.h"

#include "DX11ShaderProgram.h"
#include "DirectX11/DX11DeviceAPIEncapsulator.h"
#include "DirectX11/Geometry/DX11VertexDeclaration.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Shader/ShaderCompiled.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Memory/UniqueView.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static HRESULT DX11CreateInputLayout_(
    ::ID3D11Device *device,
    const ShaderCompiled* compiled,
    const DX11VertexDeclaration *vertexDeclaration,
    ComPtr<::ID3D11InputLayout>& inputLayout ) {
    const MemoryView<const ::D3D11_INPUT_ELEMENT_DESC> layout = vertexDeclaration->Layout();

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
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, DX11ShaderEffect, );
//----------------------------------------------------------------------------
DX11ShaderEffect::DX11ShaderEffect(IDeviceAPIEncapsulator *device, ShaderEffect *owner)
:   DeviceAPIDependantShaderEffect(device, owner) {
    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    const PCShaderProgram& vertexProgram = owner->StageProgram(ShaderProgramType::Vertex);
    if (vertexProgram) {
        Assert(vertexProgram->Available());
        Assert(owner->VertexDeclaration());
        Assert(owner->VertexDeclaration()->Available());

        const DX11VertexDeclaration *dx11VertexDeclaration =
            checked_cast<DX11VertexDeclaration *>(owner->VertexDeclaration()->DeviceAPIDependantDeclaration().get());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(vertexProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            DX11CreateInputLayout_( wrapper->Device(), dx11ShaderProgram->Compiled(), dx11VertexDeclaration, _inputLayout)
            ));

        Assert(_inputLayout);
        DX11SetDeviceResourceNameIFP(_inputLayout, owner->VertexDeclaration().get());

        _vertexShader = dx11ShaderProgram->VertexShader();
    }

    const PCShaderProgram& domainProgram = owner->StageProgram(ShaderProgramType::Domain);
    if (domainProgram) {
        Assert(domainProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(domainProgram->DeviceAPIDependantProgram().get());

        _domainShader = dx11ShaderProgram->DomainShader();
    }

    const PCShaderProgram& hullProgram = owner->StageProgram(ShaderProgramType::Hull);
    if (hullProgram) {
        Assert(hullProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(hullProgram->DeviceAPIDependantProgram().get());

        _hullShader = dx11ShaderProgram->HullShader();
    }

    const PCShaderProgram& geometryProgram = owner->StageProgram(ShaderProgramType::Geometry);
    if (geometryProgram) {
        Assert(geometryProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(geometryProgram->DeviceAPIDependantProgram().get());

        _geometryShader = dx11ShaderProgram->GeometryShader();
    }

    const PCShaderProgram& pixelProgram = owner->StageProgram(ShaderProgramType::Pixel);
    if (pixelProgram) {
        Assert(pixelProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(pixelProgram->DeviceAPIDependantProgram().get());

        _pixelShader = dx11ShaderProgram->PixelShader();
    }

    const PCShaderProgram& computeProgram = owner->StageProgram(ShaderProgramType::Compute);
    if (computeProgram) {
        Assert(computeProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(computeProgram->DeviceAPIDependantProgram().get());

        _computeShader = dx11ShaderProgram->ComputeShader();
    }
}
//----------------------------------------------------------------------------
DX11ShaderEffect::~DX11ShaderEffect() {
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
} //!namespace Core
