#include <string.h>

#include <elog.h>

#include "ush.h"
#include "cmdline.h"


struct ush_cmdline *
cmdline_create(unsigned short maxchars) {
    struct ush_cmdline *cl;
    size_t size;

    cl = malloc(sizeof(struct ush_cmdline) + maxchars + 1);
    if (cl == NULL) {
        return NULL;
    }

    memset(cl, 0, sizeof(*cl));
    return cl;
}


int
cmdline_dispose(struct ush_cmdline *cl) {
    if (cl == NULL) {
        return -1;
    }

    free(cl);
    return 0;
}
