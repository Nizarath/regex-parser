#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routine.h"
#include "nfa.h"
#include "type.h"

/* ------------ alloc -------------- */

static int st_num = 0;		/* ensures set index integrity,
				   however, lost are lost */

static state *stalloc(void)
{
	state *s;

	s = malloc(sizeof(*s));
	if (s == NULL) {
		fprintf(stderr, "state alloc error\n");
		exit(ERR_ALLOC);
	}

	s->n = st_num++;
	s->isfin = 0;
	s->trlist = NULL;
	s->mark_vis = 0;

	return s;
}

/* creates arrow and also connects */
static trans *tralloc(char c, state *s1, state *s2)
{
	trans *t;

	t = malloc(sizeof(*t));
	if (t == NULL) {
		fprintf(stderr, "arrow alloc error\n");
		exit(ERR_ALLOC);
	}
	t->c = c;
	t->nxt = s1->trlist;
	t->to = s2;	/* upd. one state */
	s1->trlist = t;

	return t;
}

static nfa *nfalloc(state *sb, state *se)
{
	nfa *nfa;

	nfa = malloc(sizeof(*nfa));
	if (nfa == NULL) {
		fprintf(stderr, "automata alloc error\n");
		exit(ERR_ALLOC);
	}
	nfa->beg = sb;
	nfa->end = se;
	nfa->poslist = NULL;

	return nfa;
}

/* ---- create partial automatas ----- */

static nfa *litnfa(char c)	/* NFA for literal */
{
	state *s1, *s2;
	trans *t;

	s1 = stalloc();
	s2 = stalloc();
	t = tralloc(c, s1, s2);

	return nfalloc(s1, s2);
}

static nfa *qnfa(nfa *nfa)	/* NFA for ? */
{
	tralloc(EMPTY_TRANS, nfa->beg, nfa->end);

	return nfa;
}

static nfa *pnfa(nfa *nfa)	/* NFA for + */
{
	tralloc(EMPTY_TRANS, nfa->end, nfa->beg);

	return nfa;
}

static nfa *mnfa(nfa *nfa)	/* NFA for * */
{
	tralloc(EMPTY_TRANS, nfa->beg, nfa->end);
	tralloc(EMPTY_TRANS, nfa->end, nfa->beg);

	return nfa;
}

/* easily redoneable to reusage of one nfa */
static nfa *ornfa(nfa *nfa1, nfa *nfa2)	/* NFA for | */
{
	state *sb, *se;

	sb = stalloc();
	se = stalloc();
	tralloc(EMPTY_TRANS, sb, nfa1->beg);
	tralloc(EMPTY_TRANS, sb, nfa2->beg);
	tralloc(EMPTY_TRANS, nfa1->end, se);
	tralloc(EMPTY_TRANS, nfa2->end, se);
	free(nfa1);			/* poslists are empty yet */
	free(nfa2);

	return nfalloc(sb, se);		/* can be before to check */
}

/* easily redoneable to reusage of one nfa */
static nfa *confa(nfa *nfa1, nfa *nfa2)	/* concatenation of NFAs */
{
	nfa *nfa;

	nfa = nfalloc(nfa1->beg, nfa2->end);
	tralloc(EMPTY_TRANS, nfa1->end, nfa2->beg);
	free(nfa1);
	free(nfa2);

	return nfa;
}

/* ---------- create automata on stack --------- */

	/* automata stack */
#define MAXNFA		100
static nfa *nfas[MAXNFA];
static int top = -1;		/* should be -1 between calls,
				   the reverse can detect errors */

static void pushnfa(nfa *nfa)
{
	if (top == MAXNFA-1) {
		fprintf(stderr, "nfa push error\n");
		exit(ERR_STACK);
	}
	nfas[++top] = nfa;
}

static nfa *popnfa(void)
{
	if (top == -1) {
		fprintf(stderr, "nfa pop error\n");
		exit(ERR_STACK);
	}
	return nfas[top--];
}

/* 
   normalized postfix regex from updregex -> automata
 */
nfa *mknfa(const char *regex)
{
	nfa *nfa;

	for (; *regex; regex++)
		if (islit(*regex))
			pushnfa(litnfa(*regex));
		else if (isop(*regex))
			switch (*regex) {
				/* unary */
				case '?':
					pushnfa(qnfa(popnfa()));
					break;
				case '*':
					pushnfa(mnfa(popnfa()));
					break;
				case '+':
					pushnfa(pnfa(popnfa()));
					break;
				/* binary */
				case '|':
					nfa = popnfa();
					pushnfa(ornfa(popnfa(), nfa));
					break;
				case OP_CONCAT:
					nfa = popnfa();
					pushnfa(confa(popnfa(), nfa));
					break;
				/* no default allowed by isnop */
			}
		else {
			fprintf(stderr, "%c code %d: unknown regex char to"
					"build NFA\n", *regex, *regex);
			exit(ERR_INPUT);
		}
		/* fetch NFA from stack */
	nfa = popnfa();
	if (top != -1) {
		fprintf(stderr, "after mknfa: NFA stack corrupted\n");
		exit(ERR_STACK);
	}
		/* marking only the last state */
	nfa->end->isfin = 1;

	return nfa;
}

/* ------- printing nfa --------- */

static void prst(state *s)	/* cannot const since mark_vis update */
{
	register trans *t;

			/* visit once */
	if (s->mark_vis)
		return;
	s->mark_vis = 1;
			/* printing part */
	printf("S%d%s\n", s->n, s->isfin ? " (FINAL):" : ":");
	for (t = s->trlist; t != NULL; t = t->nxt) {
		printf("%c-->S%d\n", t->c, t->to->n);
	}
			/* invoke others (mess if in the same loop) */
	for (t = s->trlist; t != NULL; t = t->nxt)
		prst(t->to);
			/* clear visit mark */
	s->mark_vis = 0;
}

void prnfa(const nfa *nfa)
{
	prst(nfa->beg);	/* recursion */
}
