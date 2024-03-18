#include "module.h"
#define AUTHOR "Roy"
#define VERSION "1.0"

/* ------------------------------------------------------------------------------------
 * Name		: hs_operon
 * Author	: royteeuwen
 * Version	: 1.0
 * Date		: 2nd Nov, 2006
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
 */




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
        NickAlias *na;
	char *buffer = moduleGetLastBuffer();
 	char *option = myStrGetToken(buffer, ' ', 0);
 	User *u2 = finduser(option);


	if (!option) {
	return MOD_CONT;
       }
	if (is_services_admin(u)) {
		char *vHost;
       		char *vIdent = NULL;
       		if ((na = findnick(u2->nick))) {
        		if (na->status & NS_IDENTIFIED) {
           			vHost = getvHost(u2->nick);
            			vIdent = getvIdent(u2->nick);
              			if (vIdent) {
                    			notice_lang(s_HostServ, u2, HOST_IDENT_ACTIVATED, vIdent, vHost);
                		} else {
                   			notice_lang(s_HostServ, u2, HOST_ACTIVATED, vHost);
               			}
                		anope_cmd_vhost_on(u2->nick, vIdent, vHost);           
        		} else {
           			notice_lang(s_HostServ, u2, HOST_ID);
        		}
    		} else {
        		notice_lang(s_HostServ, u2, HOST_NOT_REGED);
    		}
		if(option) {
			free(option);
		}

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
