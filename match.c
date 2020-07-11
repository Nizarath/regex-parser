#include <stdio.h>
#include <stdlib.h>
#include "routine.h"
#include "nfa.h"
#include "match.h"

/* alloc */

static pos *posalloc(nfa *nfa, state *s)
{
	pos *p;

	p = malloc(sizeof(*p));
	if (p == NULL) {
		fprintf(stderr, "position alloc error\n");
		exit(ERR_ALLOC);
	}
	p->s = s;
	p->prv = NULL;
	p->nxt = nfa->poslist;
	if (nfa->poslist)
		nfa->poslist->prv = p;
	nfa->poslist = p;

	return p;
}

/* nfa is needed to update poslist */
static void posfree(nfa *nfa, pos *p)
{
	if (p->prv)
		p->prv->nxt = p->nxt;
	else
		nfa->poslist = p->nxt;

	if (p->nxt)
		p->nxt->prv = p->prv;

	free(p);
}

/* 
  visit/leave

  recursively visit all E-connected states 
  (but leave only one, since Si may have arrows for char x while
  E-connected Sj does not)
 
  these functions do not keep states visited between calls, allowing
  multiple positions for one state (positions move independently,
  representing mutually exclusive ways),
  but they mark visiting inside each call to avoid infinite recursion
  (same as prnfa), so they lock from the same function call, not from
  generic algorithm

  Also, lock is checked here, not by the caller
 */

static void visit(nfa *nfa, state *s)
{
	register trans *t;

	if (s->mark_vis)	/* check locking */
		return;
	s->mark_vis = 1;

	posalloc(nfa, s);	/* visit */

	for (t = s->trlist; t != NULL; t = t->nxt)
		if (t->c == EMPTY_TRANS)
			visit(nfa, t->to);

	s->mark_vis = 0;	/* release lock */
}

/* redundant, generic interface (but static) */
static void leave(nfa *nfa, pos *p)
{
	posfree(nfa, p);
}

/* leave all before next NFA launch */
static void aleave(nfa *nfa)
{
	register pos *p, *np;

	for (p = nfa->poslist; p != NULL; p = np) {
		np = p->nxt;
		free(p);
	}
	nfa->poslist = NULL;
}

/* show positions */
static void prpos(nfa *nfa)
{
	register pos *p;

	printf("pos: ");
	for (p = nfa->poslist; p != NULL; p = p->nxt)
		printf("S%d ", p->s->n);
	putchar('\n');
}

/* 
  the only interface to the external world,
  check if NFA allows string
 */

int match(nfa *nfa, const char *l)
{
	register trans *t;
	register pos *p, *np;
	state *s;

	for (visit(nfa, nfa->beg); *l; l++) {
		for (p = nfa->poslist; p != NULL; p = np) {
				/* remove here or after loop */
			s = p->s, np = p->nxt;
			leave(nfa, p);
				/* move */
			for (t = s->trlist; t != NULL; t = t->nxt)
				if (t->c == *l)
					visit(nfa, t->to);
		}
		if (nfa->poslist == NULL)
			goto nomatch;
	}
	for (p = nfa->poslist; p != NULL; p = p->nxt)
		if (p->s->isfin) {
			aleave(nfa);
			return 1;
		}
nomatch:
	aleave(nfa);
	return 0;
}
