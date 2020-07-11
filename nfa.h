#ifndef _NFA_H
#define _NFA_H

#define EMPTY_TRANS	0	/* E-transition */

/* typedef in any order (no typedef inside structs) */

typedef struct state {
	int n;		/* index */
	int isfin;	/* is this state final? */
	struct trans *trlist;	/* all arrows from this state */
		/* different state marks, expandable */
	int mark_vis;	/* has been visited? (avoid infinite recursion) */
} state;

typedef struct {
	struct pos *poslist;	/* list of positions in this automata */
	struct state *beg;	/* first state */
	struct state *end;	/* end state (not necessarily final) */
} nfa;

typedef struct trans {
	char c;			/* symbol (can be EMPTY_TRANS) */
	struct state *to;	/* state where moving */
	/* struct state *from; */ /* redundant */
	struct trans *nxt;	/* another transition from the same state */
} trans;

/* the only interface */
nfa *mknfa(const char *regex);
void prnfa(const nfa *nfa);

#endif
