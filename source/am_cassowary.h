// based on: https://github.com/starwing/amcw_cassowary

#ifndef amcw_cassowary_h
#define amcw_cassowary_h

#ifndef AMCW_NS_BEGIN
# ifdef __cplusplus
#   define AMCW_NS_BEGIN extern "C" {
#   define AMCW_NS_END   }
# else
#   define AMCW_NS_BEGIN
#   define AMCW_NS_END
# endif
#endif /* AMCW_NS_BEGIN */

#ifndef AMCW_STATIC
# ifdef __GNUC__
#   define AMCW_STATIC static __attribute((unused))
# else
#   define AMCW_STATIC static
# endif
#endif

#ifdef AMCW_STATIC_API
# ifndef AMCW_IMPLEMENTATION
#  define AMCW_IMPLEMENTATION
# endif
# define AMCW_API AMCW_STATIC
#endif

#if !defined(AMCW_API) && defined(_WIN32)
# ifdef AMCW_IMPLEMENTATION
#  define AMCW_API __declspec(dllexport)
# else
#  define AMCW_API __declspec(dllimport)
# endif
#endif

#ifndef AMCW_API
# define AMCW_API
#endif

#define AMCW_OK           (0)
#define AMCW_FAILED       (-1)
#define AMCW_UNSATISFIED  (-2)
#define AMCW_UNBOUND      (-3)

#define AMCW_LESSEQUAL    (1)
#define AMCW_EQUAL        (2)
#define AMCW_GREATEQUAL   (3)

#define AMCW_REQUIRED     ((amcw_Num)1000000000)
#define AMCW_STRONG       ((amcw_Num)1000000)
#define AMCW_MEDIUM       ((amcw_Num)1000)
#define AMCW_WEAK         ((amcw_Num)1)

#include <stddef.h>

AMCW_NS_BEGIN


#ifdef AMCW_USE_FLOAT
typedef float  amcw_Num;
#else
typedef double amcw_Num;
#endif

typedef struct amcw_Solver     amcw_Solver;
typedef struct amcw_Var        amcw_Var;
typedef struct amcw_Constraint amcw_Constraint;

typedef void *amcw_Allocf (void *ud, void *ptr, size_t nsize, size_t osize);

AMCW_API amcw_Solver *amcw_newsolver   (amcw_Allocf *allocf, void *ud);
AMCW_API void       amcw_resetsolver (amcw_Solver *solver, int clear_constraints);
AMCW_API void       amcw_delsolver   (amcw_Solver *solver);

AMCW_API void amcw_updatevars (amcw_Solver *solver);
AMCW_API void amcw_autoupdate (amcw_Solver *solver, int auto_update);

AMCW_API int amcw_hasedit       (amcw_Var *var);
AMCW_API int amcw_hasconstraint (amcw_Constraint *cons);

AMCW_API int  amcw_add    (amcw_Constraint *cons);
AMCW_API void amcw_remove (amcw_Constraint *cons);

AMCW_API int  amcw_addedit (amcw_Var *var, amcw_Num strength);
AMCW_API void amcw_suggest (amcw_Var *var, amcw_Num value);
AMCW_API void amcw_deledit (amcw_Var *var);

AMCW_API amcw_Var *amcw_newvariable (amcw_Solver *solver);
AMCW_API void    amcw_usevariable (amcw_Var *var);
AMCW_API void    amcw_delvariable (amcw_Var *var);
AMCW_API int     amcw_variableid  (amcw_Var *var);
AMCW_API amcw_Num  amcw_value       (amcw_Var *var);

AMCW_API amcw_Constraint *amcw_newconstraint   (amcw_Solver *solver, amcw_Num strength);
AMCW_API amcw_Constraint *amcw_cloneconstraint (amcw_Constraint *other, amcw_Num strength);

AMCW_API void amcw_resetconstraint (amcw_Constraint *cons);
AMCW_API void amcw_delconstraint   (amcw_Constraint *cons);

AMCW_API int amcw_addterm     (amcw_Constraint *cons, amcw_Var *var, amcw_Num multiplier);
AMCW_API int amcw_setrelation (amcw_Constraint *cons, int relation);
AMCW_API int amcw_addconstant (amcw_Constraint *cons, amcw_Num constant);
AMCW_API int amcw_setstrength (amcw_Constraint *cons, amcw_Num strength);

AMCW_API int amcw_mergeconstraint (amcw_Constraint *cons, const amcw_Constraint *other, amcw_Num multiplier);


AMCW_NS_END

#endif /* amcw_cassowary_h */


#if defined(AMCW_IMPLEMENTATION) && !defined(amcw_implemented)
#define amcw_implemented


#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

#define AMCW_EXTERNAL     (0)
#define AMCW_SLACK        (1)
#define AMCW_ERROR        (2)
#define AMCW_DUMMY        (3)

#define amcw_isexternal(key)   ((key).type == AMCW_EXTERNAL)
#define amcw_isslack(key)      ((key).type == AMCW_SLACK)
#define amcw_iserror(key)      ((key).type == AMCW_ERROR)
#define amcw_isdummy(key)      ((key).type == AMCW_DUMMY)
#define amcw_ispivotable(key)  (amcw_isslack(key) || amcw_iserror(key))

#define AMCW_POOLSIZE     4096
#define AMCW_MIN_HASHSIZE 4
#define AMCW_MAX_SIZET    ((~(size_t)0)-100)

#ifdef AMCW_USE_FLOAT
# define AMCW_NUM_MAX FLT_MAX
# define AMCW_NUM_EPS 1e-4f
#else
# define AMCW_NUM_MAX DBL_MAX
# define AMCW_NUM_EPS 1e-6
#endif

AMCW_NS_BEGIN


typedef struct amcw_Symbol {
    unsigned id   : 30;
    unsigned type : 2;
} amcw_Symbol;

typedef struct amcw_MemPool {
    size_t size;
    void  *freed;
    void  *pages;
} amcw_MemPool;

typedef struct amcw_Entry {
    int       next;
    amcw_Symbol key;
} amcw_Entry;

typedef struct amcw_Table {
    size_t    size;
    size_t    count;
    size_t    entry_size;
    size_t    lastfree;
    amcw_Entry *hash;
} amcw_Table;

typedef struct amcw_VarEntry {
    amcw_Entry entry;
    amcw_Var  *var;
} amcw_VarEntry;

typedef struct amcw_ConsEntry {
    amcw_Entry       entry;
    amcw_Constraint *constraint;
} amcw_ConsEntry;

typedef struct amcw_Term {
    amcw_Entry entry;
    amcw_Num   multiplier;
} amcw_Term;

typedef struct amcw_Row {
    amcw_Entry  entry;
    amcw_Symbol infeasible_next;
    amcw_Table  terms;
    amcw_Num    constant;
} amcw_Row;

struct amcw_Var {
    amcw_Symbol      sym;
    amcw_Symbol      dirty_next;
    unsigned       refcount;
    amcw_Solver     *solver;
    amcw_Constraint *constraint;
    amcw_Num         edit_value;
    amcw_Num         value;
};

struct amcw_Constraint {
    amcw_Row     expression;
    amcw_Symbol  marker;
    amcw_Symbol  other;
    int        relation;
    amcw_Solver *solver;
    amcw_Num     strength;
};

struct amcw_Solver {
    amcw_Allocf *allocf;
    void      *ud;
    amcw_Row     objective;
    amcw_Table   vars;            /* symbol -> VarEntry */
    amcw_Table   constraints;     /* symbol -> ConsEntry */
    amcw_Table   rows;            /* symbol -> Row */
    amcw_MemPool varpool;
    amcw_MemPool conspool;
    unsigned   symbol_count;
    unsigned   constraint_count;
    unsigned   auto_update;
    amcw_Symbol  infeasible_rows;
    amcw_Symbol  dirty_vars;
};


/* utils */

static amcw_Symbol amcw_newsymbol(amcw_Solver *solver, int type);

static int amcw_approx(amcw_Num a, amcw_Num b)
{ return a > b ? a - b < AMCW_NUM_EPS : b - a < AMCW_NUM_EPS; }

static int amcw_nearzero(amcw_Num a)
{ return amcw_approx(a, 0.0f); }

static amcw_Symbol amcw_null()
{ amcw_Symbol null = { 0, 0 }; return null; }

static void amcw_initsymbol(amcw_Solver *solver, amcw_Symbol *sym, int type)
{ if (sym->id == 0) *sym = amcw_newsymbol(solver, type); }

static void amcw_initpool(amcw_MemPool *pool, size_t size) {
    pool->size  = size;
    pool->freed = pool->pages = NULL;
    assert(size > sizeof(void*) && size < AMCW_POOLSIZE/4);
}

static void amcw_freepool(amcw_Solver *solver, amcw_MemPool *pool) {
    const size_t offset = AMCW_POOLSIZE - sizeof(void*);
    while (pool->pages != NULL) {
        void *next = *(void**)((char*)pool->pages + offset);
        solver->allocf(solver->ud, pool->pages, 0, AMCW_POOLSIZE);
        pool->pages = next;
    }
    amcw_initpool(pool, pool->size);
}

static void *amcw_alloc(amcw_Solver *solver, amcw_MemPool *pool) {
    void *obj = pool->freed;
    if (obj == NULL) {
        const size_t offset = AMCW_POOLSIZE - sizeof(void*);
        void *end, *newpage = solver->allocf(solver->ud, NULL, AMCW_POOLSIZE, 0);
        *(void**)((char*)newpage + offset) = pool->pages;
        pool->pages = newpage;
        end = (char*)newpage + (offset/pool->size-1)*pool->size;
        while (end != newpage) {
            *(void**)end = pool->freed;
            pool->freed = (void**)end;
            end = (char*)end - pool->size;
        }
        return end;
    }
    pool->freed = *(void**)obj;
    return obj;
}

static void amcw_free(amcw_MemPool *pool, void *obj) {
    *(void**)obj = pool->freed;
    pool->freed = obj;
}

static amcw_Symbol amcw_newsymbol(amcw_Solver *solver, int type) {
    amcw_Symbol sym;
    unsigned id = ++solver->symbol_count;
    if (id > 0x3FFFFFFF) id = solver->symbol_count = 1;
    assert(type >= AMCW_EXTERNAL && type <= AMCW_DUMMY);
    sym.id   = id;
    sym.type = type;
    return sym;
}


/* hash table */

#define amcw_key(entry) (((amcw_Entry*)(entry))->key)

#define amcw_offset(lhs,rhs) ((int)((char*)(lhs) - (char*)(rhs)))
#define amcw_index(h,i)      ((amcw_Entry*)((char*)(h) + (i)))

static amcw_Entry *amcw_newkey(amcw_Solver *solver, amcw_Table *t, amcw_Symbol key);

static void amcw_delkey(amcw_Table *t, amcw_Entry *entry)
{ entry->key = amcw_null(), --t->count; }

static void amcw_inittable(amcw_Table *t, size_t entry_size)
{ memset(t, 0, sizeof(*t)), t->entry_size = entry_size; }

static amcw_Entry *amcw_mainposition(const amcw_Table *t, amcw_Symbol key)
{ return amcw_index(t->hash, (key.id & (t->size - 1))*t->entry_size); }

static void amcw_resettable(amcw_Table *t)
{ t->count = 0; memset(t->hash, 0, t->lastfree = t->size * t->entry_size); }

static size_t amcw_hashsize(amcw_Table *t, size_t len) {
    size_t newsize = AMCW_MIN_HASHSIZE;
    const size_t max_size = (AMCW_MAX_SIZET / 2) / t->entry_size;
    while (newsize < max_size && newsize < len)
        newsize <<= 1;
    assert((newsize & (newsize - 1)) == 0);
    return newsize < len ? 0 : newsize;
}

static void amcw_freetable(amcw_Solver *solver, amcw_Table *t) {
    size_t size = t->size*t->entry_size;
    if (size) solver->allocf(solver->ud, t->hash, 0, size);
    amcw_inittable(t, t->entry_size);
}

static size_t amcw_resizetable(amcw_Solver *solver, amcw_Table *t, size_t len) {
    size_t i, oldsize = t->size * t->entry_size;
    amcw_Table nt = *t;
    nt.size = amcw_hashsize(t, len);
    nt.lastfree = nt.size*nt.entry_size;
    nt.hash = (amcw_Entry*)solver->allocf(solver->ud, NULL, nt.lastfree, 0);
    memset(nt.hash, 0, nt.size*nt.entry_size);
    for (i = 0; i < oldsize; i += nt.entry_size) {
        amcw_Entry *e = amcw_index(t->hash, i);
        if (e->key.id != 0) {
            amcw_Entry *ne = amcw_newkey(solver, &nt, e->key);
            if (t->entry_size > sizeof(amcw_Entry))
                memcpy(ne + 1, e + 1, t->entry_size-sizeof(amcw_Entry));
        }
    }
    if (oldsize) solver->allocf(solver->ud, t->hash, 0, oldsize);
    *t = nt;
    return t->size;
}

static amcw_Entry *amcw_newkey(amcw_Solver *solver, amcw_Table *t, amcw_Symbol key) {
    if (t->size == 0) amcw_resizetable(solver, t, AMCW_MIN_HASHSIZE);
    for (;;) {
        amcw_Entry *mp = amcw_mainposition(t, key);
        if (mp->key.id != 0) {
            amcw_Entry *f = NULL, *othern;
            while (t->lastfree > 0) {
                amcw_Entry *e = amcw_index(t->hash, t->lastfree -= t->entry_size);
                if (e->key.id == 0 && e->next == 0)  { f = e; break; }
            }
            if (!f) { amcw_resizetable(solver, t, t->count*2); continue; }
            assert(f->key.id == 0);
            othern = amcw_mainposition(t, mp->key);
            if (othern != mp) {
                amcw_Entry *next;
                while ((next = amcw_index(othern, othern->next)) != mp)
                    othern = next;
                othern->next = amcw_offset(f, othern);
                memcpy(f, mp, t->entry_size);
                if (mp->next) f->next += amcw_offset(mp, f), mp->next = 0;
            } else {
                if (mp->next != 0)
                    f->next = amcw_offset(mp, f) + mp->next;
                else
                    assert(f->next == 0);
                mp->next = amcw_offset(f, mp), mp = f;
            }
        }
        mp->key = key;
        return mp;
    }
}

static const amcw_Entry *amcw_gettable(const amcw_Table *t, amcw_Symbol key) {
    const amcw_Entry *e;
    if (t->size == 0 || key.id == 0) return NULL;
    e = amcw_mainposition(t, key);
    for (; e->key.id != key.id; e = amcw_index(e, e->next))
        if (e->next == 0) return NULL;
    return e;
}

static amcw_Entry *amcw_settable(amcw_Solver *solver, amcw_Table *t, amcw_Symbol key) {
    amcw_Entry *e;
    assert(key.id != 0);
    if ((e = (amcw_Entry*)amcw_gettable(t, key)) != NULL) return e;
    e = amcw_newkey(solver, t, key);
    if (t->entry_size > sizeof(amcw_Entry))
        memset(e + 1, 0, t->entry_size-sizeof(amcw_Entry));
    ++t->count;
    return e;
}

static int amcw_nextentry(const amcw_Table *t, amcw_Entry **pentry) {
    amcw_Entry *end = amcw_index(t->hash, t->size*t->entry_size);
    amcw_Entry *e = *pentry;
    e = e ? amcw_index(e, t->entry_size) : t->hash;
    for (; e < end; e = amcw_index(e, t->entry_size))
        if (e->key.id != 0) return *pentry = e, 1;
    return *pentry = NULL, 0;
}


/* expression (row) */

static int amcw_isconstant(amcw_Row *row)
{ return row->terms.count == 0; }

static void amcw_freerow(amcw_Solver *solver, amcw_Row *row)
{ amcw_freetable(solver, &row->terms); }

static void amcw_resetrow(amcw_Row *row)
{ row->constant = 0.0f; amcw_resettable(&row->terms); }

static void amcw_initrow(amcw_Row *row) {
    amcw_key(row) = amcw_null();
    row->infeasible_next = amcw_null();
    row->constant = 0.0f;
    amcw_inittable(&row->terms, sizeof(amcw_Term));
}

static void amcw_multiply(amcw_Row *row, amcw_Num multiplier) {
    amcw_Term *term = NULL;
    row->constant *= multiplier;
    while (amcw_nextentry(&row->terms, (amcw_Entry**)&term))
        term->multiplier *= multiplier;
}

static void amcw_addvar(amcw_Solver *solver, amcw_Row *row, amcw_Symbol sym, amcw_Num value) {
    amcw_Term *term;
    if (sym.id == 0) return;
    term = (amcw_Term*)amcw_settable(solver, &row->terms, sym);
    if (amcw_nearzero(term->multiplier += value))
        amcw_delkey(&row->terms, &term->entry);
}

static void amcw_addrow(amcw_Solver *solver, amcw_Row *row, const amcw_Row *other, amcw_Num multiplier) {
    amcw_Term *term = NULL;
    row->constant += other->constant*multiplier;
    while (amcw_nextentry(&other->terms, (amcw_Entry**)&term))
        amcw_addvar(solver, row, amcw_key(term), term->multiplier*multiplier);
}

static void amcw_solvefor(amcw_Solver *solver, amcw_Row *row, amcw_Symbol entry, amcw_Symbol exit) {
    amcw_Term *term = (amcw_Term*)amcw_gettable(&row->terms, entry);
    amcw_Num reciprocal = 1.0f / term->multiplier;
    assert(entry.id != exit.id && !amcw_nearzero(term->multiplier));
    amcw_delkey(&row->terms, &term->entry);
    amcw_multiply(row, -reciprocal);
    if (exit.id != 0) amcw_addvar(solver, row, exit, reciprocal);
}

static void amcw_substitute(amcw_Solver *solver, amcw_Row *row, amcw_Symbol entry, const amcw_Row *other) {
    amcw_Term *term = (amcw_Term*)amcw_gettable(&row->terms, entry);
    if (!term) return;
    amcw_delkey(&row->terms, &term->entry);
    amcw_addrow(solver, row, other, term->multiplier);
}


/* variables & constraints */

AMCW_API int amcw_variableid(amcw_Var *var) { return var ? var->sym.id : -1; }
AMCW_API amcw_Num amcw_value(amcw_Var *var) { return var ? var->value : 0.0f; }
AMCW_API void amcw_usevariable(amcw_Var *var) { if (var) ++var->refcount; }

static amcw_Var *amcw_sym2var(amcw_Solver *solver, amcw_Symbol sym) {
    amcw_VarEntry *ve = (amcw_VarEntry*)amcw_gettable(&solver->vars, sym);
    assert(ve != NULL);
    return ve->var;
}

AMCW_API amcw_Var *amcw_newvariable(amcw_Solver *solver) {
    amcw_Var *var = (amcw_Var*)amcw_alloc(solver, &solver->varpool);
    amcw_Symbol sym = amcw_newsymbol(solver, AMCW_EXTERNAL);
    amcw_VarEntry *ve = (amcw_VarEntry*)amcw_settable(solver, &solver->vars, sym);
    assert(ve->var == NULL);
    memset(var, 0, sizeof(*var));
    var->sym      = sym;
    var->refcount = 1;
    var->solver   = solver;
    ve->var       = var;
    return var;
}

AMCW_API void amcw_delvariable(amcw_Var *var) {
    if (var && --var->refcount <= 0) {
        amcw_Solver *solver = var->solver;
        amcw_VarEntry *e = (amcw_VarEntry*)amcw_gettable(&solver->vars, var->sym);
        assert(e != NULL);
        amcw_delkey(&solver->vars, &e->entry);
        amcw_remove(var->constraint);
        amcw_free(&solver->varpool, var);
    }
}

AMCW_API amcw_Constraint *amcw_newconstraint(amcw_Solver *solver, amcw_Num strength) {
    amcw_Constraint *cons = (amcw_Constraint*)amcw_alloc(solver, &solver->conspool);
    memset(cons, 0, sizeof(*cons));
    cons->solver   = solver;
    cons->strength = amcw_nearzero(strength) ? AMCW_REQUIRED : strength;
    amcw_initrow(&cons->expression);
    amcw_key(cons).id = ++solver->constraint_count;
    amcw_key(cons).type = AMCW_EXTERNAL;
    ((amcw_ConsEntry*)amcw_settable(solver, &solver->constraints,
        amcw_key(cons)))->constraint = cons;
    return cons;
}

AMCW_API void amcw_delconstraint(amcw_Constraint *cons) {
    amcw_Solver *solver = cons ? cons->solver : NULL;
    amcw_Term *term = NULL;
    amcw_ConsEntry *ce;
    if (cons == NULL) return;
    amcw_remove(cons);
    ce = (amcw_ConsEntry*)amcw_gettable(&solver->constraints, amcw_key(cons));
    assert(ce != NULL);
    amcw_delkey(&solver->constraints, &ce->entry);
    while (amcw_nextentry(&cons->expression.terms, (amcw_Entry**)&term))
        amcw_delvariable(amcw_sym2var(solver, amcw_key(term)));
    amcw_freerow(solver, &cons->expression);
    amcw_free(&solver->conspool, cons);
}

AMCW_API amcw_Constraint *amcw_cloneconstraint(amcw_Constraint *other, amcw_Num strength) {
    amcw_Constraint *cons;
    if (other == NULL) return NULL;
    cons = amcw_newconstraint(other->solver,
            amcw_nearzero(strength) ? other->strength : strength);
    amcw_mergeconstraint(cons, other, 1.0f);
    cons->relation = other->relation;
    return cons;
}

AMCW_API int amcw_mergeconstraint(amcw_Constraint *cons, const amcw_Constraint *other, amcw_Num multiplier) {
    amcw_Term *term = NULL;
    if (cons == NULL || other == NULL || cons->marker.id != 0
            || cons->solver != other->solver) return AMCW_FAILED;
    if (cons->relation == AMCW_GREATEQUAL) multiplier = -multiplier;
    cons->expression.constant += other->expression.constant*multiplier;
    while (amcw_nextentry(&other->expression.terms, (amcw_Entry**)&term)) {
        amcw_usevariable(amcw_sym2var(cons->solver, amcw_key(term)));
        amcw_addvar(cons->solver, &cons->expression, amcw_key(term),
                term->multiplier*multiplier);
    }
    return AMCW_OK;
}

AMCW_API void amcw_resetconstraint(amcw_Constraint *cons) {
    amcw_Term *term = NULL;
    if (cons == NULL) return;
    amcw_remove(cons);
    cons->relation = 0;
    while (amcw_nextentry(&cons->expression.terms, (amcw_Entry**)&term))
        amcw_delvariable(amcw_sym2var(cons->solver, amcw_key(term)));
    amcw_resetrow(&cons->expression);
}

AMCW_API int amcw_addterm(amcw_Constraint *cons, amcw_Var *var, amcw_Num multiplier) {
    if (cons == NULL || var == NULL || cons->marker.id != 0 ||
            cons->solver != var->solver) return AMCW_FAILED;
    assert(var->sym.id != 0);
    assert(var->solver == cons->solver);
    if (cons->relation == AMCW_GREATEQUAL) multiplier = -multiplier;
    amcw_addvar(cons->solver, &cons->expression, var->sym, multiplier);
    amcw_usevariable(var);
    return AMCW_OK;
}

AMCW_API int amcw_addconstant(amcw_Constraint *cons, amcw_Num constant) {
    if (cons == NULL || cons->marker.id != 0) return AMCW_FAILED;
    cons->expression.constant +=
        cons->relation == AMCW_GREATEQUAL ? -constant : constant;
    return AMCW_OK;
}

AMCW_API int amcw_setrelation(amcw_Constraint *cons, int relation) {
    assert(relation >= AMCW_LESSEQUAL && relation <= AMCW_GREATEQUAL);
    if (cons == NULL || cons->marker.id != 0 || cons->relation != 0)
        return AMCW_FAILED;
    if (relation != AMCW_GREATEQUAL) amcw_multiply(&cons->expression, -1.0f);
    cons->relation = relation;
    return AMCW_OK;
}


/* Cassowary algorithm */

AMCW_API int amcw_hasedit(amcw_Var *var)
{ return var != NULL && var->constraint != NULL; }

AMCW_API int amcw_hasconstraint(amcw_Constraint *cons)
{ return cons != NULL && cons->marker.id != 0; }

AMCW_API void amcw_autoupdate(amcw_Solver *solver, int auto_update)
{ solver->auto_update = auto_update; }

static void amcw_infeasible(amcw_Solver *solver, amcw_Row *row) {
    if (row->constant < 0.0f && !amcw_isdummy(row->infeasible_next)) {
        row->infeasible_next.id = solver->infeasible_rows.id;
        row->infeasible_next.type = AMCW_DUMMY;
        solver->infeasible_rows = amcw_key(row);
    }
}

static void amcw_markdirty(amcw_Solver *solver, amcw_Var *var) {
    if (var->dirty_next.type == AMCW_DUMMY) return;
    var->dirty_next.id = solver->dirty_vars.id;
    var->dirty_next.type = AMCW_DUMMY;
    solver->dirty_vars = var->sym;
}

static void amcw_substitute_rows(amcw_Solver *solver, amcw_Symbol var, amcw_Row *expr) {
    amcw_Row *row = NULL;
    while (amcw_nextentry(&solver->rows, (amcw_Entry**)&row)) {
        amcw_substitute(solver, row, var, expr);
        if (amcw_isexternal(amcw_key(row)))
            amcw_markdirty(solver, amcw_sym2var(solver, amcw_key(row)));
        else
            amcw_infeasible(solver, row);
    }
    amcw_substitute(solver, &solver->objective, var, expr);
}

static int amcw_takerow(amcw_Solver *solver, amcw_Symbol sym, amcw_Row *dst) {
    amcw_Row *row = (amcw_Row*)amcw_gettable(&solver->rows, sym);
    amcw_key(dst) = amcw_null();
    if (row == NULL) return AMCW_FAILED;
    amcw_delkey(&solver->rows, &row->entry);
    dst->constant   = row->constant;
    dst->terms      = row->terms;
    return AMCW_OK;
}

static int amcw_putrow(amcw_Solver *solver, amcw_Symbol sym, const amcw_Row *src) {
    amcw_Row *row = (amcw_Row*)amcw_settable(solver, &solver->rows, sym);
    row->constant = src->constant;
    row->terms    = src->terms;
    return AMCW_OK;
}

static void amcw_mergerow(amcw_Solver *solver, amcw_Row *row, amcw_Symbol var, amcw_Num multiplier) {
    amcw_Row *oldrow = (amcw_Row*)amcw_gettable(&solver->rows, var);
    if (oldrow)
        amcw_addrow(solver, row, oldrow, multiplier);
    else
        amcw_addvar(solver, row, var, multiplier);
}

static int amcw_optimize(amcw_Solver *solver, amcw_Row *objective) {
    for (;;) {
        amcw_Symbol enter = amcw_null(), exit = amcw_null();
        amcw_Num r, min_ratio = AMCW_NUM_MAX;
        amcw_Row tmp, *row = NULL;
        amcw_Term *term = NULL;

        assert(solver->infeasible_rows.id == 0);
        while (amcw_nextentry(&objective->terms, (amcw_Entry**)&term)) {
            if (!amcw_isdummy(amcw_key(term)) && term->multiplier < 0.0f)
            { enter = amcw_key(term); break; }
        }
        if (enter.id == 0) return AMCW_OK;

        while (amcw_nextentry(&solver->rows, (amcw_Entry**)&row)) {
            term = (amcw_Term*)amcw_gettable(&row->terms, enter);
            if (term == NULL || !amcw_ispivotable(amcw_key(row))
                    || term->multiplier > 0.0f) continue;
            r = -row->constant / term->multiplier;
            if (r < min_ratio || (amcw_approx(r, min_ratio)
                        && amcw_key(row).id < exit.id))
                min_ratio = r, exit = amcw_key(row);
        }
        assert(exit.id != 0);
        if (exit.id == 0) return AMCW_FAILED;

        amcw_takerow(solver, exit, &tmp);
        amcw_solvefor(solver, &tmp, enter, exit);
        amcw_substitute_rows(solver, enter, &tmp);
        if (objective != &solver->objective)
            amcw_substitute(solver, objective, enter, &tmp);
        amcw_putrow(solver, enter, &tmp);
    }
}

static amcw_Row amcw_makerow(amcw_Solver *solver, amcw_Constraint *cons) {
    amcw_Term *term = NULL;
    amcw_Row row;
    amcw_initrow(&row);
    row.constant = cons->expression.constant;
    while (amcw_nextentry(&cons->expression.terms, (amcw_Entry**)&term)) {
        amcw_markdirty(solver, amcw_sym2var(solver, amcw_key(term)));
        amcw_mergerow(solver, &row, amcw_key(term), term->multiplier);
    }
    if (cons->relation != AMCW_EQUAL) {
        amcw_initsymbol(solver, &cons->marker, AMCW_SLACK);
        amcw_addvar(solver, &row, cons->marker, -1.0f);
        if (cons->strength < AMCW_REQUIRED) {
            amcw_initsymbol(solver, &cons->other, AMCW_ERROR);
            amcw_addvar(solver, &row, cons->other, 1.0f);
            amcw_addvar(solver, &solver->objective, cons->other, cons->strength);
        }
    } else if (cons->strength >= AMCW_REQUIRED) {
        amcw_initsymbol(solver, &cons->marker, AMCW_DUMMY);
        amcw_addvar(solver, &row, cons->marker, 1.0f);
    } else {
        amcw_initsymbol(solver, &cons->marker, AMCW_ERROR);
        amcw_initsymbol(solver, &cons->other,  AMCW_ERROR);
        amcw_addvar(solver, &row, cons->marker, -1.0f);
        amcw_addvar(solver, &row, cons->other,   1.0f);
        amcw_addvar(solver, &solver->objective, cons->marker, cons->strength);
        amcw_addvar(solver, &solver->objective, cons->other,  cons->strength);
    }
    if (row.constant < 0.0f) amcw_multiply(&row, -1.0f);
    return row;
}

static void amcw_remove_errors(amcw_Solver *solver, amcw_Constraint *cons) {
    if (amcw_iserror(cons->marker))
        amcw_mergerow(solver, &solver->objective, cons->marker, -cons->strength);
    if (amcw_iserror(cons->other))
        amcw_mergerow(solver, &solver->objective, cons->other, -cons->strength);
    if (amcw_isconstant(&solver->objective))
        solver->objective.constant = 0.0f;
    cons->marker = cons->other = amcw_null();
}

static int amcw_add_with_artificial(amcw_Solver *solver, amcw_Row *row, amcw_Constraint *cons) {
    amcw_Symbol a = amcw_newsymbol(solver, AMCW_SLACK);
    amcw_Term *term = NULL;
    amcw_Row tmp;
    int ret;
    --solver->symbol_count; /* artificial variable will be removed */
    amcw_initrow(&tmp);
    amcw_addrow(solver, &tmp, row, 1.0f);
    amcw_putrow(solver, a, row);
    amcw_initrow(row), row = NULL; /* row is useless */
    amcw_optimize(solver, &tmp);
    ret = amcw_nearzero(tmp.constant) ? AMCW_OK : AMCW_UNBOUND;
    amcw_freerow(solver, &tmp);
    if (amcw_takerow(solver, a, &tmp) == AMCW_OK) {
        amcw_Symbol entry = amcw_null();
        if (amcw_isconstant(&tmp)) { amcw_freerow(solver, &tmp); return ret; }
        while (amcw_nextentry(&tmp.terms, (amcw_Entry**)&term))
            if (amcw_ispivotable(amcw_key(term))) { entry = amcw_key(term); break; }
        if (entry.id == 0) { amcw_freerow(solver, &tmp); return AMCW_UNBOUND; }
        amcw_solvefor(solver, &tmp, entry, a);
        amcw_substitute_rows(solver, entry, &tmp);
        amcw_putrow(solver, entry, &tmp);
    }
    while (amcw_nextentry(&solver->rows, (amcw_Entry**)&row)) {
        term = (amcw_Term*)amcw_gettable(&row->terms, a);
        if (term) amcw_delkey(&row->terms, &term->entry);
    }
    term = (amcw_Term*)amcw_gettable(&solver->objective.terms, a);
    if (term) amcw_delkey(&solver->objective.terms, &term->entry);
    if (ret != AMCW_OK) amcw_remove(cons);
    return ret;
}

static int amcw_try_addrow(amcw_Solver *solver, amcw_Row *row, amcw_Constraint *cons) {
    amcw_Symbol subject = amcw_null();
    amcw_Term *term = NULL;
    while (amcw_nextentry(&row->terms, (amcw_Entry**)&term))
        if (amcw_isexternal(amcw_key(term))) { subject = amcw_key(term); break; }
    if (subject.id == 0 && amcw_ispivotable(cons->marker)) {
        amcw_Term *mterm = (amcw_Term*)amcw_gettable(&row->terms, cons->marker);
        if (mterm->multiplier < 0.0f) subject = cons->marker;
    }
    if (subject.id == 0 && amcw_ispivotable(cons->other)) {
        amcw_Term *mterm = (amcw_Term*)amcw_gettable(&row->terms, cons->other);
        if (mterm->multiplier < 0.0f) subject = cons->other;
    }
    if (subject.id == 0) {
        while (amcw_nextentry(&row->terms, (amcw_Entry**)&term))
            if (!amcw_isdummy(amcw_key(term))) break;
        if (term == NULL) {
            if (amcw_nearzero(row->constant))
                subject = cons->marker;
            else {
                amcw_freerow(solver, row);
                return AMCW_UNSATISFIED;
            }
        }
    }
    if (subject.id == 0)
        return amcw_add_with_artificial(solver, row, cons);
    amcw_solvefor(solver, row, subject, amcw_null());
    amcw_substitute_rows(solver, subject, row);
    amcw_putrow(solver, subject, row);
    return AMCW_OK;
}

static amcw_Symbol amcw_get_leaving_row(amcw_Solver *solver, amcw_Symbol marker) {
    amcw_Symbol first = amcw_null(), second = amcw_null(), third = amcw_null();
    amcw_Num r1 = AMCW_NUM_MAX, r2 = AMCW_NUM_MAX;
    amcw_Row *row = NULL;
    while (amcw_nextentry(&solver->rows, (amcw_Entry**)&row)) {
        amcw_Term *term = (amcw_Term*)amcw_gettable(&row->terms, marker);
        if (term == NULL) continue;
        if (amcw_isexternal(amcw_key(row)))
            third = amcw_key(row);
        else if (term->multiplier < 0.0f) {
            amcw_Num r = -row->constant / term->multiplier;
            if (r < r1) r1 = r, first = amcw_key(row);
        } else {
            amcw_Num r = row->constant / term->multiplier;
            if (r < r2) r2 = r, second = amcw_key(row);
        }
    }
    return first.id ? first : second.id ? second : third;
}

static void amcw_delta_edit_constant(amcw_Solver *solver, amcw_Num delta, amcw_Constraint *cons) {
    amcw_Row *row;
    if ((row = (amcw_Row*)amcw_gettable(&solver->rows, cons->marker)) != NULL)
    { row->constant -= delta; amcw_infeasible(solver, row); return; }
    if ((row = (amcw_Row*)amcw_gettable(&solver->rows, cons->other)) != NULL)
    { row->constant += delta; amcw_infeasible(solver, row); return; }
    while (amcw_nextentry(&solver->rows, (amcw_Entry**)&row)) {
        amcw_Term *term = (amcw_Term*)amcw_gettable(&row->terms, cons->marker);
        if (term == NULL) continue;
        row->constant += term->multiplier*delta;
        if (amcw_isexternal(amcw_key(row)))
            amcw_markdirty(solver, amcw_sym2var(solver, amcw_key(row)));
        else
            amcw_infeasible(solver, row);
    }
}

static void amcw_dual_optimize(amcw_Solver *solver) {
    while (solver->infeasible_rows.id != 0) {
        amcw_Symbol cur, enter = amcw_null(), leave;
        amcw_Term *objterm, *term = NULL;
        amcw_Num r, min_ratio = AMCW_NUM_MAX;
        amcw_Row tmp, *row =
            (amcw_Row*)amcw_gettable(&solver->rows, solver->infeasible_rows);
        assert(row != NULL);
        leave = amcw_key(row);
        solver->infeasible_rows = row->infeasible_next;
        row->infeasible_next = amcw_null();
        if (amcw_nearzero(row->constant) || row->constant >= 0.0f) continue;
        while (amcw_nextentry(&row->terms, (amcw_Entry**)&term)) {
            if (amcw_isdummy(cur = amcw_key(term)) || term->multiplier <= 0.0f)
                continue;
            objterm = (amcw_Term*)amcw_gettable(&solver->objective.terms, cur);
            r = objterm ? objterm->multiplier / term->multiplier : 0.0f;
            if (min_ratio > r) min_ratio = r, enter = cur;
        }
        assert(enter.id != 0);
        amcw_takerow(solver, leave, &tmp);
        amcw_solvefor(solver, &tmp, enter, leave);
        amcw_substitute_rows(solver, enter, &tmp);
        amcw_putrow(solver, enter, &tmp);
    }
}

static void *amcw_default_allocf(void *ud, void *ptr, size_t nsize, size_t osize) {
    void *newptr;
    (void)ud, (void)osize;
    if (nsize == 0) { free(ptr); return NULL; }
    newptr = realloc(ptr, nsize);
    if (newptr == NULL) abort();
    return newptr;
}

AMCW_API amcw_Solver *amcw_newsolver(amcw_Allocf *allocf, void *ud) {
    amcw_Solver *solver;
    if (allocf == NULL) allocf = amcw_default_allocf;
    if ((solver = (amcw_Solver*)allocf(ud, NULL, sizeof(amcw_Solver), 0)) == NULL)
        return NULL;
    memset(solver, 0, sizeof(*solver));
    solver->allocf = allocf;
    solver->ud     = ud;
    amcw_initrow(&solver->objective);
    amcw_inittable(&solver->vars, sizeof(amcw_VarEntry));
    amcw_inittable(&solver->constraints, sizeof(amcw_ConsEntry));
    amcw_inittable(&solver->rows, sizeof(amcw_Row));
    amcw_initpool(&solver->varpool, sizeof(amcw_Var));
    amcw_initpool(&solver->conspool, sizeof(amcw_Constraint));
    return solver;
}

AMCW_API void amcw_delsolver(amcw_Solver *solver) {
    amcw_ConsEntry *ce = NULL;
    amcw_Row *row = NULL;
    while (amcw_nextentry(&solver->constraints, (amcw_Entry**)&ce))
        amcw_freerow(solver, &ce->constraint->expression);
    while (amcw_nextentry(&solver->rows, (amcw_Entry**)&row))
        amcw_freerow(solver, row);
    amcw_freerow(solver, &solver->objective);
    amcw_freetable(solver, &solver->vars);
    amcw_freetable(solver, &solver->constraints);
    amcw_freetable(solver, &solver->rows);
    amcw_freepool(solver, &solver->varpool);
    amcw_freepool(solver, &solver->conspool);
    solver->allocf(solver->ud, solver, 0, sizeof(*solver));
}

AMCW_API void amcw_resetsolver(amcw_Solver *solver, int clear_constraints) {
    amcw_Entry *entry = NULL;
    if (!solver->auto_update) amcw_updatevars(solver);
    while (amcw_nextentry(&solver->vars, &entry)) {
        amcw_Constraint **cons = &((amcw_VarEntry*)entry)->var->constraint;
        amcw_remove(*cons);
        *cons = NULL;
    }
    assert(amcw_nearzero(solver->objective.constant));
    assert(solver->infeasible_rows.id == 0);
    assert(solver->dirty_vars.id == 0);
    if (!clear_constraints) return;
    amcw_resetrow(&solver->objective);
    while (amcw_nextentry(&solver->constraints, &entry)) {
        amcw_Constraint *cons = ((amcw_ConsEntry*)entry)->constraint;
        if (cons->marker.id != 0)
            cons->marker = cons->other = amcw_null();
    }
    while (amcw_nextentry(&solver->rows, &entry)) {
        amcw_delkey(&solver->rows, entry);
        amcw_freerow(solver, (amcw_Row*)entry);
    }
}

AMCW_API void amcw_updatevars(amcw_Solver *solver) {
    while (solver->dirty_vars.id != 0) {
        amcw_Var *var = amcw_sym2var(solver, solver->dirty_vars);
        amcw_Row *row = (amcw_Row*)amcw_gettable(&solver->rows, var->sym);
        solver->dirty_vars = var->dirty_next;
        var->dirty_next = amcw_null();
        var->value = row ? row->constant : 0.0f;
    }
}

AMCW_API int amcw_add(amcw_Constraint *cons) {
    amcw_Solver *solver = cons ? cons->solver : NULL;
    int ret, oldsym = solver ? solver->symbol_count : 0;
    amcw_Row row;
    if (solver == NULL || cons->marker.id != 0) return AMCW_FAILED;
    row = amcw_makerow(solver, cons);
    if ((ret = amcw_try_addrow(solver, &row, cons)) != AMCW_OK) {
        amcw_remove_errors(solver, cons);
        solver->symbol_count = oldsym;
    } else {
        amcw_optimize(solver, &solver->objective);
        if (solver->auto_update) amcw_updatevars(solver);
    }
    assert(solver->infeasible_rows.id == 0);
    return ret;
}

AMCW_API void amcw_remove(amcw_Constraint *cons) {
    amcw_Solver *solver;
    amcw_Symbol marker;
    amcw_Row tmp;
    if (cons == NULL || cons->marker.id == 0) return;
    solver = cons->solver, marker = cons->marker;
    amcw_remove_errors(solver, cons);
    if (amcw_takerow(solver, marker, &tmp) != AMCW_OK) {
        amcw_Symbol exit = amcw_get_leaving_row(solver, marker);
        assert(exit.id != 0);
        amcw_takerow(solver, exit, &tmp);
        amcw_solvefor(solver, &tmp, marker, exit);
        amcw_substitute_rows(solver, marker, &tmp);
    }
    amcw_freerow(solver, &tmp);
    amcw_optimize(solver, &solver->objective);
    if (solver->auto_update) amcw_updatevars(solver);
}

AMCW_API int amcw_setstrength(amcw_Constraint *cons, amcw_Num strength) {
    if (cons == NULL) return AMCW_FAILED;
    strength = amcw_nearzero(strength) ? AMCW_REQUIRED : strength;
    if (cons->strength == strength) return AMCW_OK;
    if (cons->strength >= AMCW_REQUIRED || strength >= AMCW_REQUIRED)
    { amcw_remove(cons), cons->strength = strength; return amcw_add(cons); }
    if (cons->marker.id != 0) {
        amcw_Solver *solver = cons->solver;
        amcw_Num diff = strength - cons->strength;
        amcw_mergerow(solver, &solver->objective, cons->marker, diff);
        amcw_mergerow(solver, &solver->objective, cons->other,  diff);
        amcw_optimize(solver, &solver->objective);
        if (solver->auto_update) amcw_updatevars(solver);
    }
    cons->strength = strength;
    return AMCW_OK;
}

AMCW_API int amcw_addedit(amcw_Var *var, amcw_Num strength) {
    amcw_Solver *solver = var ? var->solver : NULL;
    amcw_Constraint *cons;
    if (var == NULL) return AMCW_FAILED;
    if (strength >= AMCW_STRONG) strength = AMCW_STRONG;
    if (var->constraint) return amcw_setstrength(var->constraint, strength);
    assert(var->sym.id != 0);
    cons = amcw_newconstraint(solver, strength);
    amcw_setrelation(cons, AMCW_EQUAL);
    amcw_addterm(cons, var, 1.0f); /* var must have positive signture */
    amcw_addconstant(cons, -var->value);
    if (amcw_add(cons) != AMCW_OK) assert(0);
    var->constraint = cons;
    var->edit_value = var->value;
    return AMCW_OK;
}

AMCW_API void amcw_deledit(amcw_Var *var) {
    if (var == NULL || var->constraint == NULL) return;
    amcw_delconstraint(var->constraint);
    var->constraint = NULL;
    var->edit_value = 0.0f;
}

AMCW_API void amcw_suggest(amcw_Var *var, amcw_Num value) {
    amcw_Solver *solver = var ? var->solver : NULL;
    amcw_Num delta;
    if (var == NULL) return;
    if (var->constraint == NULL) {
        amcw_addedit(var, AMCW_MEDIUM);
        assert(var->constraint != NULL);
    }
    delta = value - var->edit_value;
    var->edit_value = value;
    amcw_delta_edit_constant(solver, delta, var->constraint);
    amcw_dual_optimize(solver);
    if (solver->auto_update) amcw_updatevars(solver);
}


AMCW_NS_END

#endif /* AMCW_IMPLEMENTATION */

/* cc: flags+='-shared -O2 -DAMCW_IMPLEMENTATION -xc'
   unixcc: output='amcw_cassowary.so'
   win32cc: output='amcw_cassowary.dll' */

