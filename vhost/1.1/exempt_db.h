#ifndef EXEMPT_DB_H
#define EXEMPT_DB_H

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

E int my__add_exception(char *who, char *by);
E int my__del_exception_by_num(int num);
E int my__del_exception_by_nick(char *who);
E Exception_t *createException(Exception_t *next, NickAlias *na, char *by);
E Exception_t *findException(Exception_t *head, char *who, boolean *found);
E Exception_t *findExceptionNick(Exception_t *head, char *who, boolean *found);
E Exception_t *insertException(Exception_t *head, Exception_t *prev, NickAlias *na, char *by);
E Exception_t *deleteException(Exception_t *head, Exception_t *prev);
E Exception_t *findExceptionNum(Exception_t *head, int num, boolean *found);

#endif

/* EOF */
