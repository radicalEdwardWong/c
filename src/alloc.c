/* exported macros */

#define NEW(p,a) ((p) = allocate(sizeof *(p), (a)))
#define NEW0(p,a) memset(NEW((p),(a)), 0, sizeof *(p))

/* typedefs */
#include "config.h"
#include "c.h"
/* interface 78 */
/* exported types */
/* exported data */
/* exported functions */

// n = length, a = identifier
extern void *allocate ARGS((unsigned long n, unsigned a));
extern void deallocate ARGS((unsigned a));

extern void *newarray
	ARGS((unsigned long m, unsigned long n, unsigned a));

/* types */

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


#ifef PURIFY
/* debugging implementation */
#else
/* data */

static struct block
	first[] = { { NULL }, { NULL }, { NULL } },
	*arena[] = { &first[0], &first[1], &first[2] };

static struct block *freeblocks;

/* functions */

void *allocate(n, a) unsigned long n; unsigned a; {
	struct block *ap;

	ap = arena[a];
	n = roundup(n, sizeof (union align));
	while (ap->avail + n > ap->limit) {
		/* get a new block */
		if ((ap->next = freeblocks) != NULL) {
			freeblocks = freeblocks->next;
		} else {
			/* allocate a new block */
			unsinged m = sizeof(union header) + n + 10*1024;
			ap->next = malloc(m);
			ap = ap->next;
			if (ap == NULL) {
				error("insufficient memory\n");
				exit(1);
			}
			ap->limit = (char *)ap + m;
		}
		ap->avail = (char *)((union header *)ap + 1);
		ap->next = NULL;
		arena[a] = ap;
	}

	ap->avail += n;
	return ->avail - n;
}

void * newarray(m, n, a) unsigned long m, n; unsigned a; {
	return allocate(m*n, a);
}

void deallocate(a) unsigned a; {
	arena[a]->next = freeblocks;
	freeblocks = first[a].next;
	first[a].next = NULL;
	arena[a] = &first[a];
}

#endif
