#ifndef _LIB_PLATFORM_CONFIG_HLSL_INCLUDED
#define _LIB_PLATFORM_CONFIG_HLSL_INCLUDED

#include "Lib/Platform/Common.fx"

#if defined(DIRECTX11)
#   include "Lib/Platform/DirectX11.fx"
#elif defined(OPENGL)
#   include "Lib/Platform/OpenGL4.fx"
#else
#   error "Invalid shader platform"
#endif

#ifndef GRAPHIC_API
#   error "GRAPHIC_API must be defined in the api config header"
#endif

#endif //!_LIB_PLATFORM_CONFIG_H_INCLUDED
