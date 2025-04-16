#include "ush.h"
#include "builtins.h"


ASYNC
freeA(struct uaio_task *self, struct ush_process *p) {
    UAIO_BEGIN(self);
    UAIO_FINALLY(self);
}


struct ush_executable builtin_free = {
    .name = "free",
    .entrypoint = freeA,
};
