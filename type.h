#ifndef _TYPE_H
#define _TYPE_H

#define OP_CONCAT	1

/* set belong
 *
 * cannot be macros, because then data arrays
 * will be open
 */
int islit(char c);
int isunop(char c);
int isbinop(char c);
int isop(char c);
int isblnk(char c);
int isch(char c);
int isexprbeg(const char *c);
int isexprend(const char *c);

#endif
