#pragma once

#include "Graphics.h"

#include "Core/Pair.h"
#include "Core/String.h"
#include "Core/Vector.h"

namespace Core {
namespace Graphics {
class VertexDeclaration;
enum class VertexSubPartFormat;
enum class VertexSubPartSemantic;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *VertexSubPartFormatToShaderFormat(VertexSubPartFormat format);
//----------------------------------------------------------------------------
const char *VertexSubPartSemanticToShaderSemantic(VertexSubPartSemantic semantic);
//----------------------------------------------------------------------------
void FillVertexSubstitutions(   VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>)& defines,
                                const VertexDeclaration *declaration );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
