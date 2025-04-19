#ifndef BUILTINS_H_
#define BUILTINS_H_


#include "ush.h"


extern const struct ush_executable builtin_free;
extern const struct ush_executable builtin_iodump;


ASYNC
freeA(struct uaio_task *self, struct ush_process *p);


ASYNC
iodumpA(struct uaio_task *self, struct ush_process *p);


#endif  // BUILTINS_H_
