#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "core.h"

#define LNG_NUM_STRINGS     79

#define LNG_REQUEST_SYNTAX              0
#define LNG_REQUESTED                   1
#define LNG_REQUEST_WAIT                2
#define LNG_REQUEST_MEMO                3
#define LNG_ACTIVATE_SYNTAX             4
#define LNG_ACTALL_SYNTAX               5
#define LNG_ACTIVATED                   6
#define LNG_ACTIVATE_MEMO               7
#define LNG_ACTIVATE_MEMO_REASON        8
#define LNG_REJECT_SYNTAX               9
#define LNG_REJALL_SYNTAX               10
#define LNG_REJECTED                    11
#define LNG_REJECT_MEMO                 12
#define LNG_REJECT_MEMO_REASON          13
#define LNG_NO_REQUEST                  14
#define LNG_HELP                        15
#define LNG_HELP_SETTER                 16
#define LNG_HELP_REQUEST                17
#define LNG_HELP_ACTIVATE               18
#define LNG_HELP_ACTALL                 19
#define LNG_HELP_ACTIVATE_MEMO          20
#define LNG_HELP_REJECT                 21
#define LNG_HELP_REJECT_MEMO            22
#define LNG_HELP_REJALL                 23
#define LNG_HELP_REJALL_MEMO            24
#define LNG_WAITING_SYNTAX              25
#define LNG_HELP_WAITING                26
#define LNG_REQUEST_FORBIDDEN           27
#define LNG_REQUEST_FORBIDDEN_REASON    28
#define LNG_CONFIG_SYNTAX               29
#define LNG_HELP_CONFIG                 30
#define LNG_VHOST_SYNTAX                31
#define LNG_NO_REQUESTS                 32
#define LNG_NOT_ENOUGH_REQUESTS         33
#define LNG_NO_MATCHES                  34
#define LNG_REGEX_DISABLED              35
#define LNG_WILDCARD_DISABLED           36
#define LNG_REGEX_ERROR                 37
#define LNG_CONF_SAVE_DB                38
#define LNG_CONF_REGEX_ADD_FAIL         39
#define LNG_CONF_REGEX_ADD              40
#define LNG_CONF_REGEX_LIST             41
#define LNG_CONF_REGEX_LIST_END         42
#define LNG_CONF_REGEX_DELETED          43
#define LNG_CONF_TEMPORARY              44
#define LNG_VHOST_LISTING               45
#define LNG_VHOST_REQUEST_COUNT         46
#define LNG_VHOST_REQUEST_DISPLAYED     47
#define LNG_VHOST_REMINDER              48
#define LNG_KEY_NICK                    49
#define LNG_KEY_OFFLINE                 50
#define LNG_KEY_CURRENT                 51
#define LNG_KEY_NONE                    52
#define LNG_KEY_REQUEST                 53
#define LNG_KEY_AGO                     54
#define LNG_KEY_OPER                    55
#define LNG_KEY_HOST                    56
#define LNG_KEY_REASON                  57
#define LNG_REQUEST_ALREADY_APPROVED    58
#define LNG_REQUEST_REMOVED_HOST        59
#define LNG_REQUEST_REMOVED_GROUP       60
#define LNG_REQUEST_NOT_SHOWN           61
#define LNG_REQUEST_UPDATE_DENIED       62
#define LNG_REQUEST_AUTO_APPROVED       63
#define LNG_HOST_REJECTED               64
#define LNG_HOST_IDENT_REJECTED         65
#define LNG_HOST_ACTIVATED              66
#define LNG_HOST_IDENT_ACTIVATED        67
#define LNG_REQUEST_AUTO_REJECTED       68
#define LNG_CONF_EXCEPTION_ADD_FAIL     69
#define LNG_CONF_EXCEPTION_ADD          70
#define LNG_CONF_EXCEPTION_LIST         71
#define LNG_CONF_EXCEPTION_LIST_END     72
#define LNG_CONF_EXCEPTION_DELETED      73
#define LNG_CONF_EXCEPTION_NOT_FOUND    74
#define LNG_REQUEST_REMOVED_GONE        75
#define LNG_CONFIG_REGEX_SYNTAX         76
#define LNG_CONFIG_EXCEPTION_SYNTAX     77
#define LNG_USER_IS_EXEMPT              78

int my__def_language(void);
void my__add_languages(void);

#endif

/* EOF */
