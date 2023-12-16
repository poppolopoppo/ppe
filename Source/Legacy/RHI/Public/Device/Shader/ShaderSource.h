#pragma once

#include "Graphics.h"

#include "Allocator/PoolAllocator.h"
#include "Container/AssociativeVector.h"
#include "Container/Pair.h"
#include "Container/RawStorage.h"
#include "IO/FS/Filename.h"
#include "IO/String.h"
#include "Memory/MemoryView.h"
#include "Memory/RefPtr.h"

namespace PPE {
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
                    const PPE::FFilename& filename,
                    const TMemoryView<const char>& sourceCode,
                    const TMemoryView<const TPair<FString, FString>>& defines);

    FShaderSource(   FString&& sourceName,
                    const PPE::FFilename& filename,
                    RAWSTORAGE(Shader, char)&& sourceCode,
                    ASSOCIATIVE_VECTOR(Shader, FString, FString)&& defines);

    ~FShaderSource();

    FShaderSource(const FShaderSource& ) = delete;
    FShaderSource& operator =(const FShaderSource& ) = delete;

    const FString& SourceName() const { return _sourceName; }
    const PPE::FFilename& Filename() const { return _filename; }
    TMemoryView<const char> SourceCode() const { return MakeView(_sourceCode); }
    TMemoryView<const TPair<FString, FString>> Defines() const { return MakeView(_defines); }

    void Preprocess(RAWSTORAGE(Shader, char)& preprocessed,
                    const FShaderProgram *program,
                    const FVertexDeclaration *vertexDeclaration) const;

    void FillSubstitutions( VECTOR(Shader, TPair<FString COMMA FString>)& substitutions,
                            const FVertexDeclaration *vertexDeclaration) const;

    static FShaderSource *LoadFromFileIFP(  const PPE::FFilename& filename,
                                            const TMemoryView<const TPair<FString, FString>>& defines);

    SINGLETON_POOL_ALLOCATED_DECL();

    static FStringView AppIn_SubstitutionName();
    static FStringView AppIn_VertexDefinitionName();
    static FileSystem::FStringView SystemDirpath();

private:
    FString _sourceName;
    PPE::FFilename _filename;
    RAWSTORAGE(Shader, char) _sourceCode;
    ASSOCIATIVE_VECTOR(Shader, FString, FString) _defines;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
