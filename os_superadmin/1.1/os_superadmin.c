/**
 * -----------------------------------------------------------------------------
 * Name    : os_superadmin
 * Author  : Cronus <Cronus@Nite-Serv.com>
 * Date    : 3/5/2010 (Last update: 3/31/2010)
 * Version : 1.1
 * -----------------------------------------------------------------------------
 * Requires    : Anope 1.8.3, UnrealIRCd
 * Tested      : Anope 1.8.3 + UnrealIRCd 3.2.8.1
 * -----------------------------------------------------------------------------
 * This module adds the operserv SUPERADMIN command that allows you to LIST
 * all SuperAdmin's and also has the capability to make any user a SuperAdmin 
 * or turn off any user's SuperAdmin.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *   1.1  -  Made OperServ notice user he/she has been granted|removed SuperAdmin Status
 *           Made Anope send a Global when command is used.
 *           Tells you if NO one has SuperAdmin enabled.
 *           Will tell how many Users have SuperAdmin enabled.
 *
 *   1.0  -  Initial release
 *
 * -----------------------------------------------------------------------------
 *
 * Credits:
 *
 * Adam - Helped me out throughout the Initial construction of this module
 *
 * -----------------------------------------------------------------------------
 **/



#include "module.h"

#define AUTHOR      "Cronus"
#define VERSION     "$Id: os_superadmin.c 1.1 $"
#define NAME        "os_superadmin"


int do_superadmin(User *u);
void myOperServHelp(User *u);
int myHelpSyntax(User *u);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    int status;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    c = createCommand("SUPERADMIN", do_superadmin, is_services_root, -1, -1, -1, -1, -1);
    status = moduleAddCommand(OPERSERV, c, MOD_UNIQUE);
    if (status != MOD_ERR_OK) {
        return MOD_STOP;
    } else {
        alog("%s: Successfully loaded module.", s_OperServ);
        alog("%s: Providing command '\2/msg OperServ HELP SUPERADMIN\2'", s_OperServ);
    }
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    
    moduleSetOperHelp(myOperServHelp);
    moduleAddHelp(c, myHelpSyntax);

    return MOD_CONT;
}

void myOperServHelp(User *u)
{
    if (is_services_root(u)) {
        notice_user(s_OperServ, u, "  SUPERADMIN     List all SA's and optionally grant or remove SA status.");
	}
	return MOD_CONT;
}

int myHelpSyntax(User * u)
{
	if (is_services_root(u)) {
		notice_user(s_OperServ, u, "Syntax: \2SUPERADMIN  {LIST|ON|OFF}\2");
		notice_user(s_OperServ, u, "            \2SUPERADMIN LIST\2 will list all SA's");
		notice_user(s_OperServ, u, "            \2SUPERADMIN ON|OFF [NICK]\2 will turn SA ON or OFF for [NICK]");
	}
	return MOD_CONT;
}

int do_superadmin(User *u) 
{
	User *u2;
	char *buffer = moduleGetLastBuffer();
	char *option = myStrGetToken(buffer, ' ', 0);
	char *nick = myStrGetToken(buffer, ' ', 1);

	if (!option) {
		myHelpSyntax(u);
	} else if (!stricmp(option, "LIST")) {
		int count = 0;
		for (u2 = firstuser(); u2; u2 = nextuser()) {
			if (u2->isSuperAdmin) { 
				notice_user(s_OperServ, u, "%s (%s@%s) [%s] is a super admin", 
								u2->nick, u2->username, u2->host, u2->server->name);
			count++;
			}
		}
		if (count == 0) { 
			notice_user(s_OperServ, u, "No users found with SuperAdmin enabled.");
		}
		else { 
			notice_user(s_OperServ, u, "%u users found with SuperAdmin enabled.", count);
		}
	} else if (!stricmp(option, "ON")) {
		if (!nick) {
			notice_user(s_OperServ, u, "Syntax: \2SUPERADMIN ON|OFF [Nick]\2");
		}
		else if (!(u2 = finduser(nick)))
		{
			notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
		}
		else {
			alog("%s: %s used superadmin on for %s", s_OperServ, u->nick, u2->nick);
			u2->isSuperAdmin = 1;
			notice_user(s_OperServ, u2, "You are now a SuperAdmin.");
			anope_cmd_global(s_OperServ,"\2%s\2 (%s@%s) has been granted SUPERADMIN status by \2%s\2", u2->nick, u2->username, u2->host, u->nick);
		}
	} else if (!stricmp(option, "OFF")) {
		if (!nick) {
			notice_user(s_OperServ, u, "Syntax: \2SUPERADMIN ON|OFF [Nick]\2");
		}
		else if (!(u2 = finduser(nick)))
		{
			notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
		} else {
			alog("%s: %s used superadmin off for %s", s_OperServ, u->nick, u2->nick);
			u2->isSuperAdmin = 0;
			notice_user(s_OperServ, u2, "You are no longer a SuperAdmin.");
			anope_cmd_global(s_OperServ,"\2%s\2 (%s@%s) has had their SUPERADMIN status removed by \2%s\2", u2->nick, u2->username, u2->host, u->nick);
		}
	}
	if (option)
			free(option);
	if (nick)
			free(nick);
}

void AnopeFini(void)
{
	alog("%s: os_superadmin: Module Unloaded.", s_OperServ);
}

/* EOF */		
		
