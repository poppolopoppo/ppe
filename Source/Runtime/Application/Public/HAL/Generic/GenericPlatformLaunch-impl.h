
#include "HAL/Generic/GenericPlatformLaunch.h"

#ifndef PPE_APPLICATIONLAUNCH_TYPE
#   error "must define PPE_APPLICATIONLAUNCH_TYPE to implement main function"
#endif

#ifdef PPE_APPLICATIONLAUNCH_INCLUDED
#   error "should be included only once inside your main.cpp"
#endif
#define PPE_APPLICATIONLAUNCH_INCLUDED
