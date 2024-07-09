// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "HAL/PlatformIncludes.h"

#include "RTTI_fwd.h"

#include "Container/AssociativeVector.h"
#include "Container/HashMap.h"
#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "IO/String.h"
#include "IO/TextWriter.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector.h"

#ifdef BUILD_PCH // deprecated
#   include "stdafx.generated.h"
#endif
