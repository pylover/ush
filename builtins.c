#include "ush.h"
#include "term.h"
#include "builtins.h"


ASYNC
freeA(struct uaio_task *self, struct ush_process *p) {
    UAIO_BEGIN(self);
    TERM_PRINTF(p->term, "free memory: %lu\n", esp_get_free_heap_size());
    UAIO_FINALLY(self);
}


const struct ush_executable builtin_free = {
    .name = "free",
    .main = freeA,
};
