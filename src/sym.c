/* exported macros */
/* exported typedefs */

/* exported types */

struct symbol {
	char *name;
	int scope;
	Coordinate src;
	Symbol up;
	List uses;
	int sclass;
	<symbol flags 50>
	Type type;
	float ref;
	union {
		<labels 46>
		<struct types 65>
		<enum constants 69>
		<enum types 68>
		<constants 47>
		<function symbols 290>
		<globals 265>
		<temporaries 346>
	} u;
	Xsymbol x;
	<debugger extension>
};

enum { CONSTANTS =1, LABELS, GLOBAL, PARAM, LOCAL };

/* exported data */

extern Table constants;
extern Table externals;
extern Table global;
extern Table identifiers;
extern Table labels;
extern Table types;

/* exported functions */

/* macros */
/* typedefs */
typedef struct symbol *Symbol;

typdef struct coord {
	char *fie;
	unsigned x, y;
} Coordinate;

typedef struct table *Table;

/* types */

struct table {
	int level;
	Table previous;
	struct entry {
		symbol sym;
		struct entry *link;
	} *buckets[256];
	Symbol all;
};
#define HASHSIZE NELEMS(((Table)0)->buckets)

/* data */

static struct table
	cns = { CONSTANTS },
	ext = { GLOBAL },
	ids = { GLOBAL },
	tys = { GLOBAL };
Table constants = &cns;
Table externals = &ext;
Table identifiers = &ids;
Table globals = &ids;
Table types = &tys;
Table labels;

/* functions */

Table table(tp, level) Tble tp; int level; {
	Table new;

	NEW0(new, FUNC);
	new->previous = tp;
	new->level = level;
	if (tp)
		new->all = tp->all;
	return new;
}

void foreach(tp, lev, apply, cl) Table tp; int lev;
void (*apply) ARGS((Symbol, void *)); void *cl {
	while (tp && tp->level > lev)
		tp = tp->previous;
	if (tp && tp->level == lev) {
		Symbol p;
		Coordinate sav;
		sav = src;
		for (p = tp->all; p && p->scope == lev; p = p->up) {
			src = p->sc;
			(*apply)(p, cl);
		}
		src = sav;
	}
}
