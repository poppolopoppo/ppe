#pragma once

#include "Graphics.h"

#include "Device/Geometry/VertexDeclaration.h"

#include "Container/Pair.h"
#include "Container/Vector.h"
#include "IO/String.h"

namespace PPE {
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
} //!namespace PPE
