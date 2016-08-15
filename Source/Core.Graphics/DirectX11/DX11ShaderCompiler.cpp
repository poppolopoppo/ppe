#include "stdafx.h"

#include "DX11ShaderCompiler.h"

#include "Name.h"

#include "DirectX11/DX11CompilerIncludes.h"
#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Geometry/VertexDeclaration.h"
#include "Device/Shader/ShaderCompiled.h"
#include "Device/Shader/ShaderSource.h"
#include "Device/Shader/VertexSubstitutions.h"
#include "Device/State/SamplerState.h"

#include "Device/Shader/ConstantBufferLayout.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Allocator/ThreadLocalHeap.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/Stream.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Memory/HashFunctions.h"
#include "Core/Memory/UniqueView.h"

#include <sstream>

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class DX11ShaderIncludeHandler_ : public ::ID3DInclude {
public:
    DX11ShaderIncludeHandler_(  const Dirpath& systemDir,
                                const ShaderSource *source,
                                const VertexDeclaration *vertexDeclaration);
    virtual ~DX11ShaderIncludeHandler_();

    DX11ShaderIncludeHandler_(const DX11ShaderIncludeHandler_& ) = delete;
    DX11ShaderIncludeHandler_& operator =(const DX11ShaderIncludeHandler_& ) = delete;

    HRESULT STDCALL Open(::D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override;
    HRESULT STDCALL Close(LPCVOID pData) override;

private:
    void Open_(const Filename& filename, LPCVOID *ppData, UINT *pBytes);
    void GenerateAutomaticSubstitutions_(LPCVOID *ppData, UINT *pBytes);

    static void *Allocate_(size_t sizeInBytes) { return GetThreadLocalHeap().Malloc(sizeInBytes, MEMORY_DOMAIN_TRACKING_DATA(Shader)); }
    static void Deallocate_(void *ptr) { GetThreadLocalHeap().Free(ptr); }

    const Dirpath _systemDir;
    const ShaderSource *_source;
    const VertexDeclaration *_vertexDeclaration;
};
//----------------------------------------------------------------------------
DX11ShaderIncludeHandler_::DX11ShaderIncludeHandler_(
    const Dirpath& systemDir,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration )
:   _systemDir(systemDir)
,   _source(source)
,   _vertexDeclaration(vertexDeclaration) {
    Assert(VirtualFileSystem::Instance().DirectoryExists(_systemDir));
    Assert(source);
    Assert(vertexDeclaration);
}
//----------------------------------------------------------------------------
DX11ShaderIncludeHandler_::~DX11ShaderIncludeHandler_() {}
//----------------------------------------------------------------------------
HRESULT STDCALL DX11ShaderIncludeHandler_::Open(::D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID/* pParentData */, LPCVOID *ppData, UINT *pBytes) {
    *ppData = nullptr;
    *pBytes = 0;

    if (0 == CompareI(  MakeStringSlice(pFileName, Meta::noinit_tag()),
                        ShaderSource::AppIn_SubstitutionName()) ) {
        GenerateAutomaticSubstitutions_(ppData, pBytes);
        return S_OK;
    }

    FileSystem::char_type buffer[2048];
    const FileSystem::char_type sep = FileSystem::char_type(FileSystem::Separator);

    switch (IncludeType)
    {
    case ::D3D_INCLUDE_SYSTEM:
        Format(buffer, L"{0}{1}{2}", _systemDir, sep, pFileName);
        break;
    case ::D3D_INCLUDE_LOCAL:
        Format(buffer, L"{0}{1}{2}", _source->Filename().Dirpath(), sep, pFileName);
        break;
    default:
        AssertNotImplemented();
        return E_FAIL;
    }

    const Filename includeFilename(buffer);
    if (!VirtualFileSystem::Instance().FileExists(includeFilename))
        return E_FAIL;

    Open_(includeFilename, ppData, pBytes);
    return S_OK;
}
//----------------------------------------------------------------------------
HRESULT STDCALL DX11ShaderIncludeHandler_::Close(LPCVOID pData) {
    if (pData)
        Deallocate_(remove_const(pData));

    return S_OK;
}
//----------------------------------------------------------------------------
void DX11ShaderIncludeHandler_::Open_(const Filename& filename, LPCVOID *ppData, UINT *pBytes) {
    const auto file = VirtualFileSystem::Instance().OpenReadable(filename, AccessPolicy::Ate_Binary );
    AssertRelease(file);

    const size_t sizeInBytes = checked_cast<size_t>(file->TellI()); // Ate
    void *const storage = Allocate_(sizeInBytes);

    file->SeekI(0);
    file->Read(storage, sizeInBytes);

    *ppData = storage;
    *pBytes = checked_cast<UINT>(sizeInBytes);
}
//----------------------------------------------------------------------------
void DX11ShaderIncludeHandler_::GenerateAutomaticSubstitutions_(LPCVOID *ppData, UINT *pBytes) {
    VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>) substitutions;
    _source->FillSubstitutions(substitutions, _vertexDeclaration);

    ThreadLocalOStringStream oss;

    oss << "#ifndef __AppIn_SubstitutionName_INCLUDED" << std::endl
        << "#define __AppIn_SubstitutionName_INCLUDED" << std::endl
        << std::endl
        << "/* automatically generated by shader compiler */" << std::endl
        << std::endl;

    for (const Pair<String COMMA String>& s : substitutions)
        oss << "#define " << s.first << " " << s.second << std::endl;

    oss << std::endl
        << "/* end of automatically generated content */" << std::endl
        << std::endl
        << "#endif //!__AppIn_SubstitutionName_INCLUDED" << std::endl
        << std::endl;

    const size_t sizeInBytes = oss.str().size();
    void *const storage = Allocate_(sizeInBytes);

    ::memcpy(storage, oss.str().c_str(), sizeInBytes);

    *ppData = storage;
    *pBytes = checked_cast<UINT>(sizeInBytes);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static ValueType DX11VariableTypeToValueType_(const size_t rows, const size_t columns) {
    switch (columns)
    {
    case 1:
        switch (rows)
        {
        case 1: return ValueTraits< T >::TypeId;
        case 2: return ValueTraits< ScalarVector<T, 2> >::TypeId;
        case 3: return ValueTraits< ScalarVector<T, 3> >::TypeId;
        case 4: return ValueTraits< ScalarVector<T, 4> >::TypeId;
        }
        break;

    case 2:
        switch (rows)
        {
        case 1: return ValueTraits< ScalarMatrix<T, 1, 2> >::TypeId;
        case 2: return ValueTraits< ScalarMatrix<T, 2, 2> >::TypeId;
        case 3: return ValueTraits< ScalarMatrix<T, 3, 2> >::TypeId;
        case 4: return ValueTraits< ScalarMatrix<T, 4, 2> >::TypeId;
        }
        break;

    case 3:
        switch (rows)
        {
        case 1: return ValueTraits< ScalarMatrix<T, 1, 3> >::TypeId;
        case 2: return ValueTraits< ScalarMatrix<T, 2, 3> >::TypeId;
        case 3: return ValueTraits< ScalarMatrix<T, 3, 3> >::TypeId;
        case 4: return ValueTraits< ScalarMatrix<T, 4, 3> >::TypeId;
        }
        break;

    case 4:
        switch (rows)
        {
        case 1: return ValueTraits< ScalarMatrix<T, 1, 4> >::TypeId;
        case 2: return ValueTraits< ScalarMatrix<T, 2, 4> >::TypeId;
        case 3: return ValueTraits< ScalarMatrix<T, 3, 4> >::TypeId;
        case 4: return ValueTraits< ScalarMatrix<T, 4, 4> >::TypeId;
        }
        break;
    }

    AssertNotImplemented();
    return ValueType::Void;
}
//----------------------------------------------------------------------------
static ValueType DX11ShaderTypeToValueType_(const ::D3D11_SHADER_TYPE_DESC& desc) {
    Assert(::D3D10_SVC_STRUCT != desc.Class);

    const size_t rows = desc.Rows;
    const size_t columns = desc.Columns;

    ValueType result = ValueType::Void;

    switch (desc.Type)
    {
    case ::D3D10_SVT_FLOAT:
        result = DX11VariableTypeToValueType_<float>(rows, columns);
        break;
    case ::D3D10_SVT_INT:
        result = DX11VariableTypeToValueType_<i32>(rows, columns);
        break;
    case ::D3D10_SVT_UINT:
        result = DX11VariableTypeToValueType_<u32>(rows, columns);
        break;
    case ::D3D10_SVT_UINT8:
        result = DX11VariableTypeToValueType_<u8>(rows, columns);
        break;
    case ::D3D10_SVT_BOOL:
        result = DX11VariableTypeToValueType_<bool>(rows, columns);
        break;
    default:
        AssertNotImplemented();
        break;
    }

    AssertRelease(ValueType::Void != result);
    return result;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void DX11CompileShaderBlob_(
    ComPtr<::ID3DBlob>& compiled,
    IDeviceAPIShaderCompiler* compiler,
    const ShaderSource *source,
    const VertexDeclaration* vertexDeclaration,
    ShaderProgramType programType,
    ShaderProfileType profileType,
    ShaderCompilerFlags flags,
    const char* entryPoint ) {

    const MemoryView<const char> sourceCode = source->SourceCode();

    DX11ShaderIncludeHandler_ dx11Include(ShaderSource::SystemDirpath(), source, vertexDeclaration);
    const ::D3D_SHADER_MACRO dx11Defines[] = {
        {"DIRECTX11", "1"},
        {nullptr, nullptr},
    };

    const char *dx11Target = ShaderProfileTypeToD3D11Target(programType, profileType);
    const UINT dx11Compile = ShaderCompilerFlagsToD3D11CompileFlags(flags);

    ComPtr<::ID3DBlob> errors = nullptr;

    if ( FAILED(::D3DCompile(
                    sourceCode.Pointer(),
                    sourceCode.SizeInBytes(),
                    source->SourceName().c_str(),
                    dx11Defines,
                    &dx11Include,
                    entryPoint,
                    dx11Target,
                    dx11Compile,
                    0,
                    compiled.GetAddressOf(),
                    errors.GetAddressOf()
        )) ) {
        AssertRelease(errors);

        const char *msg = reinterpret_cast<const char *>(errors->GetBufferPointer());
        throw ShaderCompilerException(msg, compiler, source);
    }
    Assert(!errors);
}
//----------------------------------------------------------------------------
static void DX11ReflectShaderBlob_(
    ShaderCompiled::constants_type& constants,
    ShaderCompiled::textures_type& textures,
    IDeviceAPIShaderCompiler* compiler,
    const ShaderSource* source,
    const ComPtr<::ID3DBlob>& compiled
    ) {

    ::ID3D11ShaderReflection *dx11Reflector = nullptr;
    if (FAILED(::D3DReflect(
        compiled->GetBufferPointer(),
        compiled->GetBufferSize(),
        ::IID_ID3D11ShaderReflection,
        (void **)&dx11Reflector
        )) ) {
        throw ShaderCompilerException("failed to reflect shader program", compiler, source);
    }
    Assert(dx11Reflector);

    ::D3D11_SHADER_DESC dx11ShaderDesc;
    if (FAILED(dx11Reflector->GetDesc(&dx11ShaderDesc)))
        AssertNotReached();

    constants.reserve(dx11ShaderDesc.ConstantBuffers);
    textures.reserve(dx11ShaderDesc.BoundResources);

    for (UINT i = 0; i < dx11ShaderDesc.ConstantBuffers; ++i) {
        ::ID3D11ShaderReflectionConstantBuffer *dx11Reflector_constantBuffer = nullptr;
        dx11Reflector_constantBuffer = dx11Reflector->GetConstantBufferByIndex(i);
        Assert(dx11Reflector_constantBuffer);

        ::D3D11_SHADER_BUFFER_DESC dx11ConstantDesc;
        if (FAILED(dx11Reflector_constantBuffer->GetDesc(&dx11ConstantDesc)) )
            AssertNotReached();

        PConstantBufferLayout const layout = new ConstantBufferLayout();

        for (UINT j = 0; j < dx11ConstantDesc.Variables; ++j) {
            ::ID3D11ShaderReflectionVariable *dx11Reflector_variable = nullptr;
            dx11Reflector_variable = dx11Reflector_constantBuffer->GetVariableByIndex(j);
            Assert(dx11Reflector_variable);

            ::D3D11_SHADER_VARIABLE_DESC dx11VariableDesc;
            if (FAILED(dx11Reflector_variable->GetDesc(&dx11VariableDesc)) )
                AssertNotReached();

            const bool inUse = (::D3D_SVF_USED & dx11VariableDesc.uFlags) == ::D3D_SVF_USED;

            ::D3D11_SHADER_TYPE_DESC dx11TypeDesc;
            if (FAILED(dx11Reflector_variable->GetType()->GetDesc(&dx11TypeDesc)))
                AssertNotReached();

            const ValueType type = DX11ShaderTypeToValueType_(dx11TypeDesc);

            layout->AddField(   dx11VariableDesc.Name,
                                type,
                                dx11VariableDesc.StartOffset,
                                dx11VariableDesc.Size,
                                inUse );
        }

        Assert(layout->Block().size() > 0);
        AssertRelease(checked_cast<size_t>(dx11ConstantDesc.Size) == layout->Block().SizeInBytes());
        constants.Insert_AssertUnique(dx11ConstantDesc.Name, layout);
    }

    for (UINT i = 0; i < dx11ShaderDesc.BoundResources; ++i) {
        ::D3D11_SHADER_INPUT_BIND_DESC dx11ResourceDesc;
        if (FAILED(dx11Reflector->GetResourceBindingDesc(i, &dx11ResourceDesc)) )
            AssertNotReached();

        if (::D3D_SIT_TEXTURE != dx11ResourceDesc.Type)
            continue;

        AssertRelease(1 == dx11ResourceDesc.BindCount);

        ShaderProgramTexture texture;
        texture.Name = dx11ResourceDesc.Name;
        switch (dx11ResourceDesc.Dimension)
        {
        case ::D3D11_SRV_DIMENSION_TEXTURE2D:
            texture.Dimension = TextureDimension::Texture2D;
            break;
        case ::D3D11_SRV_DIMENSION_TEXTURE3D:
            texture.Dimension = TextureDimension::Texture3D;
            break;
        case ::D3D11_SRV_DIMENSION_TEXTURECUBE:
            texture.Dimension = TextureDimension::TextureCube;
            break;
        case ::D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
            texture.Dimension = TextureDimension::Texture2DArray;
            break;
        case ::D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
            texture.Dimension = TextureDimension::TextureCubeArray;
            break;
        default:
            AssertNotImplemented();
            break;
        }

        AssertRelease(dx11ResourceDesc.BindPoint == textures.size());
        textures.push_back(texture);
    }
}
//----------------------------------------------------------------------------
static void DX11StripShaderBlobIFN_(
    ComPtr<::ID3DBlob>& compiled,
    IDeviceAPIShaderCompiler* compiler,
    const ShaderSource *source,
    ShaderCompilerFlags flags ) {

    if (Meta::HasFlag(flags, ShaderCompilerFlags::NoOptimize) )
        return; // shader is not stripped when NoOptimize is set (debug)

    ComPtr<::ID3DBlob> striped = nullptr;

    if ( FAILED(::D3DStripShader(
                    compiled->GetBufferPointer(),
                    compiled->GetBufferSize(),
                    ::D3DCOMPILER_STRIP_REFLECTION_DATA |
                    ::D3DCOMPILER_STRIP_DEBUG_INFO |
                    ::D3DCOMPILER_STRIP_PRIVATE_DATA |
                    ::D3DCOMPILER_STRIP_TEST_BLOBS,
                    striped.GetAddressOf()
        )) ) {
        throw ShaderCompilerException("failed to strip shader program", compiler, source);
    }

    compiled = striped;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11ShaderCompiler::DX11ShaderCompiler() {}
//----------------------------------------------------------------------------
DX11ShaderCompiler::~DX11ShaderCompiler() {}
//----------------------------------------------------------------------------
ShaderCompiled* DX11ShaderCompiler::CompileShaderSource(
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration,
    ShaderProgramType programType,
    ShaderProfileType profileType,
    ShaderCompilerFlags flags,
    const char *entryPoint ) {

    ComPtr<::ID3DBlob> compiled;

    DX11CompileShaderBlob_(compiled, this, source, vertexDeclaration, programType, profileType, flags, entryPoint);
    Assert(compiled);

    ShaderCompiled::constants_type constants;
    ShaderCompiled::textures_type textures;
    DX11ReflectShaderBlob_(constants, textures, this, source, compiled);

    DX11StripShaderBlobIFN_(compiled, this, source, flags);
    Assert(compiled);

    ShaderCompiled::blob_type blob;
    blob.Resize_DiscardData(compiled->GetBufferSize());
    memcpy(blob.Pointer(), compiled->GetBufferPointer(), blob.SizeInBytes());

    const u64 fingerprint = Fingerprint64(blob.MakeConstView());

    return new ShaderCompiled(
        fingerprint,
        std::move(blob),
        std::move(constants),
        std::move(textures) );
}
//----------------------------------------------------------------------------
void DX11ShaderCompiler::PreprocessShaderSource(
    RAWSTORAGE(Shader, char)& output,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration ) {

    const MemoryView<const char> sourceCode = source->SourceCode();

    DX11ShaderIncludeHandler_ dx11Include(ShaderSource::SystemDirpath(), source, vertexDeclaration);
    const ::D3D_SHADER_MACRO dx11Defines[] = {
        {"DIRECTX11", "1"},
        {nullptr, nullptr},
    };

    ComPtr<::ID3DBlob> errors = nullptr;
    ComPtr<::ID3DBlob> preprocessed = nullptr;

    if ( FAILED(::D3DPreprocess(
                    sourceCode.Pointer(),
                    sourceCode.SizeInBytes(),
                    source->SourceName().c_str(),
                    dx11Defines,
                    &dx11Include,
                    preprocessed.GetAddressOf(),
                    errors.GetAddressOf()
        )) ) {
        AssertRelease(errors);

        const char *msg = reinterpret_cast<const char *>(errors->GetBufferPointer());
        throw ShaderCompilerException(msg, this, source);
    }
    Assert(!errors);
    Assert(preprocessed);

    output.Resize_DiscardData(preprocessed->GetBufferSize());
    memcpy(output.Pointer(), preprocessed->GetBufferPointer(), output.SizeInBytes());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UINT ShaderCompilerFlagsToD3D11CompileFlags(ShaderCompilerFlags value) {
    UINT result = 0;
    if (Meta::HasFlag(value, ShaderCompilerFlags::Debug) )
        result |= D3DCOMPILE_DEBUG;
    if (Meta::HasFlag(value, ShaderCompilerFlags::Optimize) )
        result |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    else if (Meta::HasFlag(value, ShaderCompilerFlags::NoOptimize) )
        result |= D3DCOMPILE_SKIP_OPTIMIZATION;
    if (Meta::HasFlag(value, ShaderCompilerFlags::Pedantic) )
        result |= D3DCOMPILE_ENABLE_STRICTNESS;
    if (Meta::HasFlag(value, ShaderCompilerFlags::WError) )
        result |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
    return result;
}
//----------------------------------------------------------------------------
ShaderCompilerFlags D3D11CompileFlagsToShaderCompilerFlags(UINT  value) {
    ShaderCompilerFlags result = ShaderCompilerFlags::None;
    if (D3DCOMPILE_DEBUG == (D3DCOMPILE_DEBUG & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::Debug));
    if (D3DCOMPILE_OPTIMIZATION_LEVEL3 == (D3DCOMPILE_OPTIMIZATION_LEVEL3 & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::Optimize));
    else if (D3DCOMPILE_SKIP_OPTIMIZATION == (D3DCOMPILE_SKIP_OPTIMIZATION & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::NoOptimize));
    if (D3DCOMPILE_ENABLE_STRICTNESS == (D3DCOMPILE_ENABLE_STRICTNESS & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::Pedantic));
    if (D3DCOMPILE_WARNINGS_ARE_ERRORS == (D3DCOMPILE_WARNINGS_ARE_ERRORS & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::WError));
    return result;
}
//----------------------------------------------------------------------------
LPCSTR ShaderProfileTypeToD3D11Target(ShaderProgramType program, ShaderProfileType profile) {
    switch (profile)
    {
    case Core::Graphics::ShaderProfileType::ShaderModel5:
        switch (program)
        {
        case Core::Graphics::ShaderProgramType::Vertex:
            return "vs_5_0";
        case Core::Graphics::ShaderProgramType::Hull:
            return "hs_5_0";
        case Core::Graphics::ShaderProgramType::Domain:
            return "ds_5_0";
        case Core::Graphics::ShaderProgramType::Pixel:
            return "ps_5_0";
        case Core::Graphics::ShaderProgramType::Geometry:
            return "gs_5_0";
        case Core::Graphics::ShaderProgramType::Compute:
            return "cs_5_0";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::ShaderProfileType::ShaderModel4_1:
        switch (program)
        {
        case Core::Graphics::ShaderProgramType::Vertex:
            return "vs_4_1";
        case Core::Graphics::ShaderProgramType::Pixel:
            return "ps_4_1";
        case Core::Graphics::ShaderProgramType::Geometry:
            return "gs_4_1";
        case Core::Graphics::ShaderProgramType::Compute:
            return "cs_4_1";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::ShaderProfileType::ShaderModel4:
        switch (program)
        {
        case Core::Graphics::ShaderProgramType::Vertex:
            return "vs_4_0";
        case Core::Graphics::ShaderProgramType::Pixel:
            return "ps_4_0";
        case Core::Graphics::ShaderProgramType::Geometry:
            return "gs_4_0";
        case Core::Graphics::ShaderProgramType::Compute:
            return "cs_4_0";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::ShaderProfileType::ShaderModel3:
        switch (program)
        {
        case Core::Graphics::ShaderProgramType::Vertex:
            return "vs_3_0";
        case Core::Graphics::ShaderProgramType::Pixel:
            return "ps_3_0";
        default:
            AssertNotImplemented();
        }
        break;
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core