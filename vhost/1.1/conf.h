#ifndef CONF_H
#define CONF_H

#include "core.h"
#include "help.h"
#include "reject.h"

int hs_call_config(User *u);
int hs_do_config(User *u, char *setting, char *param, boolean ischan);
int hs_do_config_regex(User *u, char *param, boolean ischan);
int hs_do_config_exception(User *u, char *param, boolean ischan);
int hs_do_config_show(User *u, boolean ischan);
int hs_do_config_set(User *u, char *param, boolean ischan);
int hs_do_config_set_internal(User *u, char *setting, int value, boolean ischan);

#endif

/* EOF */
