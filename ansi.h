#ifndef ANSI_H_
#define ANSI_H_


#include "term.h"


#define ANSI_RESET "\33[0m"
#define ANSI_ERASETOEND "\33[0K"


ASYNC
ansiA(struct uaio_task *self, struct term *term);


#endif
