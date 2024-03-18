#ifndef REJECT_H
#define REJECT_H

#include "core.h"

int hs_call_reject(User *u);
int hs_call_reject_all(User *u);
int hs_do_reject(User *u, char *nick, char *reason, boolean ischan);
int hs_do_reject_match(User *u, char *reason, boolean ischan, char *match);

#endif

/* EOF */
