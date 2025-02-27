#ifndef HISTORY_H_
#define HISTORY_H_


#include "str.h"


struct history {
    struct strring;

    int current;
};


int
history_init(struct history *h, uint8_t ringmaskbits);


int
history_deinit(struct history *h);


#endif  // HISTORY_H_
