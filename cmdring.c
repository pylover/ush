#include "config.h"

#include "cmdring.h"


void
cmdring_flush(struct cmdring *cr) {
    while (ERING_USED(cr)) {
        str_deinit(ERING_RPTR(cr));
        ERING_DECR(cr);
    }
}


int
cmdring_pushnew(struct cmdring *cr) {
    if (ERING_ISFULL(cr)) {
        str_deinit(ERING_RPTR(cr));
        ERING_DECR(cr);
    }

    if (str_init(ERING_WPTR(cr), CONFIG_USH_TERM_LINESIZE)) {
        return -1;
    }

    cr->current = cr->head;
    ERING_INCR(cr);
    return 0;
}
