#ifndef CMDRING_H_
#define CMDRING_H_


#include "str.h"


struct cmdring {
    struct strring;
    int current;
};


#define cmdring_init(cr, b) strring_init((struct strring *)cr, b)
#define cmdring_deinit(cr) strring_deinit((struct strring *)cr)
#define CMDRING_CURRENT(cr) ((cr)->buffer + (cr)->current)


int
cmdring_pushnew(struct cmdring *cr);


void
cmdring_flush(struct cmdring *cr);


#endif  // CMDRING_H_
