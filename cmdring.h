#ifndef CMDRING_H_
#define CMDRING_H_


#include "str.h"


struct cmdring {
    struct strring;
    int current;
};


#define CMDRING_CURRENT(cr) ((cr)->buffer + (cr)->current)


int
cmdring_init(struct cmdring *cr, uint8_t ringmaskbits);


int
cmdring_deinit(struct cmdring *cr);


#endif  // CMDRING_H_
