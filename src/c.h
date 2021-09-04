/* exported macros */

#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif

#ifdef __STDC__
#include <stdarg.h>
#define va_init(a,b) va_start(a,b)
#else
#include <varargs.h>
#define va_init(a,b) va_start(a)
#endif

#ifdef __STDC__
#define VARARGS(newlist,oldlist,olddcls) newlist
#else
#define VARARGS(newlist,oldlist,olddcls) oldlist olddcls
#endif

/* supports pointers of different sizes than int/word */
#ifndef NULL
#define NULL ((void*)0)
#endif

#define NELEMS(a) ((int)(sizeof (a)/sizeof ((a)[0])))
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

/* typedefs */
#include "config.h"
/* interface 78 */
/* exported types */
/* exported data */
/* exported functions */


