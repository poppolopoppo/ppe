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
#include "Core/IO/StreamProvider.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Memory/HashFunctions.h"
#include "Core/Memory/UniqueView.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FDX11ShaderIncludeHandler_ : public ::ID3DInclude {
public:
    FDX11ShaderIncludeHandler_( const FDirpath& systemDir,
                                const FShaderSource *source,
                                const FVertexDeclaration *vertexDeclaration);
    virtual ~FDX11ShaderIncludeHandler_();

    FDX11ShaderIncludeHandler_(const FDX11ShaderIncludeHandler_& ) = delete;
    FDX11ShaderIncludeHandler_& operator =(const FDX11ShaderIncludeHandler_& ) = delete;

    HRESULT STDCALL Open(::D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override final;
    HRESULT STDCALL Close(LPCVOID pData) override final;

private:
    void Open_(const FFilename& filename, LPCVOID *ppData, UINT *pBytes);
    void GenerateAutomaticSubstitutions_(LPCVOID *ppData, UINT *pBytes);

    static void* Allocate_(size_t sizeInBytes) { return Alloca(sizeInBytes); }
    static void Deallocate_(void *ptr) { FreeAlloca(ptr); }

    const FDirpath _systemDir;
    const FShaderSource *_source;
    const FVertexDeclaration *_vertexDeclaration;
};
//----------------------------------------------------------------------------
FDX11ShaderIncludeHandler_::FDX11ShaderIncludeHandler_(
    const FDirpath& systemDir,
    const FShaderSource *source,
    const FVertexDeclaration *vertexDeclaration )
:   _systemDir(systemDir)
,   _source(source)
,   _vertexDeclaration(vertexDeclaration) {
    Assert(FVirtualFileSystem::Instance().DirectoryExists(_systemDir));
    Assert(source);
    Assert(vertexDeclaration);
}
//----------------------------------------------------------------------------
FDX11ShaderIncludeHandler_::~FDX11ShaderIncludeHandler_() {}
//----------------------------------------------------------------------------
HRESULT STDCALL FDX11ShaderIncludeHandler_::Open(::D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID/* pParentData */, LPCVOID *ppData, UINT *pBytes) {
    *ppData = nullptr;
    *pBytes = 0;

    FStringView filename(MakeCStringView(pFileName));
    if (0 == CompareI(filename, FShaderSource::AppIn_SubstitutionName()) ) {
        GenerateAutomaticSubstitutions_(ppData, pBytes);
        return S_OK;
    }

    size_t length = 0;
    FileSystem::char_type buffer[2048];
    FWFixedSizeTextWriter oss(buffer);

    switch (IncludeType)
    {
    case ::D3D_INCLUDE_SYSTEM:
        Format(oss, L"{0}{1}", _systemDir, filename);
        break;
    case ::D3D_INCLUDE_LOCAL:
        Format(oss, L"{0}{1}", _source->Filename().Dirpath(), filename);
        break;
    default:
        AssertNotImplemented();
        return E_FAIL;
    }

    const FFilename includeFilename(oss.Written());
    if (!FVirtualFileSystem::Instance().FileExists(includeFilename))
        return E_FAIL;

    Open_(includeFilename, ppData, pBytes);
    return S_OK;
}
//----------------------------------------------------------------------------
HRESULT STDCALL FDX11ShaderIncludeHandler_::Close(LPCVOID pData) {
    if (pData)
        Deallocate_(remove_const(pData));

    return S_OK;
}
//----------------------------------------------------------------------------
void FDX11ShaderIncludeHandler_::Open_(const FFilename& filename, LPCVOID *ppData, UINT *pBytes) {
    const auto file = FVirtualFileSystem::Instance().OpenReadable(filename, EAccessPolicy::Binary);
    AssertRelease(file);

    const size_t sizeInBytes = checked_cast<size_t>(file->SeekI(0, ESeekOrigin::End)); // Ate
    void *storage = nullptr;

    if (sizeInBytes) {
        storage = Allocate_(sizeInBytes);
        file->SeekI(0);
        file->Read(storage, sizeInBytes);
    }

    *ppData = storage;
    *pBytes = checked_cast<UINT>(sizeInBytes);
}
//----------------------------------------------------------------------------
void FDX11ShaderIncludeHandler_::GenerateAutomaticSubstitutions_(LPCVOID *ppData, UINT *pBytes) {
    VECTOR_THREAD_LOCAL(Shader, TPair<FString COMMA FString>) substitutions;

    _source->FillSubstitutions(substitutions, _vertexDeclaration);

    FStringBuilder oss;

    oss << "#ifndef __AppIn_SubstitutionName_INCLUDED" << Eol
        << "#define __AppIn_SubstitutionName_INCLUDED" << Eol
        << Eol
        << "/* automatically generated by shader compiler */" << Eol
        << Eol;

    for (const TPair<FString COMMA FString>& s : substitutions)
        oss << "#define " << s.first << " " << s.second << Eol;

    oss << Eol
        << "/* end of automatically generated content */" << Eol
        << Eol
        << "#endif //!__AppIn_SubstitutionName_INCLUDED" << Eol
        << Eol;

    oss.Writte
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
static EValueType DX11VariableTypeToValueType_(const size_t rows, const size_t columns) {
    switch (rows)
    {
    case 1:
        switch (columns)
        {
        case 1: return TValueTraits< T >::TypeId;
        case 2: return TValueTraits< TScalarVector<T, 2> >::TypeId;
        case 3: return TValueTraits< TScalarVector<T, 3> >::TypeId;
        case 4: return TValueTraits< TScalarVector<T, 4> >::TypeId;
        }
        break;

    case 2:
        switch (columns)
        {
        case 1: return TValueTraits< TScalarMatrix<T, 1, 2> >::TypeId;
        case 2: return TValueTraits< TScalarMatrix<T, 2, 2> >::TypeId;
        case 3: return TValueTraits< TScalarMatrix<T, 3, 2> >::TypeId;
        case 4: return TValueTraits< TScalarMatrix<T, 4, 2> >::TypeId;
        }
        break;

    case 3:
        switch (columns)
        {
        case 1: return TValueTraits< TScalarMatrix<T, 1, 3> >::TypeId;
        case 2: return TValueTraits< TScalarMatrix<T, 2, 3> >::TypeId;
        case 3: return TValueTraits< TScalarMatrix<T, 3, 3> >::TypeId;
        case 4: return TValueTraits< TScalarMatrix<T, 4, 3> >::TypeId;
        }
        break;

    case 4:
        switch (columns)
        {
        case 1: return TValueTraits< TScalarMatrix<T, 1, 4> >::TypeId;
        case 2: return TValueTraits< TScalarMatrix<T, 2, 4> >::TypeId;
        case 3: return TValueTraits< TScalarMatrix<T, 3, 4> >::TypeId;
        case 4: return TValueTraits< TScalarMatrix<T, 4, 4> >::TypeId;
        }
        break;
    }

    AssertNotImplemented();
    return EValueType::Void;
}
//----------------------------------------------------------------------------
static EValueType DX11ShaderTypeToValueType_(const ::D3D11_SHADER_TYPE_DESC& desc) {
    Assert(::D3D10_SVC_STRUCT != desc.Class);

    const size_t rows = desc.Rows;
    const size_t columns = desc.Columns;

    EValueType result = EValueType::Void;

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

    AssertRelease(EValueType::Void != result);
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
    TComPtr<::ID3DBlob>& compiled,
    IDeviceAPIShaderCompiler* compiler,
    const FShaderSource *source,
    const FVertexDeclaration* vertexDeclaration,
    EShaderProgramType programType,
    EShaderProfileType profileType,
    EShaderCompilerFlags flags,
    const char* entryPoint ) {

    const TMemoryView<const char> sourceCode = source->SourceCode();

    FDX11ShaderIncludeHandler_ dx11Include(FShaderSource::SystemDirpath(), source, vertexDeclaration);
    const ::D3D_SHADER_MACRO dx11Defines[] = {
        {"DIRECTX11", "1"},
        {nullptr, nullptr},
    };

    const char *dx11Target = ShaderProfileTypeToD3D11Target(programType, profileType);
    const UINT dx11Compile = ShaderCompilerFlagsToD3D11CompileFlags(flags);

    TComPtr<::ID3DBlob> errors = nullptr;

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
        CORE_THROW_IT(FShaderCompilerException(msg, compiler, source));
    }
    Assert(!errors);
}
//----------------------------------------------------------------------------
static void DX11ReflectShaderBlob_(
    FShaderCompiled::constants_type& constants,
    FShaderCompiled::textures_type& textures,
    IDeviceAPIShaderCompiler* compiler,
    const FShaderSource* source,
    const TComPtr<::ID3DBlob>& compiled
    ) {

    ::ID3D11ShaderReflection *dx11Reflector = nullptr;
    if (FAILED(::D3DReflect(
        compiled->GetBufferPointer(),
        compiled->GetBufferSize(),
        ::IID_ID3D11ShaderReflection,
        (void **)&dx11Reflector
        )) ) {
        CORE_THROW_IT(FShaderCompilerException("failed to reflect shader program", compiler, source));
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

        PConstantBufferLayout const layout = new FConstantBufferLayout();

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

            const EValueType type = DX11ShaderTypeToValueType_(dx11TypeDesc);

            layout->AddField(   FName(MakeCStringView(dx11VariableDesc.Name)),
                                type,
                                dx11VariableDesc.StartOffset,
                                dx11VariableDesc.Size,
                                inUse );
        }

        Assert(layout->Block().size() > 0);
        AssertRelease(checked_cast<size_t>(dx11ConstantDesc.Size) == layout->Block().SizeInBytes());
        constants.Insert_AssertUnique(FName(MakeCStringView(dx11ConstantDesc.Name)), layout);
    }

    for (UINT i = 0; i < dx11ShaderDesc.BoundResources; ++i) {
        ::D3D11_SHADER_INPUT_BIND_DESC dx11ResourceDesc;
        if (FAILED(dx11Reflector->GetResourceBindingDesc(i, &dx11ResourceDesc)) )
            AssertNotReached();

        if (::D3D_SIT_TEXTURE != dx11ResourceDesc.Type)
            continue;

        AssertRelease(1 == dx11ResourceDesc.BindCount);

        FShaderProgramTexture texture;
        texture.Name = MakeCStringView(dx11ResourceDesc.Name);

        switch (dx11ResourceDesc.Dimension)
        {
        case ::D3D11_SRV_DIMENSION_TEXTURE2D:
            texture.Dimension = ETextureDimension::FTexture2D;
            break;
        case ::D3D11_SRV_DIMENSION_TEXTURE3D:
            texture.Dimension = ETextureDimension::Texture3D;
            break;
        case ::D3D11_SRV_DIMENSION_TEXTURECUBE:
            texture.Dimension = ETextureDimension::FTextureCube;
            break;
        case ::D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
            texture.Dimension = ETextureDimension::Texture2DArray;
            break;
        case ::D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
            texture.Dimension = ETextureDimension::TextureCubeArray;
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
    TComPtr<::ID3DBlob>& compiled,
    IDeviceAPIShaderCompiler* compiler,
    const FShaderSource *source,
    EShaderCompilerFlags flags ) {

    if (not (flags ^ EShaderCompilerFlags::StripDebug))
        return; // shader is not stripped when StripDebug is not set

    TComPtr<::ID3DBlob> striped = nullptr;

    if ( FAILED(::D3DStripShader(
                    compiled->GetBufferPointer(),
                    compiled->GetBufferSize(),
                    ::D3DCOMPILER_STRIP_REFLECTION_DATA |
                    ::D3DCOMPILER_STRIP_DEBUG_INFO |
                    ::D3DCOMPILER_STRIP_PRIVATE_DATA |
                    ::D3DCOMPILER_STRIP_TEST_BLOBS,
                    striped.GetAddressOf()
        )) ) {
        CORE_THROW_IT(FShaderCompilerException("failed to strip shader program", compiler, source));
    }

    compiled = striped;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11ShaderCompiler::FDX11ShaderCompiler() {}
//----------------------------------------------------------------------------
FDX11ShaderCompiler::~FDX11ShaderCompiler() {}
//----------------------------------------------------------------------------
FShaderCompiled* FDX11ShaderCompiler::CompileShaderSource(
    const FShaderSource *source,
    const FVertexDeclaration *vertexDeclaration,
    EShaderProgramType programType,
    EShaderProfileType profileType,
    EShaderCompilerFlags flags,
    const char *entryPoint ) {

    TComPtr<::ID3DBlob> compiled;

    DX11CompileShaderBlob_(compiled, this, source, vertexDeclaration, programType, profileType, flags, entryPoint);
    Assert(compiled);

    FShaderCompiled::constants_type constants;
    FShaderCompiled::textures_type textures;
    DX11ReflectShaderBlob_(constants, textures, this, source, compiled);

    DX11StripShaderBlobIFN_(compiled, this, source, flags);
    Assert(compiled);

    FShaderCompiled::blob_type blob;
    blob.Resize_DiscardData(compiled->GetBufferSize());
    memcpy(blob.Pointer(), compiled->GetBufferPointer(), blob.SizeInBytes());

    const u64 fingerprint = Fingerprint64(blob.MakeConstView());

    return new FShaderCompiled(
        fingerprint,
        std::move(blob),
        std::move(constants),
        std::move(textures) );
}
//----------------------------------------------------------------------------
void FDX11ShaderCompiler::PreprocessShaderSource(
    RAWSTORAGE(Shader, char)& output,
    const FShaderSource *source,
    const FVertexDeclaration *vertexDeclaration ) {

    const TMemoryView<const char> sourceCode = source->SourceCode();

    FDX11ShaderIncludeHandler_ dx11Include(FShaderSource::SystemDirpath(), source, vertexDeclaration);
    const ::D3D_SHADER_MACRO dx11Defines[] = {
        {"DIRECTX11", "1"},
        {nullptr, nullptr},
    };

    TComPtr<::ID3DBlob> errors = nullptr;
    TComPtr<::ID3DBlob> preprocessed = nullptr;

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
        CORE_THROW_IT(FShaderCompilerException(msg, this, source));
    }
    Assert(!errors);
    Assert(preprocessed);

    output.Resize_DiscardData(preprocessed->GetBufferSize());
    memcpy(output.Pointer(), preprocessed->GetBufferPointer(), output.SizeInBytes());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UINT ShaderCompilerFlagsToD3D11CompileFlags(EShaderCompilerFlags value) {
    UINT result = 0;
    if (value ^ EShaderCompilerFlags::Debug)
        result |= D3DCOMPILE_DEBUG;
    if (value ^ EShaderCompilerFlags::Optimize)
        result |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    else if (value ^ EShaderCompilerFlags::NoOptimize)
        result |= D3DCOMPILE_SKIP_OPTIMIZATION;
    if (value ^ EShaderCompilerFlags::Pedantic)
        result |= D3DCOMPILE_ENABLE_STRICTNESS;
    if (value ^ EShaderCompilerFlags::WError)
        result |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
    if (value ^ EShaderCompilerFlags::IEEEStrict)
        result |= D3DCOMPILE_IEEE_STRICTNESS;
    return result;
}
//----------------------------------------------------------------------------
EShaderCompilerFlags D3D11CompileFlagsToShaderCompilerFlags(UINT  value) {
    EShaderCompilerFlags result = EShaderCompilerFlags::None;
    if (D3DCOMPILE_DEBUG == (D3DCOMPILE_DEBUG & value) )
        result = result + EShaderCompilerFlags::Debug;
    if (D3DCOMPILE_OPTIMIZATION_LEVEL3 == (D3DCOMPILE_OPTIMIZATION_LEVEL3 & value) )
        result = result + EShaderCompilerFlags::Optimize;
    else if (D3DCOMPILE_SKIP_OPTIMIZATION == (D3DCOMPILE_SKIP_OPTIMIZATION & value) )
        result = result + EShaderCompilerFlags::NoOptimize;
    if (D3DCOMPILE_ENABLE_STRICTNESS == (D3DCOMPILE_ENABLE_STRICTNESS & value) )
        result = result + EShaderCompilerFlags::Pedantic;
    if (D3DCOMPILE_WARNINGS_ARE_ERRORS == (D3DCOMPILE_WARNINGS_ARE_ERRORS & value) )
        result = result + EShaderCompilerFlags::WError;
    if (D3DCOMPILE_IEEE_STRICTNESS == (D3DCOMPILE_IEEE_STRICTNESS & value))
        result = result + EShaderCompilerFlags::IEEEStrict;
    return result;
}
//----------------------------------------------------------------------------
LPCSTR ShaderProfileTypeToD3D11Target(EShaderProgramType program, EShaderProfileType profile) {
    switch (profile)
    {
    case Core::Graphics::EShaderProfileType::ShaderModel5:
        switch (program)
        {
        case Core::Graphics::EShaderProgramType::Vertex:
            return "vs_5_0";
        case Core::Graphics::EShaderProgramType::Hull:
            return "hs_5_0";
        case Core::Graphics::EShaderProgramType::Domain:
            return "ds_5_0";
        case Core::Graphics::EShaderProgramType::Pixel:
            return "ps_5_0";
        case Core::Graphics::EShaderProgramType::Geometry:
            return "gs_5_0";
        case Core::Graphics::EShaderProgramType::Compute:
            return "cs_5_0";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::EShaderProfileType::ShaderModel4_1:
        switch (program)
        {
        case Core::Graphics::EShaderProgramType::Vertex:
            return "vs_4_1";
        case Core::Graphics::EShaderProgramType::Pixel:
            return "ps_4_1";
        case Core::Graphics::EShaderProgramType::Geometry:
            return "gs_4_1";
        case Core::Graphics::EShaderProgramType::Compute:
            return "cs_4_1";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::EShaderProfileType::ShaderModel4:
        switch (program)
        {
        case Core::Graphics::EShaderProgramType::Vertex:
            return "vs_4_0";
        case Core::Graphics::EShaderProgramType::Pixel:
            return "ps_4_0";
        case Core::Graphics::EShaderProgramType::Geometry:
            return "gs_4_0";
        case Core::Graphics::EShaderProgramType::Compute:
            return "cs_4_0";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::EShaderProfileType::ShaderModel3:
        switch (program)
        {
        case Core::Graphics::EShaderProgramType::Vertex:
            return "vs_3_0";
        case Core::Graphics::EShaderProgramType::Pixel:
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
