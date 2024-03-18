#include <module.h>

#define AUTHOR "Edavis"
#define VERSION "1.0.0"
#define UMODE_h_unreal32 0x00000002

int is_helper(User *u) {
    if (!u)
    return 0;
    if (!stricmp(IRCDModule, "unreal32"))
    return u->mode & UMODE_h_unreal32;
    return 0;
}


int do_continue(User * u);

int AnopeInit(int argc, char **argv)
{
    Command *c;

    c = createCommand("SET", do_continue, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("SETALL", do_continue, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("DEL", do_continue, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("DELALL", do_continue, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

        if (findModule("os_raw")) {
                alog("I found os_raw loaded!, Sorry it didn't work out.");
                return MOD_STOP;
        }

    return MOD_CONT;
}

void AnopeFini(void)
{
       
}



int do_continue(User * u)
{
  if (!is_host_setter(u) && !is_helper(u)) {
      notice(s_HostServ, u->nick, "Access denied");
      return MOD_STOP;
  }
  return MOD_CONT;
} 