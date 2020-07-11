/*
 
 language formula is as follows:
 EXPR = LIT
 EXPR = EXPR UNOP
 EXPR = EXPR BINOP EXPR
 EXPR = (EXPR)

 or

 EXPR = <LIT> | <EXPR UNOP> | <EXPR BINOP EXPR> | <(EXPR)>

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routine.h"
#include "type.h"

/* ------------- alloc ------------ */

static char *alloc(const char *s1, unsigned mult, int add)
{
	char *s2;

	s2 = malloc(strlen(s1)*mult + add);
	if (s2 == NULL) {
		fprintf(stderr, "string alloc fail\n");
		exit(ERR_ALLOC);
	}
	strcpy(s2, s1);

	return s2;
}

/* ----------- stage 0 -- check input ------------ */

static void icheck(const char *regex)
{
	register int par;

	for (par = 0; *regex; regex++) {
		if (*regex == '(')
			par++;
		else if (*regex == ')')
			par--;
		else if (!isch(*regex)) {
			fprintf(stderr, "incorrect char %c code %d\n", 
				*regex, *regex);
			exit(ERR_INPUT);
		}
	}
	if (par) {
		fprintf(stderr, "() mismatch\n");
		exit(ERR_INPUT);
	}

	/* OK */
}

/* ------------ stage 1 -- rm blanks ------------- */

static char *rmblank(const char *regex)
{
	char *nregex;
	register char *nr;

	nregex = alloc(regex, 1, 0);
	for (nr = nregex; *regex; regex++)
		if (!isblnk(*regex))
			*nr++ = *regex;
	*nr = '\0';

	return nregex;
}

/* ------------ stage 2 -- concat -------------- 

   LIT/UNOP/) LIT/( 
   ->
   LIT/UNOP/) OP_CONCAT LIT/(

   (keeping EXPRs same)
 
 */

static char *concat(const char *regex)
{
	char *nregex;
	register char *nr;

	nregex = alloc(regex, 2, 0);
	for (nr = nregex; *regex; regex++) {
		*nr++ = *regex;
		if (isexprend(regex) && isexprbeg(regex+1))
			*nr++ = OP_CONCAT;
	}
	*nr = '\0';

	return nregex;
}

/* ---------- stage 3 -- () -----------
 
 EXPR BINOP EXPR -> (EXPR BINOP EXPR)
 
 (keeping EXPRs and BINOP same)

 */ 

static char *mkpar(const char *regex)
{
	char *nregex;
	register char *nr, *t, op;
	register int par, l;

	/* 
	   another approach is to allocate strlen(regex) + 2*n,
	   where n is BINOP count,
	   but it can be proved that strlen(regex) >= 2*n,
	   so no additional loop, and the value difference is small
	 */
	nregex = alloc(regex, 2, 0);
	strcpy(nregex, regex);
	l = strlen(nregex);

	for (nr = nregex; *nr; nr++) {
		if (isbinop(*nr)) {
			op = *(t = nr);
		//	printf("1: %s\n", nregex);

			/* EXPR (left) */
			for (par = 0; !(par==0 && isexprbeg(nr)); ) {
				nr--;
				if (*nr == '(')
					par++;
				else if (*nr == ')')
					par--;
			}
			memmove(nr+1, nr, l - (nr - nregex));
			nregex[++l] = '\0';	/* not necessary? */
			*nr = '(';
		//	printf("2: %s\n", nregex);

			/* BINOP */
			/* t[1] == op; */
			nr = t+1;	/* par is 0 */
		//	printf("3: %s\n", nregex);

			/* EXPR (right) */
			for (; !(par == 0 && isexprend(nr)); ) {
				nr++;
				if (*nr == '(')
					par++;
				else if (*nr == ')')
					par--;
			}
			nr++;

			memmove(nr+1, nr, l - (nr - nregex));
			*nr = ')';
			nregex[++l] = '\0';
		//	printf("4: %s\n", nregex);

			/* return to old position */
			nr = t+1;
		}
	}

	return nregex;
}

/*
  ------ stage 4 -- infix->postfix -------
 
  (EXPR BINOP EXPR) -> (EXPR EXPR BINOP)
 
  (keeping EXPRs and BINOP same)

  moves from the deepest nesting
 
 */

#define MAXPARS		100

static int pars[MAXPARS];
static int top = -1;	/* should be -1 between calls,
			   the reverse can detect mistakes */

static char *in2post(const char *regex)
{
	char *nregex;
	register char *nr, *t, op;
	register int par;

	nregex = alloc(regex, 1, 0);
	strcpy(nregex, regex);
	//printf("have %s\n", nregex);
	for (nr = nregex; *nr; nr++)
		if (*nr == '(')
			pars[++top] = nr - nregex;
		else if (*nr == ')') {
			t = nregex + pars[top--] + 1;
			//printf("(%d;%d)\n", t-nregex, nr-nregex);
			for (; t != nr; t++)
				if (*t == '(') {
					par = 1;
					do {
						t++;
						if (*t == '(')
							par++;
						else if (*t == ')')
							par--;
					} while (par);
				} else if (isbinop(*t)) {
					op = *t;
					memcpy(t, t+1, nr-t-1);
					nr[-1] = op;
					//printf("%s\n", nregex);
					break;	/* fully (EXPR BINOP EXPR) */
				}
		}

	return nregex;
}

/* -------------- stage 5 -- rm () ------------- */

char *rmpar(const char *regex)
{
	char *nregex;
	register char *nr;

	nregex = alloc(regex, 1, 0);
	for (nr = nregex; *regex; regex++)
		if (*regex != '(' && *regex != ')')
			*nr++ = *regex;
	*nr = '\0';

	return nregex;
}

/* 
  -------- apply transformations ---------
 
  updregex is the only interface to the external world

 */

static char *(*fregex[])(const char *) = {
	rmblank,
	concat,
	mkpar,
	in2post,
	rmpar
};

char *updregex(const char *regex)
{
	register char *(**p)(const char *);
	register char *nregex, *t;

	icheck(regex);
	nregex = strdup(regex);
	for (p = fregex; p < &fregex[NELEM(fregex)]; p++) {
		t = (*p)(nregex);
		free(nregex);
		nregex = t;
	}

	return nregex;
}
