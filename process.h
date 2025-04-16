#ifndef PROCESS_H_
#define PROCESS_H_


#include "cmd.h"


struct ush_process *
process_create(struct ush *sh, struct cmd *cmd);


void
process_free(struct ush_process *p);


#endif  // PROCESS_H_
