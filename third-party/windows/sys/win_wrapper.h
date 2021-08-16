// Missing Macros, Preprocessors and funcions
// for llvm clang toolchain on Winodws

#ifndef __WIN_WRAPPER_H
#define __WIN_WRAPPER_H
#include <basetsd.h>
#include <time.h>
#include <limits.h>

#define CLOCK_MONOTONIC 0
#define clock_gettime(val, ts) timespec_get(ts, TIME_UTC);
typedef SSIZE_T ssize_t;
#define SSIZE_MAX LONG_MAX
#define S_IWUSR _S_IWRITE
#define S_IRUSR _S_IREAD
#define mode_t int

#endif /* __WIN_WRAPPER_H */