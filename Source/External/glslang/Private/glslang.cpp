
#include "glslang-external.h"

#include "glslang/Include/Types.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/ResourceLimits.h"
#include "glslang/MachineIndependent/localintermediate.h"

#ifdef GLSLANG_IS_SHARED_LIBRARY
#   error "we want static linking for glslang!"
#endif

GLSLANG_EXPORT
