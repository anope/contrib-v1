#ifndef REGEX_DB_H
#define REGEX_DB_H

#include "core.h"

#ifndef _WIN32
#define E extern
#define EI extern
#else
#ifndef MODULE_COMPILE
#define E extern __declspec(dllexport)
#define EI extern __declspec(dllimport)
#else
#define E extern __declspec(dllimport)
#define EI extern __declspec(dllexport)
#endif
#endif

E char *my__checkregex(char *s);
E int my__add_regex(char *nick, char *reason, char *regex);
E int my__del_regex_by_num(int num);
E int my__del_regex_by_nick(char *nick);
E Regex *createRegex(Regex *next, char *nick, char *reason, char *regex);
E Regex *findRegex(Regex *head, char *regex, boolean *found);
E Regex *findRegexNum(Regex *head, int num, boolean *found);
E Regex *findRegexNick(Regex *head, char *nick, boolean *found);
E Regex *insertRegex(Regex *head, Regex *prev, char *nick, char *reason, char *regex);
E Regex *deleteRegex(Regex *head, Regex *prev);

#endif

/* EOF */
