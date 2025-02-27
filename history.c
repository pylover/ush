#include "config.h"

#include "history.h"


int
history_init(struct history *h, uint8_t ringmaskbits) {
    if (strring_init((struct strring*)h, ringmaskbits)) {
        return -1;
    }

    while (ERING_AVAIL(h)) {
        if (str_init(ERING_WPTR(h), CONFIG_USH_TERMINAL_LINESIZE)) {
            goto failed;
        }

        ERING_INCR(h);
    }

    return 0;

failed:
    history_deinit(h);
    return -1;
}


int
history_deinit(struct history *h) {
    int ret = 0;

    ret |= strring_deinit((struct strring*)h);
    return ret;
}
