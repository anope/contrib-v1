#ifndef WAITING_H
#define WAITING_H

#include "core.h"

int hs_call_waiting(User *u);
int hs_do_waiting(User *u, char *nick, char *title, boolean ischan);

#endif

/* EOF */
