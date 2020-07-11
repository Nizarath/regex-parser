#include <stdio.h>
#include <stdlib.h>
#include "regex.h"
#include "match.h"
#include "nfa.h"

int main(int argc, char *argv[])
{
	char *nregex;
	nfa *nfa;

	/* regex */
	nregex = updregex(argv[1]);

	/* automata */
	nfa = mknfa(nregex);

	/* now match it */
	for (argc -= 2, argv += 2; argc; argc--, argv++)
		if (match(nfa, *argv))
			printf("%s\n", *argv);

	exit(0);
}
