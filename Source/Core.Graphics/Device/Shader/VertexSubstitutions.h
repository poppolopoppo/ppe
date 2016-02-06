#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/IO/String.h"

namespace Core {
namespace Graphics {
class VertexDeclaration;
enum class VertexSubPartFormat;
enum class VertexSubPartSemantic;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice VertexSubPartFormatToShaderFormat(VertexSubPartFormat format);
//----------------------------------------------------------------------------
StringSlice VertexSubPartSemanticToShaderSemantic(VertexSubPartSemantic semantic);
//----------------------------------------------------------------------------
void FillVertexSubstitutions(   VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>)& defines,
                                const VertexDeclaration *declaration );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
