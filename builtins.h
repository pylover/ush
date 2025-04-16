#ifndef BUILTINS_H_
#define BUILTINS_H_


#include "ush.h"


extern const struct ush_executable builtin_free;


ASYNC
freeA(struct uaio_task *self, struct ush_process *p);


#endif  // BUILTINS_H_
