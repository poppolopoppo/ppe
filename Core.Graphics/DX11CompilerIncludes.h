#pragma once

#include "Graphics.h"

#include "DX11Includes.h"

#ifdef OS_WINDOWS
#   include <d3dcompiler.h>

//  include the Direct3D Shader Compiler Library file
#   pragma comment(lib, "d3dcompiler.lib")
#else
#   error "no support"
#endif
