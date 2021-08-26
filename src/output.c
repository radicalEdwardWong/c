<exported macros>
<typedefs>
#include "config.h"
<interface 78>
<exported types>
<exported data>
/* exported functions */

extern void outs ARGS((char *));

extern void print ARGS((char *, ...));


/* functions */

void outs(s) char *s; {
	char *p;
	for (p = bp; (*p = *s++) != 0; p++)
		;
	bp = p;
	if (bp > io[fd]->limit)
		outflush();
}

void print VARARGS((char *fmt, ...),(fmt, va_alist),char *fmt; va_dcl) {
	va_list ap;

	va_init(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);
}
