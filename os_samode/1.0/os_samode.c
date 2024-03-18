/* OperServ os_samode
 * By Thomas Edwards (TMFKSOFT)
 * I only take credit for modifications.
 * This module is directly modified from os_umode (Core)
 * This module merely gives general Oper's access to change
 * a users umodes.
 *
 * Usage: /msg operserv samode user modes
 */
/*************************************************************************/

#include "module.h"

int do_operumodes(User * u);
void myOperServHelp(User * u);

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv)
{
    Command *c;

    moduleAddAuthor("Thomas Edwards (TMFKSOFT)");
    moduleAddVersion("1.0");

    c = createCommand("SAMODE", do_operumodes, is_services_oper,
                      OPER_HELP_UMODE, -1, -1, -1, -1);
    moduleAddCommand(OPERSERV, c, MOD_UNIQUE);

    moduleSetOperHelp(myOperServHelp);

    if (!ircd->umode) {
        return MOD_STOP;
    }
    return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void)
{

}


/**
 * Add the help response to anopes /os help output.
 * @param u The user who is requesting help
 **/
void myOperServHelp(User * u)
{
    if (is_services_oper(u)) {
        notice(s_OperServ, u->nick, "    SAMODE      Change a user's modes");
    }
}

/**
 * Change any user's UMODES
 *
 * User must be at least oper.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 */
int do_operumodes(User * u)
{
    char *nick = strtok(NULL, " ");
    char *modes = strtok(NULL, "");

    User *u2;
    if (!nick || !modes) {
        notice(s_OperServ, u->nick,"Syntax: SAMODE nick modes");
        return MOD_CONT;
    }

    /**
     * Only accept a +/- mode string
     *-rob
     **/
    if ((modes[0] != '+') && (modes[0] != '-')) {
        notice(s_OperServ, u->nick,"Syntax: SAMODE nick modes");
        return MOD_CONT;
    }
    if (!(u2 = finduser(nick))) {
        notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
    } else {
        common_svsmode(u2, modes, NULL);

        notice_lang(s_OperServ, u, OPER_UMODE_SUCCESS, nick);
        notice_lang(s_OperServ, u2, OPER_UMODE_CHANGED, u->nick);

        if (WallOSMode)
            anope_cmd_global(s_OperServ, "\2%s\2 used SAMODE on %s",
                             u->nick, nick);
    }
    return MOD_CONT;
}