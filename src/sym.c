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
/* exported functions */

/* macros */
/* typedefs */
typedef struct symbol *Symbol;

typdef struct coord {
	char *fie;
	unsigned x, y;
} Coordinate;

/* types */
/* data */
/* functions */


