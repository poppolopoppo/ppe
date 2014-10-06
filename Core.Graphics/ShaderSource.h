#pragma once

#include "Graphics.h"

#include "Core/AssociativeVector.h"
#include "Core/Filename.h"
#include "Core/MemoryView.h"
#include "Core/Pair.h"
#include "Core/PoolAllocator.h"
#include "Core/RawStorage.h"
#include "Core/RefPtr.h"
#include "Core/String.h"

namespace Core {
namespace Graphics {
class ShaderProgram;
class VertexDeclaration;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderSource);
class ShaderSource : public RefCountable {
public:
    ShaderSource(   const char *sourceName,
                    const Core::Filename& filename,
                    const MemoryView<const char>& sourceCode,
                    const MemoryView<const Pair<String, String>>& defines);

    ShaderSource(   String&& sourceName,
                    const Core::Filename& filename,
                    RAWSTORAGE_THREAD_LOCAL(Shader, char)&& sourceCode,
                    ASSOCIATIVE_VECTOR_THREAD_LOCAL(Shader, String, String)&& defines);

    ~ShaderSource();

    ShaderSource(const ShaderSource& ) = delete;
    ShaderSource& operator =(const ShaderSource& ) = delete;

    const String& SourceName() const { return _sourceName; }
    const Core::Filename& Filename() const { return _filename; }
    MemoryView<const char> SourceCode() const { return MakeView(_sourceCode); }
    MemoryView<const Pair<String, String>> Defines() const { return MakeView(_defines); }

    void Preprocess(RAWSTORAGE_THREAD_LOCAL(Shader, char)& preprocessed,
                    const ShaderProgram *program,
                    const VertexDeclaration *vertexDeclaration) const;

    void FillSubstitutions( VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>)& substitutions,
                            const VertexDeclaration *vertexDeclaration) const;

    static ShaderSource *LoadFromFile(  const Core::Filename& filename,
                                        const MemoryView<const Pair<String, String>>& defines);

    SINGLETON_POOL_ALLOCATED_DECL(ShaderSource);

    static const char *AppIn_SubstitutionName();
    static const char *AppIn_VertexDefinitionName();
    static const FileSystem::char_type *SystemDirpath();

private:
    String _sourceName;
    Core::Filename _filename;
    RAWSTORAGE_THREAD_LOCAL(Shader, char) _sourceCode;
    ASSOCIATIVE_VECTOR_THREAD_LOCAL(Shader, String, String) _defines;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
