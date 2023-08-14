#pragma once


#ifndef DO_PRAGMA
#define DO_PRAGMA_(arg) _Pragma (#arg)
#define DO_PRAGMA(arg) DO_PRAGMA_(arg)
#endif


#if defined _WIN32 && !(defined __MINGW32__ || __MINGW64__)

#define WXSHIM_PUSH_IGNORES \
    __pragma(warning(push)) \
    __pragma(warning(disable:4996)) \
    __pragma(warning(disable:4242))

#define WXSHIM_POP_IGNORES \
    __pragma(warning(pop))

#else

#define WXSHIM_PUSH_IGNORES \
    DO_PRAGMA(GCC diagnostic push); \
    DO_PRAGMA(GCC diagnostic ignored "-Wold-style-cast"); \
    DO_PRAGMA(GCC diagnostic ignored "-Wsign-conversion"); \
    DO_PRAGMA(GCC diagnostic ignored "-Wdouble-promotion");

#define WXSHIM_POP_IGNORES \
    DO_PRAGMA(GCC diagnostic pop)

#endif
