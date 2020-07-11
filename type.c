#include "routine.h"
#include "type.h"

/* ----------- sets ------------ */
 
static const char lit[] = {
	'a','b','c','d','e','f','g','h','i','j','k','l','m',
	'n','o','p','q','r','s','t','u','v','w','x','y','z',
	'A','B','C','D','E','F','G','H','I','J','K','L','M',
	'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
	'0','1','2','3','4','5','6','7','8','9', '.'
};

static const char unop[] = {
	'?', '*', '+'
};

static const char binop[] = {
	'|', OP_CONCAT
};

static const char blnk[] = {
	' ', '\t'
};

/* ----- sets belong check ------ */

static int belong(char c, const char *s, int n)
{
	register const char *p;

	for (p = s; p < &s[n]; p++)
		if (c == *p)
			return 1;
	return 0;
}

int islit(char c)
{
	return belong(c, lit, NELEM(lit));
}

int isunop(char c)
{
	return belong(c, unop, NELEM(unop));
}

int isbinop(char c)
{
	return belong(c, binop, NELEM(binop));
}

int isop(char c)
{
	/* no overhead */
	return isunop(c) || isbinop(c);
}

int isblnk(char c)
{
	return belong(c, blnk, NELEM(blnk));
}

int isch(char c)
{
	return islit(c) || isop(c) || isblnk(c);
}

int isexprbeg(const char *c)
{
	return islit(*c) || *c == '(';
}

int isexprend(const char *c)
{
	/* (1) islit can also be the beginning, not to confuse with
	 e.g. a* -- a is not end here, see language formula

	 (2) also, ')' not always means end e.g.
	   )))? -- none of ) is end
	 */
	if (islit(*c) || *c == ')')
		return !isunop(c[1]);
	return isunop(*c);
}
