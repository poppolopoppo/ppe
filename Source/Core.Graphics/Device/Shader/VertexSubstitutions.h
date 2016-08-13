#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/IO/String.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice VertexFormatToShaderFormat(VertexFormat format);
//----------------------------------------------------------------------------
StringSlice VertexSemanticToShaderSemantic(const Graphics::Name& semantic);
//----------------------------------------------------------------------------
void FillVertexSubstitutions(   VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>)& defines,
                                const VertexDeclaration *declaration );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
