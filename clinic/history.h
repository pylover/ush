#ifndef HISTORY_H_
#define HISTORY_H_


#include "state.h"


struct history *
history_create(unsigned int maskbits);


int
history_dispose(struct history *h);


#endif  // HISTORY_H_
