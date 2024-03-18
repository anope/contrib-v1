#ifndef HELP_H
#define HELP_H

#include "core.h"

int hs_help_request(User *u);
int hs_help_activate(User *u);
int hs_help_actall(User *u);
int hs_help_reject(User *u);
int hs_help_rejall(User *u);
int hs_help_waiting(User *u);
int hs_help_config(User *u);
int hs_help_config_regex(User *u);
int hs_help_config_exception(User *u);
void hs_help(User *u);

#endif

/* EOF */
