/* OperServ os_pingfake.c
 * Module made by Freelancer.
 * Based off os_umode.c core module. (Just changes a few commands here & there... :D)
 * Makes a user thing they have pinged by svskilling them.
 */

#include "module.h"

int do_operpingfake(User * u);
void myOperServHelp(User * u);
int os_help_pingfake(User * u);

int AnopeInit(int argc, char **argv)
{
    Command *c;

    moduleAddAuthor("Freelancer");
    moduleAddVersion("1.0");

    c = createCommand("PINGFAKE", do_operpingfake, is_services_root,
                      os_help_pingfake, -1, -1, -1, -1);
    moduleAddCommand(OPERSERV, c, MOD_UNIQUE);
	moduleAddHelp(c, os_help_pingfake);
	moduleSetOperHelp(myOperServHelp);
	
    if (!ircd->umode) {
        return MOD_STOP;
    }
    return MOD_CONT;
}

void AnopeFini(void)
{

}

void myOperServHelp(User *u)
{
	if (u->isSuperAdmin)
		notice_user(s_OperServ, u, "    PINGFAKE Make a user look like they have pinged out");
}

int os_help_pingfake(User *u)
{
	notice_user(s_OperServ, u, "Syntax: \2PINGFAKE \37nick\37");
	notice_user(s_OperServ, u, " ");
	notice_user(s_OperServ, u, "Allows root admins to make a user appear they have pinged.");
	
	return MOD_STOP;
}

int do_operpingfake(User * u)
{
    char *nick = strtok(NULL, " ");
	
    User *u2;

    if (!u->isSuperAdmin) {
        notice_lang(s_OperServ, u, OPER_SUPER_ADMIN_ONLY);
        return MOD_CONT;
    }

    if (!nick) {
        syntax_error(s_OperServ, u, "PINGFAKE", OPER_UMODE_SYNTAX);
        return MOD_CONT;
    }

    if (!(u2 = finduser(nick))) {
        notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
    } else {
		anope_cmd_svskill(ServerName, nick, "Ping timeout");

        if (WallOSMode)
            anope_cmd_global(s_OperServ, "\2%s\2 used PINGFAKE on %s",
                             u->nick, nick);
	}
    return MOD_CONT;
}