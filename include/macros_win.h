#ifndef MACROS_WIN_H
#define MACROS_WIN_H

#if defined(_WIN32) && defined(__MINGW32__)
// Avoid conflict from /mingw32/.../include/winerror.h */
#undef ERROR_STACK_OVERFLOW
#endif

#endif /* end of include guard: MACROS_WIN_H */
