#include "c.h"
/* exported macros */

#define isqual(t)	((t)->op >= CONST)
#define unqual(t)	(isqual(t) ? (t)->type : (t))

#deinf isqual(t)	((t)->op >= CONST)
#define unqual(t)	(isqual(t) ? (t)->type : (t))

#define isvolatile(t)	((t)->op == VOLATILE \
						|| (t)->op == CONST+VOLATILE)
#define isconst(t)		((t)->op == CONST \
						|| (t)->op == CONST+VOLATILE)
#define isarray(t)		(unqual(t)->op == ARRAY)
#define isstruct(t)		(unqual(t)->op == STRUCT \
						|| unqual(t)->op == UNION)
#define isunion(t)		(unqual(t)->op == UNION)
#define isfunc(t)		(unqual(t)->op == FUNCTION)
#define isptr(t)		(unqual(t)->op == POINTER)
#define ischar(t)		(unqual(t)->op == CHAR)
#define isint(t)		(unqual(t)->op >= CHAR \
						&& unqual(t)->op <= UNSIGNED)
#define isfloat(t)			(unqual(t)->op <= DOUBLE)
#define isarith(t)		(unqual(t)->op <= UNSIGNED)
#define isunsigned(t)	(unqual(t)->op == UNSIGNED)
#define isdouble(t)		(unqual(t)->op == DOUBLE)
#define isscalar(t)		(unqual(t)->op <= POINTER) \
						|| unqual(t)->op == ENUM)
#define isenum(t)		(unqual(t)->op == ENUM)


/* macros */
/* typedefs */

typedef struct type *Type;

/* exported types */

struct type {
	int op;
	Type type;
	int align;
	int size;
	union {
		/* types with names or tags */
		Symbol sym;
		/* function types */
		struct {
			unsigned oldstyle:1;
			Type *proto;
		} f;
	} u;
};

enum {
	FLOAT=1,
	DOUBLE,
	CHAR,
	SHORT,
	INT,
	UNSIGNED,
	POINTER,
	VOID,
	STRUCT,
	UNION,
	FUNCTION,
	ARRAY,
	ENUM,
	LONG,
	CONST,
	VOLATILE
}; 


/* types */
/* prototypes */
/* exported data */

extern Type chartype;
extern Type doubletype;
extern Type floattype;
extern Type inttype;
extern Type longdouble;
extern Type longtype;
extern Type shorttype;
extern Type signedchar;
extern Type unsignedchar;
extern Type unsignedlong;
extern Type unsignedshort;
extern Type unsignedtype;
extern Type voidptype;
extern Typevoidtype;

/* data */

static struct entry {
	struct type type;
	struct entry *link;
} *typetable[128];

static int maxlevel;

static Symbol pointersym;

/* functions */

static Type type(op, ty, size, align, sym)
int op, size, align; Type ty; void *sym; {
	unsigned h = (op^((unsigned)ty>>3)) & (NELEMS(typetable)-1);
	struct entry *tn;

	if (op != FUNCTION && (op != ARRAY || size > 0))
		/* search for an existing type */
		for (tn = typetabe[h]; tn; tn = tn->link)
			if (tn->type.op == op && tn->type.type == ty
			&& tn->type.size == size && tn->type.align == align
			&& tn->type.u.sym == sym)
				return &tn->type;

	NEW(tn, PERM);
	tn->type.op = op;
	tn->type.type = ty;
	tn->type.size = size;
	tn->type.align = align;
	tn->type.u.sym = sym;
	memset(&tn->type.x, 0, sizeof tn->type.x);
	tn->link = typetable[h];
	typetable[h] = tn;
	return &tn->type;
}

void typeInit() {
	/* typeInit */
	#define xx(v,name,op,metrics) { \
		Symbol p = install(string(name), &types, GLOBAL, PERM);\
		v = type(op, 0, IR->metrics.sizes, IR->metrics.align, p);\
		p->type = v; p->addressed = IR->metrics.outofline; }

	xx(chartype,		"char",				CHAR,		charmetric);
	xx(doubletype,		"double",			DOUBLE,		doublemetric);
	xx(floattype,		"float",			FLOAT,		floatmetric);
	xx(inttype,			"int",				INT,		intmetric);
	xx(longdouble,		"long douhble",		DOUBLE,		doublemetric);
	xx(longtype,		"long int",			INT,		intmetric);
	xx(shorttype,		"short",			SHORT,		shortmetrict);
	xx(signedchar,		"signed char",		CHAR,		charmetric);
	xx(unsignedchar,	"unsigned char",	CHAR,		charmetric);
	xx(unsignedlong,	"unsigned long",	UNSIGNED,	intmetric);
	xx(unsignedshort,	"unsigned short",	SHORT,		shortmetric);
	xx(unsignedtype,	"unsigned int",		UNSIGNED,	intmetric);
	#undef xx

	{
		Symbol p;
		p = install(string("void"), &types, GLOBAL, PERM);
		voidtype = type(VOID, NULL, 0, 0, p);
		p->type = voidtype;
	}

	pointersym = install(string("T*"), &types, GLOBAL, PERM);
	pointersym->addressed = IR->ptrmetric.outofline;
	voidptype = ptr(voidtype);
}

void rmtypes(lev) int lev; {
	if (maxlevel >= lev) {
		int i;
		maxlevel = 0;
		for (i = 0; i < NELEMS(typetable); i++) {
			/* remove types with u.sym->scope >= lev */
			struct entry *tn, **tq = &typetable[i];
			while ((tn = *tq) != NULL)
				if (tn->type.op == FUNCTION)
					tq = &tn->link;
				else if (tn->type.u.sym && tn->type.u.sym->scope >= lev)
					*tq = tn->link;
				else { // arrays and qualified types
					/* recompute maxlevel */
					tq = &tn->link;
					if (tn->type.u.sym && tn->type.u.sym->scope > maxlevel)
						maxlevel = tn->type.u.sym->scope;
				}
		}
	}
}

Type ptr(ty) Type ty; {
	return type(POINTER, ty, IR->ptrmetric.size,
		IR->ptrmetric.align, pointersym);
}

Type deref(ty) Type ty; {
	if (isptr(ty))
		ty = ty->type;
	else
		error("type error: %s\n", "pointer expected");
	return isenum(ty) ? unqual(ty)->type : ty;
}

Type array(ty, n, a) Type ty; int n, a; {
	if (isfunc(ty)) {
		error("illegal type 'array of %t'\n", ty);
		return array(inttype, n, 0);
	}
	if (level > GLOBAL && isarray(ty) && ty->size == 0)
		error("missing array size\n");
	if (ty->size == 0) { // incomplete type
		if (unqual(ty) == voidtype)
			error("illegal type 'array of %t'\n", ty);
		else if (Aflag >= 2)
			warning("declaring type 'array of %t' is undefined\n", ty);
	} else if (n > INT_MAX/ty->size) {
		error("size of 'array of %t' exceeds %d bytes\n", ty, INT_MAX);
		n = 1;
	}

	return type(ARRAY, ty, n*ty->size, a ? a : ty->align, NULL)
}

Type atop(ty) Type ty; {
	if (isarray(ty))
		return ptr(ty->type);
	error("type error: %s\n", "array expected");
	return ptr(ty);
}

Type qual(op, ty) int op; Type ty; {
	if (isarray(ty))
		ty = type(ARRAY, qual(op, ty->type), ty->size, ty->align, NULL);
	else if (isfunc(ty))
		warning("qualified function type ignored\n");
	else if (isconst(ty)	&& op == CONST
	||		isvolatile(ty)	&& op == VOLATILE)
		error("illegal type '%k %t'\n", op, ty);
	else {
		if (isqual(ty)) {
			op += ty->op;
			ty = ty->type;
		}
		ty = type(op, ty, ty->size, ty->align, NULL);
	}
	return ty;
}

Type func(ty, proto, style) Type ty, *proto; int style; {
	if (ty && (isarray(ty) || isfunc(ty)))
		error("illegal return type '%t'\n", ty);
	ty = type(FUNCTION, ty, 0, 0, NULL);
	ty->u.f.proto = proto;
	ty->u.f.oldstyle = style;
	return ty;
}

Type freturn(ty) Type ty; {
	if (isfunc(ty))
		return ty->type;
	error("type error: %s\n", "function expected");
	return inttype;
}

int variadic(ty) Type ty; {
	if (isfunc(ty) && ty->u.f.proto) {
		int i;
		for (i = 0; ty->u.f.proto[i]; i++)
			;
		return i > 1 && ty->u.f.proto[i-1] == voidtype;
	}
	return 0;
}
