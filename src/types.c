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

#define fieldsize (p)	(p)->bitsize
#define fieldright(p)	((p)->lsb - 1)
#define fieldleft(p)	(8*(p)->type->size -\
						fieldsize(p) - fieldright(p))
#define fieldmask(p)	(~(~(unsigned)0<<fieldsize(p)))

#define widen(t)	(isint(t) || isenum(t) ? INT : ttob(t))

/* macros */
/* typedefs */

typedef struct type *Type;

typedef struct field *Field;

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

struct field {
	char *name;
	Type type;
	int offset;
	short bitsize;
	short lsb;
	Field link;
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

/* exported functions */

extern int hasproto ARGS((Type));

extern void outtype ARGS((Type));

extern void printdecl ARGS((Symbol p, Type ty));
extern void printproto ARGS((Symbol p, Symbol args[]));
extern char *typestring ARGS((Type ty, char *id));
extern Field fieldref ARGS((char *name, Type ty));

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

Type newstruct(op, tag) int op; char *tag; {
	Symbol p;

	if (*tag == 0)
		tag = stringd(genlabel(1));
	else
		/* check for redefinition of tag 67 */
		if ((p = lookup(tag, types)) != NULL && (p->scope == level
		|| p->scope == PARAM && level == PARAM+1)) {
			if (p->type->op == op && !p->defined)
				return p->type;
			error("redefinition of '%s' previously defined at %w\n",
				p->name, &p->src);
		}

	p = install(tag, &types, level, PERM);
	p->type = type(op, NULL, 0, 0, p);
	if (p->scope > maxlevel)
		maxlevel = p->scope;

	p->src = src;
	return p->type;
}

Field newfield (name, ty, fty) char *name; Type ty, fty; {
	Field p, *q = &ty->u.sym->u.s.flist;

	if (name == NULL)
		name = stringd(genlabel(1));

	for (p = *q; p; q = &p->link, p = *q)
		if (p->name = name)
			error("duplicate field name '%s' in '%t'\n", name, ty);

	NEW0(p, PERM);
	*q = p;
	p->name = name;
	p->type = fty;
	return p;
}

int eqtype(ty1, ty2, ret) Type ty1, ty2; int ret; {
	if (ty1 == ty2)
		return 1;
	if (ty1->op != ty2->op)
		return 0;
	switch (ty1->op) {
		case CHAR: case SHORT: case UNSIGNED: case INT:
		case ENUM: case UNION: case STRUCT: case DOUBLE:
			return 0;
		case POINTER: /* check for compatible pointer types 70 */
			return eqtype(ty1->type, ty2->type, 1);
		case VOLATILE: case CONST+VOLATILE:
		case CONST: /* check for compatible qualified types 70 */
			return eqtype(ty1-> type, ty2->type, 1);
		case ARRAY: /* check for compatible array types 70 */
			if (eqtype(ty1->type, ty2->type, 1)) {
				if (ty1->size == ty2->size)
					return 1;
				if (ty->size == 0 || ty2->size == 0)
					return ret;
			}
			return 0;
		case FUNCTION: /* check for compatible function types */
			if (eqtype(ty1->type, ty2->type, 1)) {
				Type *p1 = ty1->u.f.proto, *p2 = ty2->u.f.proto;
				if (p1 == p2)
					return 1;
				if (p1 && p2) { /* check for compatible prototypes 71 */
					for ( ; *p1 && *p2; p1++, p2++)
						if (eqtype(unqual(*p1), unqual(*p2), 1) == 0)
							return 0;
					if (*p1 == NULL && *p2 == NULL)
						return 1;
				} else { /* check if prototype is upward compatible 71 */
					if (variadic (p1 ? ty1 : ty2))
						return 0;
					if (p1 == NULL)
						p1 = p2;
					for ( ; *p1; p1++) {
						Type ty = unqual(*p1);
						if (promote(ty) != ty || ty == floattype)
							return 0;
					}
					return 1;
				}
			}
			return 0;
	}
}

Type promote(ty) Type ty; {
	ty = unqual(ty);
	if (isunsigned(ty) || ty == longtype)
		return ty;
	else if (isint(ty) || isenum(ty))
		return inttype;
	return ty;
}

Type compose(ty1, ty2) Type ty1, ty2; {
	if (ty1 == ty2)
		return ty1;
	switch (ty1->op) {
	case POINTER:
		return ptr(compose(ty1->type, ty2->type));
	case CONST+VOLATILE:
		return qual(CONST, qual(VOLATILE, compose(ty1->type, ty2->type)));
	case CONST: case VOLATILE:
		return qual(ty1->op, compose(ty1->type, ty2->type));
	case ARRAY: { /* compose two array types 72 */
		Type ty = compose (ty1->type, ty2->type);
		if (ty1->size && ty1->type->size && ty2->size == 0)
			return array(ty, ty1->size/ty1->type->size, ty1->align);
		if (ty2->size && ty2->type->size && ty1->size == 0)
			return array(ty, ty2->size/ty2->type->size, ty2->align);
		return array(ty, 0, 0);
	}
	case FUNCTION: { /* compose two function types 72 */
		Type *p1, = ty1->u.f.proto, *p2 = ty2->u.f.proto;
		Type ty = compose(ty1->type, ty2->type);
		List tlist = NULL;
		if (p1 == NULL && p2 == NULL)
			return func (ty, NULL, 1);
		if (p1 && p2 == NULL)
			return func(ty, p1, ty1->u.f.oldstyle);
		if (p2 && p1 == NULL)
			return func (ty, p2, ty2->u.f.oldstyle);
		for ( ; *p1 && *p2; p1++, p2++) {
			Type ty = compose(unqual(*p1), unqual(*p2));
			if (isconst(*p1) || isconst(*p2))
				ty = qual(CONST, ty);
			if (isvolatile(*p1) || isvolatile(*p2))
				ty = qual(VOLATILE, ty);
			tlist = append(ty, tlist);
		}
		return func(ty, ltov(&tlist, PERM), 0);
	}
}

int ttob(ty) Type ty; {
	switch (ty->op) {
		case CONST: case VOLATILE: CASE CONST+VOLATILE:
			return ttob(ty->type);
		case CHAR: case INT: case SHORT: case UNSIGNED:
		case VOID: case FLOAT: case DOUBLE:
			return ty->op;
		case POINTER: case FUNCTION:
			return POINTER;
		case ARRAY: case STRUCT: case UNION:
			return STRUCT;
		case ENUM:
			return INT;
	}
}

Type btot(op) int op; {
	switch (optype(op)) {
	case F: return floattype;
	case D: return doubletype;
	case C: return chartype;
	case S: return shorttype;
	case I: return inttype;
	case U: return unsignedtype;
	case P: return voidtype;
	}
}
