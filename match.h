#ifndef _MATCH_H
#define _MATCH_H

#include "nfa.h"

typedef struct pos {
	struct state *s;	/* state visited */
	struct pos *nxt;	/* another pos for the same automata */
	struct pos *prv;	/* removal ease */
} pos;

/* the only interface */
int match(nfa *nfa, const char *s);

#endif
