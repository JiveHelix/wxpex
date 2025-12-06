#pragma once


#include <wxpex/wxshim.h>

#if defined(_CONSOLE_APP)
#define wxshimAPP(appClass) wxshimIMPLEMENT_APP_CONSOLE(appClass)
#else
#define wxshimAPP(appClass) wxshimIMPLEMENT_APP(appClass)
#endif
