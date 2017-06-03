#include "stdafx.h"

#include "ShaderSource.h"

#include "Device/Geometry/VertexDeclaration.h"
#include "ShaderProgram.h"
#include "VertexSubstitutions.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/StringView.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"
#include "Core/IO/VirtualFileSystem.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FShaderSource, );
//----------------------------------------------------------------------------
FStringView FShaderSource::AppIn_SubstitutionName() { return MakeStringView("__AppIn_AutoSubstitutionHeader__"); }
FStringView FShaderSource::AppIn_VertexDefinitionName() { return MakeStringView("__AppIn_AutoVertexDefinition__"); }
FileSystem::FStringView FShaderSource::SystemDirpath() { return MakeStringView(L"Data:/Shaders/Lib"); }
//----------------------------------------------------------------------------
FShaderSource::FShaderSource( const char *sourceName,
                            const Core::FFilename& filename,
                            const TMemoryView<const char>& sourceCode,
                            const TMemoryView<const TPair<FString, FString>>& defines)
:   _sourceName(sourceName)
,   _filename(filename)
,   _defines(defines.begin(), defines.end()) {
    Assert(sourceName);
    Assert(sourceCode.size());

    _sourceCode.Resize_DiscardData(sourceCode.SizeInBytes());
    memcpy(_sourceCode.Pointer(), sourceCode.Pointer(), _sourceCode.SizeInBytes());
}
//----------------------------------------------------------------------------
FShaderSource::FShaderSource( FString&& sourceName,
                            const Core::FFilename& filename,
                            RAWSTORAGE_THREAD_LOCAL(Shader, char)&& sourceCode,
                            ASSOCIATIVE_VECTOR_THREAD_LOCAL(Shader, FString, FString)&& defines)
:   _sourceName(std::move(sourceName))
,   _filename(filename)
,   _sourceCode(std::move(sourceCode))
,   _defines(std::move(defines)) {
    Assert(_sourceName.size());
    Assert(_sourceCode.size());
}
//----------------------------------------------------------------------------
FShaderSource::~FShaderSource() {}
//----------------------------------------------------------------------------
void FShaderSource::Preprocess(
    RAWSTORAGE_THREAD_LOCAL(Shader, char)& preprocessed,
    const FShaderProgram * /* program */,
    const FVertexDeclaration *vertexDeclaration) const {
    Assert(_sourceCode.size());

    VECTOR_THREAD_LOCAL(Shader, TPair<FString COMMA FString>) substitutions;
    substitutions.reserve(FVertexDeclaration::MaxSubPartCount + _defines.size());
    substitutions.insert(substitutions.end(), _defines.begin(), _defines.end());
    FillVertexSubstitutions(substitutions, vertexDeclaration);

    const char expandDefine[] = "#define ";
    const char expandHeader[] = "/******* Begin shader source expansion *******/\n";
    const char expandFooter[] = "/*******  End shader source expansion  *******/\n";

    size_t expandSize = lengthof(expandHeader) + lengthof(expandFooter) - 2 /* remove final '\0' */;
    for (const TPair<FString, FString>& substitution : substitutions)
        expandSize +=   lengthof(expandDefine) - 1 +
                        substitution.first.size() +
                        substitution.second.size() +
                        1 /* ' ' */ +
                        1 /* '\n' */;

    preprocessed.Resize_DiscardData(expandSize + _sourceCode.size());

    char *outb = preprocessed.Pointer();

    memcpy(outb, expandHeader, sizeof(expandHeader) - 1);
    outb += lengthof(expandHeader) - 1;

    for (const TPair<FString, FString>& substitution : substitutions) {
        memcpy(outb, expandDefine, sizeof(expandDefine) - 1);
        outb += lengthof(expandDefine) - 1;

        memcpy(outb, substitution.first.c_str(), substitution.first.size());
        outb += substitution.first.size();

        *outb++ = ' ';

        memcpy(outb, substitution.second.c_str(), substitution.second.size());
        outb += substitution.second.size();

        *outb++ = '\n';
    }

    memcpy(outb, expandFooter, sizeof(expandFooter) - 1);
    outb += sizeof(expandFooter) - 1;

    memcpy(outb, _sourceCode.Pointer(), _sourceCode.SizeInBytes());
    outb += _sourceCode.SizeInBytes();

    Assert(outb - preprocessed.Pointer() == intptr_t(preprocessed.size()) );
}
//----------------------------------------------------------------------------
void FShaderSource::FillSubstitutions(
    VECTOR_THREAD_LOCAL(Shader, TPair<FString COMMA FString>)& substitutions,
    const FVertexDeclaration *vertexDeclaration ) const {
    Assert(vertexDeclaration);

    substitutions.reserve(FVertexDeclaration::MaxSubPartCount + _defines.size());
    substitutions.insert(substitutions.end(), _defines.begin(), _defines.end());
    FillVertexSubstitutions(substitutions, vertexDeclaration);
}
//----------------------------------------------------------------------------
FShaderSource *FShaderSource::LoadFromFileIFP(const Core::FFilename& filename,
                                            const TMemoryView<const TPair<FString, FString>>& defines) {
    Assert(!filename.empty());

    RAWSTORAGE_THREAD_LOCAL(Shader, char) sourceCode;
    if (false == FVirtualFileSystem::ReadAll(filename, sourceCode, EAccessPolicy::Binary))
        return nullptr;

    ASSOCIATIVE_VECTOR_THREAD_LOCAL(Shader, FString, FString) sourceDefines(defines.size());
    sourceDefines.insert(defines.begin(), defines.end());

    const FWString nativeFilenameW = FVirtualFileSystem::Instance().Unalias(filename);

    return new FShaderSource(ToString(nativeFilenameW), filename, std::move(sourceCode), std::move(sourceDefines));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
