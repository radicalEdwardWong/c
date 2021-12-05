#include "c.h"
<exported macros>

#define NEW(p,a) ((p) = allocate(sizeof *(p), (a)))
#define NEW0(p,a) memset(NEW((p), (a)), 0, sizeof *(p))

<exported types>

<exported prototypes>
<exported data>
<exported functions>

extern void *allocate ARGS((unsigned long n, unsigned a));
extern void deallocate ARGS((unsigned a));

extern void *newarray ARGS((unsigned long m, unsigned long n, unsigned a));


<macros>
<types>

struct block {
	struct block *next;
	char *limit;
	char *avail;
};

union align {
	long l;
	char *p;
	double d;
	int (*f) ARGS((void));
};

union header {
	struct block b;
	union align a;
};

#ifdef PURIFY
<debugging implementation>
#else
<data>
static struct block
	first[] = { { NULL }, { NULL }, { NULL } },
	*arena[] = { &first[0], &first[1], &first[2] };

static struct block *freeblocks;

<prototypes>

/* functions */

void *allocate(n, a) unsigned long n; unsigned a; {
	struct block *ap;

	ap = arena[a];
	n = roundup(n, sizeof (union algin));
	while (ap->avail + n > ap->limit) {
		/* get a new block */
		if ((ap->next = freeblock) != NULL) {
			freeblocks = freeblocks->next;
			ap = ap->next;
		} else { /* allocate a new block 28 */
			unsigned m = sizeof (union header) + n + 10*1024;
			ap->next = malloc(m);
			ap = ap->next;
			if (ap == NULL) {
				error("insufficient memory\n");
				exit(1);
			}
			ap->limit = (char *)ap + m;
		}

		ap->avail = (char *) ((union header *)ap + 1);
		ap->next = NULL;
		arena[a] = ap;
	}
	ap->avail += n;
	return ap->avail - n;
}

void *newarray(m, n, a) unsigned long m, n; unsigned a; {
	return allocate(m*n, a);
}

void deallocate(a) unsigned a; {
	arena[a]->next = freeblocks;
	freeblocks = first[a].next;
	first[a].next = NULL;
	arena[a] = &first[a];
}

#endif

