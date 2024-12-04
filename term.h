#ifndef TERM_H_
#define TERM_H_


#include "ush.h"


int
term_print(struct ush *sh, const char *restrict fmt, ...);


void
term_backspace(struct ush *sh);


void
term_navleft(struct ush *sh, int c);


void
term_navright(struct ush *sh, int c);


int
term_insert(struct ush *sh, char c);


void
term_navend(struct ush *sh);


void
term_rewrite(struct ush *sh);


#endif  // TERM_H_
