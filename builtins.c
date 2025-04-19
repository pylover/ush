#include <driver/gpio.h>

#include <elog.h>

#include "ush.h"
#include "term.h"
#include "builtins.h"


ASYNC
freeA(struct uaio_task *self, struct ush_process *p) {
    UAIO_BEGIN(self);
    printf("free memory: %lu\n", esp_get_free_heap_size());
    UAIO_FINALLY(self);
}


const struct ush_executable builtin_free = {
    .name = "free",
    .main = freeA,
};


ASYNC
iodumpA(struct uaio_task *self, struct ush_process *p) {
    UAIO_BEGIN(self);
    gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);
    // (1ULL << 4) | (1ULL << 18) | (1ULL << 26));
    UAIO_FINALLY(self);
}


const struct ush_executable builtin_iodump = {
    .name = "iodump",
    .main = iodumpA,
};
