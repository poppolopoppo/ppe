// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/Imgui.h"

#ifdef _WIN32
//  #TODO: remove workaround for static inline definition with header units
#   if _USE_32BIT_TIME_T
#       define mktime(_Tm) _mktime32(_Tm)
#       define _mkgmtime(_Tm) _mkgmtime32(_Tm)
#       define gmtime_s(_Tm, _Time) _gmtime32_s(_Tm, _Time)
#       define localtime_s(_Tm, _Time) _localtime32_s(_Tm, _Time)
#   else
#       define mktime(_Tm) _mktime64(_Tm)
#       define _mkgmtime(_Tm) _mkgmtime64(_Tm)
#       define gmtime_s(_Tm, _Time) _gmtime64_s(_Tm, _Time)
#       define localtime_s(_Tm, _Time) _localtime64_s(_Tm, _Time)
#   endif
#endif

#include "imgui-impl.h"

#ifdef _WIN32
#   undef mktime
#   undef _mkgmtime
#   undef gmtime_s
#   undef localtime_s
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
