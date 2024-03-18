#ifndef COMMANDS_H
#define COMMANDS_H

#include "core.h"

CommandHash *vHost_cmdTable[MAX_CMD_HASH];
void my__add_message_list(void);
int ns_call_info(User *u);

#endif

/* EOF */
