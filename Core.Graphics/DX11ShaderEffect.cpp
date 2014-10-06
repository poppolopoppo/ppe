#include "stdafx.h"

#include "DX11ShaderEffect.h"

#include "DX11DeviceEncapsulator.h"
#include "DX11ShaderProgram.h"
#include "DX11VertexDeclaration.h"

#include "DeviceAPIEncapsulator.h"
#include "DeviceEncapsulatorException.h"

#include "Core/UniqueView.h"
#include "Core/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static HRESULT CreateInputLayout_(
    ::ID3D11Device *device,
    ::ID3DBlob *vertexShaderBlob,
    const VertexDeclaration *vertexDeclaration,
    ComPtr<ID3D11InputLayout>& inputLayout) {
    const MemoryView<const ::D3D11_INPUT_ELEMENT_DESC> layout = vertexDeclaration->Layout();

    return device->CreateInputLayout(
        layout.Pointer(),
        checked_cast<UINT>(layout.size()),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        inputLayout.GetAddressOf()
        );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderEffect::ShaderEffect(IDeviceAPIEncapsulator *device, Graphics::ShaderEffect *owner)
:   DeviceAPIDependantShaderEffect(device, owner) {
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    const Graphics::PCShaderProgram& vertexProgram = owner->StageProgram(ShaderProgramType::Vertex);
    if (vertexProgram) {
        Assert(vertexProgram->Available());
        Assert(owner->VertexDeclaration());
        Assert(owner->VertexDeclaration()->Available());

        const DX11::VertexDeclaration *dx11VertexDeclaration =
            checked_cast<DX11::VertexDeclaration *>(owner->VertexDeclaration()->DeviceAPIDependantDeclaration().get());

        const DX11::ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11::ShaderProgram *>(vertexProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            CreateInputLayout_( wrapper->Device(),
                                dx11ShaderProgram->Entity(),
                                dx11VertexDeclaration,
                                _inputLayout)
            ));

        Assert(_inputLayout);
        DX11SetDeviceResourceNameIFP(_inputLayout, owner->VertexDeclaration());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateVertexShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _vertexShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_vertexShader, vertexProgram);
    }

    const Graphics::PCShaderProgram& domainProgram = owner->StageProgram(ShaderProgramType::Domain);
    if (domainProgram) {
        Assert(domainProgram->Available());

        const DX11::ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11::ShaderProgram *>(domainProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateDomainShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _domainShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_domainShader, domainProgram);
    }

    const Graphics::PCShaderProgram& hullProgram = owner->StageProgram(ShaderProgramType::Hull);
    if (hullProgram) {
        Assert(hullProgram->Available());

        const DX11::ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11::ShaderProgram *>(hullProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateHullShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _hullShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_hullShader, hullProgram);
    }

    const Graphics::PCShaderProgram& geometryProgram = owner->StageProgram(ShaderProgramType::Geometry);
    if (geometryProgram) {
        Assert(geometryProgram->Available());

        const DX11::ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11::ShaderProgram *>(geometryProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateGeometryShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _geometryShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_geometryShader, geometryProgram);
    }

    const Graphics::PCShaderProgram& pixelProgram = owner->StageProgram(ShaderProgramType::Pixel);
    if (pixelProgram) {
        Assert(pixelProgram->Available());

        const DX11::ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11::ShaderProgram *>(pixelProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreatePixelShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _pixelShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_pixelShader, pixelProgram);
    }

    const Graphics::PCShaderProgram& computeProgram = owner->StageProgram(ShaderProgramType::Compute);
    if (computeProgram) {
        Assert(computeProgram->Available());

        const DX11::ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11::ShaderProgram *>(computeProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateComputeShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _computeShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_computeShader, computeProgram);
    }
}
//----------------------------------------------------------------------------
ShaderEffect::~ShaderEffect() {
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
SINGLETON_POOL_ALLOCATED_DEF(ShaderEffect, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core