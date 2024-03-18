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

    c = createCommand("SENDPASS", do_continue, NULL, -1, -1, -1, -1, -1); 
    moduleAddCommand(NICKSERV, c, MOD_HEAD); 

    moduleAddAuthor(AUTHOR); 
    moduleAddVersion(VERSION); 

    return MOD_CONT; 
}

void AnopeFini(void)
{ 

}



int do_continue(User * u)
{
  if (!is_helper(u)) {
      notice(s_NickServ, u->nick, "Access denied");
      return MOD_STOP;
  {
  return MOD_CONT;
  }
}
 