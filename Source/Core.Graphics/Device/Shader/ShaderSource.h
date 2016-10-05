#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Container/RawStorage.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class FShaderProgram;
class FVertexDeclaration;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderSource);
class FShaderSource : public FRefCountable {
public:
    FShaderSource(   const char *sourceName,
                    const Core::FFilename& filename,
                    const TMemoryView<const char>& sourceCode,
                    const TMemoryView<const TPair<FString, FString>>& defines);

    FShaderSource(   FString&& sourceName,
                    const Core::FFilename& filename,
                    RAWSTORAGE_THREAD_LOCAL(Shader, char)&& sourceCode,
                    ASSOCIATIVE_VECTOR_THREAD_LOCAL(Shader, FString, FString)&& defines);

    ~FShaderSource();

    FShaderSource(const FShaderSource& ) = delete;
    FShaderSource& operator =(const FShaderSource& ) = delete;

    const FString& SourceName() const { return _sourceName; }
    const Core::FFilename& Filename() const { return _filename; }
    TMemoryView<const char> SourceCode() const { return MakeView(_sourceCode); }
    TMemoryView<const TPair<FString, FString>> Defines() const { return MakeView(_defines); }

    void Preprocess(RAWSTORAGE_THREAD_LOCAL(Shader, char)& preprocessed,
                    const FShaderProgram *program,
                    const FVertexDeclaration *vertexDeclaration) const;

    void FillSubstitutions( VECTOR_THREAD_LOCAL(Shader, TPair<FString COMMA FString>)& substitutions,
                            const FVertexDeclaration *vertexDeclaration) const;

    static FShaderSource *LoadFromFileIFP(   const Core::FFilename& filename,
                                            const TMemoryView<const TPair<FString, FString>>& defines);

    SINGLETON_POOL_ALLOCATED_DECL();

    static FStringView AppIn_SubstitutionName();
    static FStringView AppIn_VertexDefinitionName();
    static FileSystem::FStringView SystemDirpath();

private:
    FString _sourceName;
    Core::FFilename _filename;
    RAWSTORAGE_THREAD_LOCAL(Shader, char) _sourceCode;
    ASSOCIATIVE_VECTOR_THREAD_LOCAL(Shader, FString, FString) _defines;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
