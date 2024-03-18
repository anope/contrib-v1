#include "module.h"
#define AUTHOR "Roy"
#define VERSION "1.4"

/* ------------------------------------------------------------------------------------
 * Name		: hs_operon
 * Author	: royteeuwen
 * Version	: 1.4
 * Date		: 5th Nov, 2006
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ------------------------------------------------------------------------------------
 * Tested	: Anope-1.7.17 on Unreal3.2.5
 * ------------------------------------------------------------------------------------
 * Description:
 * This makes it possible for a services admin or root to put a vhost on for a person
 * by typing /hs on nick
 * ------------------------------------------------------------------------------------
 * Changes v1.3 to v1.4:
 * - Tells you when a nick is not registered
 * Changes v1.2 to v1.3:
 * - It will now stop when a person does not excist or is not in use
 * - It will now message you when a person is not identified
 * Changes v1.0 to v1.2:
 * - It is now possible to define the level needed to put a host on at operlvl.
 * - When the root putted a wrong level for oper, it will notice the perosn tryting to use
 * - It will now tell the IRC operator that the vhost has been activated
 * - It will now tell a person when they try this and dont have the right IRC operator lvl
 */


#define OPERLVL 1    /* Define the oper lvl needed. Services operator = 1, services admin = 2, root = 3 */
#define WRONGLVL "The services root has putten a wrong level"
#define HOST_OPER_ACTIVATED "The vhost has been activated"
#define HOST_OPER_DENIED "You don't have the correct IRC operator status to put a nick's vhost on"
#define HOST_NON_EXCIST "This nick does not have a vhost"
#define HOST_NOT_IDED "This nick is not identified"
#define HOST_NO_REGED "This nick is not registered"

/* ---------------------------------------------------------------------- */
/* DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING         */
/* ---------------------------------------------------------------------- */

#define LANG_NUM_STRINGS 7

int do_hoston(User *u);
void mAddLanguage(void);
int mHSHelp(User * u);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    c = createCommand("ON", do_hoston, NULL, -1, -1, -1, -1,
                      -1);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);
    moduleAddAuthor(AUTHOR);

    moduleAddVersion(VERSION);
    alog("hs_operon.c v%s by %s loaded", VERSION, AUTHOR);

    moduleAddHelp(c, mHSHelp);

    mAddLanguage();

    return MOD_CONT;
}

int do_hoston(User *u)
{
	int is_servadmin;
	if (OPERLVL == 1) {
		is_servadmin = is_services_oper(u);
 	} else if (OPERLVL == 2) {
		is_servadmin = is_services_admin(u);
 	} else if (OPERLVL == 3) {
		is_servadmin =  is_services_root(u);
	} else {
      	notice(s_HostServ, u->nick,WRONGLVL);
		return MOD_STOP;
	}
      NickAlias *na;
	char *buffer = moduleGetLastBuffer();
 	char *option = myStrGetToken(buffer, ' ', 0);
 	User *u2 = finduser(option);


	if (!option) {
	return MOD_CONT;
      }
	if (is_servadmin) {
		char *vHost;
       	char *vIdent = NULL;
			if (!(u2 = finduser(option))) {
				notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, option);
				if (option) free(option);
				return MOD_STOP;
			}
       		if ((na = findnick(u2->nick))) {
        			if (na->status & NS_IDENTIFIED) {
           				vHost = getvHost(u2->nick);
            			vIdent = getvIdent(u2->nick);
					if (!vHost) {
						notice(s_HostServ, u->nick, HOST_NON_EXCIST);
						if (option) free(option);
						return MOD_STOP;
					}
              			if (vIdent) {
                    			notice_lang(s_HostServ, u2, HOST_IDENT_ACTIVATED, vIdent, vHost);
						notice(s_HostServ, u->nick, HOST_OPER_ACTIVATED);
                			} else {
                   			notice_lang(s_HostServ, u2, HOST_ACTIVATED, vHost);
						notice(s_HostServ, u->nick, HOST_OPER_ACTIVATED);
               			}
                			anope_cmd_vhost_on(u2->nick, vIdent, vHost);
                			if (ircd->vhost) {
                    			u2->vhost = sstrdup(vHost);
                			}
                			if (ircd->vident) {
                   			 if (vIdent)
                        			u2->vident = sstrdup(vIdent);
                			}
               			 set_lastmask(u2);           
        			} else {
           				notice(s_HostServ, u->nick, HOST_NOT_IDED);
        			}
    			} else {
        			notice(s_HostServ, u->nick, HOST_NO_REGED);
    			}
			if (option) free(option);
			return MOD_STOP;
      }
	else if (!is_servadmin && option) {
		notice(s_HostServ, u->nick, HOST_OPER_DENIED);
		if (option) free(option);
		return MOD_STOP;
	}
    return MOD_CONT;
}

void mAddLanguage(void)
{
    char *langtable_hs_on[] = {
        "Syntax: ON [Nick]\n"
        " \n"
        "Activates the vhost currently assigned to the nick in use.\n"
        "When you use this command any user who performs a /whois \n"
        "on you will see the vhost instead of your real IP address.\n"
        " \n"
        "Services Admins can put the vhost on for another nick\n"
    };
    
    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_hs_on);
}
int mHSHelp(User * u) {
    moduleNoticeLang(s_HostServ, u, 0);
    return MOD_STOP;
}

void AnopeFini(void)
{
	alog("hs_operon: Module unloaded.");
}

/* EOF */ 
