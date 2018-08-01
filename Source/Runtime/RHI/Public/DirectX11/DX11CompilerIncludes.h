#pragma once

#include "Graphics.h"

#include "DirectX11/DX11Includes.h"

#ifdef PLATFORM_WINDOWS
#   include <d3dcompiler.h>

//  include the Direct3D Shader Compiler Library file
#   pragma comment(lib, "d3dcompiler.lib")
#else
#   error "no support"
#endif
