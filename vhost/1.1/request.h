#ifndef REQUEST_H
#define REQUEST_H

#include "core.h"

int hs_call_request(User *u);
int hs_match_request(char *nick, char *vhost, char *vident);
int hs_match_request_case(char *nick, char *vhost, char *vident);
int hs_do_request(User *u, char *nick, char *request, boolean ischan);

#endif

/* EOF */
