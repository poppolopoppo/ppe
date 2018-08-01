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
FStringView VertexFormatToShaderFormat(FVertexFormat format);
//----------------------------------------------------------------------------
FStringView VertexSemanticToShaderSemantic(const Graphics::FName& semantic);
//----------------------------------------------------------------------------
void FillVertexSubstitutions(   VECTOR(Shader, TPair<FString COMMA FString>)& defines,
                                const FVertexDeclaration *declaration );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
