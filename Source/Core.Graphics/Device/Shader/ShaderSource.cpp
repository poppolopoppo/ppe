#include "stdafx.h"

#include "ShaderSource.h"

#include "Device/Geometry/VertexDeclaration.h"
#include "ShaderProgram.h"
#include "VertexSubstitutions.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"
#include "Core/IO/VirtualFileSystem.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(ShaderSource, );
//----------------------------------------------------------------------------
const char *ShaderSource::AppIn_SubstitutionName() { return "__AppIn_AutoSubstitutionHeader__"; }
const char *ShaderSource::AppIn_VertexDefinitionName() { return "__AppIn_AutoVertexDefinition__"; }
const FileSystem::char_type *ShaderSource::SystemDirpath() { return L"GameData:/Shaders/Lib"; }
//----------------------------------------------------------------------------
ShaderSource::ShaderSource( const char *sourceName,
                            const Core::Filename& filename,
                            const MemoryView<const char>& sourceCode,
                            const MemoryView<const Pair<String, String>>& defines)
:   _sourceName(sourceName)
,   _filename(filename)
,   _defines(defines.begin(), defines.end()) {
    Assert(sourceName);
    Assert(sourceCode.size());

    _sourceCode.Resize_DiscardData(sourceCode.SizeInBytes());
    memcpy(_sourceCode.Pointer(), sourceCode.Pointer(), _sourceCode.SizeInBytes());
}
//----------------------------------------------------------------------------
ShaderSource::ShaderSource( String&& sourceName,
                            const Core::Filename& filename,
                            RAWSTORAGE_THREAD_LOCAL(Shader, char)&& sourceCode,
                            ASSOCIATIVE_VECTOR_THREAD_LOCAL(Shader, String, String)&& defines)
:   _sourceName(std::move(sourceName))
,   _filename(filename)
,   _sourceCode(std::move(sourceCode))
,   _defines(std::move(defines)) {
    Assert(_sourceName.size());
    Assert(_sourceCode.size());
}
//----------------------------------------------------------------------------
ShaderSource::~ShaderSource() {}
//----------------------------------------------------------------------------
void ShaderSource::Preprocess(
    RAWSTORAGE_THREAD_LOCAL(Shader, char)& preprocessed,
    const ShaderProgram *program,
    const VertexDeclaration *vertexDeclaration) const {
    Assert(_sourceCode.size());

    VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>) substitutions;
    substitutions.reserve(VertexDeclaration::MaxSubPartCount + _defines.size());
    substitutions.insert(substitutions.end(), _defines.begin(), _defines.end());
    FillVertexSubstitutions(substitutions, vertexDeclaration);

    const char expandDefine[] = "#define ";
    const char expandHeader[] = "/******* Begin shader source expansion *******/\n";
    const char expandFooter[] = "/*******  End shader source expansion  *******/\n";

    size_t expandSize = lengthof(expandHeader) + lengthof(expandFooter) - 2 /* remove final '\0' */;
    for (const Pair<String, String>& substitution : substitutions)
        expandSize +=   lengthof(expandDefine) - 1 +
                        substitution.first.size() +
                        substitution.second.size() +
                        1 /* ' ' */ +
                        1 /* '\n' */;

    preprocessed.Resize_DiscardData(expandSize + _sourceCode.size());

    char *outb = preprocessed.Pointer();

    memcpy(outb, expandHeader, sizeof(expandHeader) - 1);
    outb += lengthof(expandHeader) - 1;

    for (const Pair<String, String>& substitution : substitutions) {
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

    Assert(outb - preprocessed.Pointer() == preprocessed.size());
}
//----------------------------------------------------------------------------
void ShaderSource::FillSubstitutions(
    VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>)& substitutions,
    const VertexDeclaration *vertexDeclaration ) const {
    Assert(vertexDeclaration);

    substitutions.reserve(VertexDeclaration::MaxSubPartCount + _defines.size());
    substitutions.insert(substitutions.end(), _defines.begin(), _defines.end());
    FillVertexSubstitutions(substitutions, vertexDeclaration);
}
//----------------------------------------------------------------------------
ShaderSource *ShaderSource::LoadFromFile(   const Core::Filename& filename,
                                            const MemoryView<const Pair<String, String>>& defines) {
    Assert(!filename.empty());

    RAWSTORAGE_THREAD_LOCAL(Shader, char) sourceCode;
    VirtualFileSystem::Instance().ReadAll(filename, sourceCode);

    ASSOCIATIVE_VECTOR_THREAD_LOCAL(Shader, String, String) sourceDefines(defines.size());
    sourceDefines.insert(defines.begin(), defines.end());

    const WString nativeFilenameW = VirtualFileSystem::Instance().Unalias(filename);

    return new ShaderSource(ToString(nativeFilenameW), filename, std::move(sourceCode), std::move(sourceDefines));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
