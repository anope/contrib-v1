/* 
 *  ---------------------------------------------------------------
 *  Name:           os_say_act
 *  Description:    Allows users with CA_SAY rights to act or say via
 *                  the service of choice on specified channel.
 *  Author:         GuruMadMat <gurumadmat@gmail.com>
 *  Date:           04/Oct/2011
 *  Version:        1.0
 *  Commands Added: /msg OperServ Say [Service] [#Channel] [Message]
 *  Commands Added: /msg OperServ Act [Service] [#Channel] [Message]
 *  ---------------------------------------------------------------
 *  ---------------------------------------------------------------
 *  Releases:
 *  1.0   -  First public release to Anope Modules Site.
 * 
 */
#include "module.h"

#define AUTHOR "GuruMadMat"
#define VERSION "os_say_act.c v1.0 by GuruMadMat"

int my_query_say(User *u);
int my_query_act(User *u);
int myQueryHelp_say(User *u);
int myQueryHelp_act(User *u);
void myOperServHelp(User * u);


int AnopeInit(int argc, char **argv) 
{
	

	Command *c_say;
	
	c_say = createCommand("SAY", my_query_say, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c_say, MOD_HEAD);
	
	Command *c_act;
	
	c_act = createCommand("ACT", my_query_act, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c_act, MOD_HEAD);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	moduleSetOperHelp(myOperServHelp);
	moduleAddHelp(c_say, myQueryHelp_say);
	moduleAddHelp(c_act, myQueryHelp_act);
	
	alog("[OS_SAY_ACT]: Module loaded.");
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("[OS_SAY_ACT]: Module unloaded.");
}

void myOperServHelp(User * u)
{
	notice(s_OperServ, u->nick, "    SAY     Let's a Service Send a Channel Message");
	notice(s_OperServ, u->nick, "    ACT     Let's a Service Act on Channel");
}

int myQueryHelp_say(User *u)
{
	notice(s_OperServ, u->nick, "Syntax: \002SAY \037Service\037 \037Channel\037 \037Message\002\037\n");
	notice(s_OperServ, u->nick, " \n");
	notice(s_OperServ, u->nick, "Makes Chosen Service Send Message on Specified Channel.\n");
	notice(s_OperServ, u->nick, "Available Services:\n");
	notice(s_OperServ, u->nick, " \n");
	notice(s_OperServ, u->nick, "OperServ\n");
	notice(s_OperServ, u->nick, "ChanServ\n");
	notice(s_OperServ, u->nick, "NickServ\n");
	notice(s_OperServ, u->nick, "BotServ\n");
	notice(s_OperServ, u->nick, "HostServ\n");
	notice(s_OperServ, u->nick, "MemoServ\n");
	notice(s_OperServ, u->nick, "HelpServ\n");
	notice(s_OperServ, u->nick, " \n");
	notice(s_OperServ, u->nick, "Limited to users who have OS/CA_SAY permission.\n");
    return MOD_CONT;
}


int myQueryHelp_act(User *u)
{
	notice(s_OperServ, u->nick, "Syntax: \002ACT \037Service\037 \037Channel\037 \037Message\002\037\n");
	notice(s_OperServ, u->nick, " \n");
	notice(s_OperServ, u->nick, "Makes Chosen Service Act On Specified Channel.\n");
	notice(s_OperServ, u->nick, "Available Services:\n");
	notice(s_OperServ, u->nick, " \n");
	notice(s_OperServ, u->nick, "OperServ\n");
	notice(s_OperServ, u->nick, "ChanServ\n");
	notice(s_OperServ, u->nick, "NickServ\n");
	notice(s_OperServ, u->nick, "BotServ\n");
	notice(s_OperServ, u->nick, "HostServ\n");
	notice(s_OperServ, u->nick, "MemoServ\n");
	notice(s_OperServ, u->nick, "HelpServ\n");
	notice(s_OperServ, u->nick, " \n");
	notice(s_OperServ, u->nick, "Limited to users who have OS/CA_SAY permission.\n");
    return MOD_CONT;
}
/* ############################## Start here again ##########################
#############################################################################
*/

int is_restricted(char *chosen_serv, char *say_or_act)
{
    	int j;
    	int say_restricted_services_number = 0;
	char *restrict_services_say = NULL;
	char **say_restricted_services;
	
	Directive confvalues[] = {
		{say_or_act, {{PARAM_STRING, PARAM_OPTIONAL, &restrict_services_say}}}
	};
		
	moduleGetConfigDirective(confvalues);

	say_restricted_services = buildStringList(restrict_services_say, &say_restricted_services_number);

    
    /* Look through Directive for restricted service */
   
        for (j = 0; j < say_restricted_services_number; j++) {
            if (stricmp(say_restricted_services[j], chosen_serv) == 0) {
                return 1;
            }
        }
    
    return 0;
}
int my_query_say(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chosen_serv = myStrGetToken(buf, ' ', 0);
	char *chan = myStrGetToken(buf, ' ', 1);
	char *msg = myStrGetTokenRemainder(buf, ' ', 2);
	char *RestrictAllSay = NULL;
	
	Directive confvalues2[] = {
		{"RestrictAllSay", {{PARAM_STRING, PARAM_OPTIONAL, &RestrictAllSay}}}
	};
		
	moduleGetConfigDirective(confvalues2);

	ChannelInfo *ci;
	if ((!chan) || (!msg)) {
		notice(s_OperServ, u->nick, "No Channel or Message Specified!");
	} else if (msg && chan) {
	if (!(ci = cs_findchan(chan))) {
		notice_lang(s_OperServ, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (!nick_identified(u)) {
		notice_lang(s_OperServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
		notice_lang(s_OperServ, u, CHAN_X_FORBIDDEN, chan);
	} else if (!is_services_admin(u) && !check_access(u, ci, CA_SAY)) {
		notice_lang(s_OperServ, u, PERMISSION_DENIED);
	} else if (is_restricted(chosen_serv, "RestrictServicesSay")) {
		notice(s_OperServ, u->nick, "%s is restricted from being used!", chosen_serv);

	} else {
		if (stricmp(chosen_serv, "operserv") == 0)   {

			anope_cmd_privmsg(s_OperServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "chanserv") == 0) {

			anope_cmd_privmsg(s_ChanServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "hostserv") == 0) {

			anope_cmd_privmsg(s_HostServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "nickserv") == 0) {

			anope_cmd_privmsg(s_NickServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "helpserv") == 0) {

			anope_cmd_privmsg(s_HelpServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "memoserv") == 0) {

			anope_cmd_privmsg(s_MemoServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "botserv") == 0) {

			anope_cmd_privmsg(s_BotServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
		} else if (stricmp(chosen_serv, "all") == 0) {
			
			if (RestrictAllSay) {
				anope_cmd_privmsg(s_OperServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_privmsg(s_ChanServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_privmsg(s_NickServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_privmsg(s_BotServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_privmsg(s_HelpServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_privmsg(s_MemoServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

				anope_cmd_privmsg(s_HostServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
			} else {
				notice(s_OperServ, u->nick, "The use of ALL command is restricted!");
			}
		} else {
		
			notice(s_OperServ, u->nick, "%s is not a Service!", chosen_serv);
			
		}
	}
	}
	
	if (chan)
		free(chan);
	
	if (msg)
		free(msg);
	
	return MOD_CONT;
}
int my_query_act(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chosen_serv = myStrGetToken(buf, ' ', 0);
	char *chan = myStrGetToken(buf, ' ', 1);
	char *msg = myStrGetTokenRemainder(buf, ' ', 2);
	char *RestrictAllAct = NULL;
	
	Directive confvalues2[] = {
		{"RestrictAllAct", {{PARAM_STRING, PARAM_OPTIONAL, &RestrictAllAct}}}
	};
		
	moduleGetConfigDirective(confvalues2);
	ChannelInfo *ci;
	
	if ((!chan) || (!msg)) {
		notice(s_OperServ, u->nick, "No Channel or Message Specified!");
	} else if (msg && chan) {
	if (!(ci = cs_findchan(chan))) {
		notice_lang(s_OperServ, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (!nick_identified(u)) {
		notice_lang(s_OperServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
		notice_lang(s_OperServ, u, CHAN_X_FORBIDDEN, chan);
	} else if (!is_services_admin(u) && !check_access(u, ci, CA_SAY)) {
		notice_lang(s_OperServ, u, PERMISSION_DENIED);
	} else if (is_restricted(chosen_serv, "RestrictServicesAct")) {
		notice(s_OperServ, u->nick, "%s is restricted from being used!", chosen_serv);

	} else {
		if (stricmp(chosen_serv, "operserv") == 0)  {

			anope_cmd_action(s_OperServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "chanserv") == 0) {

			anope_cmd_action(s_ChanServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "hostserv") == 0) {

			anope_cmd_action(s_HostServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "nickserv") == 0) {

			anope_cmd_action(s_NickServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "helpserv") == 0) {

			anope_cmd_action(s_HelpServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "memoserv") == 0) {

			anope_cmd_action(s_MemoServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

		} else if (stricmp(chosen_serv, "botserv") == 0) {

			anope_cmd_action(s_BotServ, chan, "%s", msg);
			alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
		} else if (stricmp(chosen_serv, "all") == 0) {

			if (RestrictAllAct) {
				anope_cmd_action(s_OperServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_action(s_ChanServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_action(s_NickServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_action(s_BotServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_action(s_HelpServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
		
				anope_cmd_action(s_MemoServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);

				anope_cmd_action(s_HostServ, chan, "%s", msg);
				alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_OperServ, chan, msg, u->nick, u->vident, u->host);
			} else {
				notice(s_OperServ, u->nick, "The use of ALL command is restricted!");
			}
		} else {
		
			notice(s_OperServ, u->nick, "%s is not a Service!", chosen_serv);
			
		}
	}
	}
	
	if (chan)
		free(chan);
	
	if (msg)
		free(msg);
	
	return MOD_CONT;
}
