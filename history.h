#ifndef HISTORY_H_
#define HISTORY_H_


#include "ush.h"


void
history_init(struct ush *sh);


void
history_alloc(struct ush *sh);


void
history_updatecurrent(struct ush *sh);


int
history_prev(struct ush *sh);


int
history_next(struct ush *sh);


#endif  // HISTORY_H_
