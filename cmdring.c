#include "config.h"

#include "cmdring.h"


int
cmdring_init(struct cmdring *cr, uint8_t ringmaskbits) {
    if (strring_init((struct strring*)cr, ringmaskbits)) {
        return -1;
    }

    while (ERING_AVAIL(cr)) {
        if (str_init(ERING_WPTR(cr), CONFIG_USH_TERM_LINESIZE)) {
            goto failed;
        }

        ERING_INCR(cr);
    }

    cr->current = 0;
    return 0;

failed:
    cmdring_deinit(cr);
    return -1;
}


int
cmdring_deinit(struct cmdring *cr) {
    int ret = 0;

    ret |= strring_deinit((struct strring*)cr);
    return ret;
}
