#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define DEBUG_ENABLED 1

#if !DEBUG_ENABLED
#define printf(x, fmt...)
#define putchar(x)
#endif


#endif // _CONSTANTS_H_
