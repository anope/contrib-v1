#ifndef ACTIVATE_H
#define ACTIVATE_H

#include "core.h"

int hs_call_activate(User *u);
int hs_call_activate_all(User *u);
int hs_do_activate(User *u, char *nick, char *reason, boolean ischan);
int hs_do_activate_match(User *u, char *reason, boolean ischan, char *match);

#endif

/* EOF */
