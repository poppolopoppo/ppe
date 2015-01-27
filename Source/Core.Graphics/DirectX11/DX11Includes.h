#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceDiagnostics.h"

#ifdef _DEBUG
#   define WITH_DIRECTX11_DEBUG_LAYER
//#   define WITH_DIRECTX11_WARP_DRIVER //%__NOCOMMIT%
#endif

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
#   define WITH_DIRECTX11_DEBUG_MARKERS
#endif

#ifdef OS_WINDOWS
#   include <d3d11.h>
#   include <dxgi.h>
#   include <d3dx11.h>

//  include the Direct3D Library file
#   pragma comment (lib, "d3d11.lib")
#   ifdef WITH_DIRECTX11_DEBUG_LAYER
#       pragma comment (lib, "d3dx11d.lib")
#   else
#       pragma comment (lib, "d3dx11.lib")
#   endif

//  Direct3D interface GUIDs
#   pragma comment (lib, "dxguid.lib")

#else
#   error "no support"
#endif
