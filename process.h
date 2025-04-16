#ifndef PROCESS_H_
#define PROCESS_H_


#include "cmd.h"


int
process_fromcmd(struct ush_process *p, struct cmd *cmd);


void
process_free(struct ush_process *p);


#endif  // PROCESS_H_
