CC = gcc

regex:	main.c type.c in2post.c nfa.c match.c routine.h type.h regex.h nfa.h match.h
	cc -o regex main.c type.c in2post.c nfa.c match.c

clean:
	rm -f *.o regex
