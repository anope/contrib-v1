/**
 ***********
 * Module Name: os_jail
 * Author: Adam Walker - http://waterfieldinternet.com
 * Created on: 21/06/2010
 * More info at: http://irc.waterfieldinternet.com/forum
 ***********
 *
 * This module adds a !jail fantasy command to BotServ.
 * This enables any IRC Operator to "jail" a user by
 * forcing them to part all channels then join a custom
 * jail channel where an IRC Operator can talk to them
 * privately.
 *
 * Commands:
 *
 * - !jail <nickname>
 * - /msg operserv jail <nickname>
 *
 * Configuation Options:
 * 
 * JailChannel "#TheBox" - Must be set for the script to work
 * 
 ***********
 * Last Updated: 29/06/2010
 * v1.0.0 - No Date as of Yet - Initial release
 * v1.0.1 - Auto Join JailChannel for jailer (IRCOP)
 * v1.1.0 - OperServ command added /os jail <nick>
 *
 **/

#include "module.h"
#include <time.h>

#define AUTHOR			"Adam Walker - http://waterfieldinternet.com"
#define VERSION			"$Id: os_jail.c v1.1.0 29/06/2010 Adam Walker $"

/* Configurable Variables */
#define JAIL_SYNTAX		"Syntax: jail <nickname>"
#define JAIL_NO_NICK		"Error: No Such User is Online"
#define JAIL_SUCCESS		"Success: %s has been jailed."
#define JAIL_USER_INFO		"Notice: You have been jailed by %s."
#define JAIL_USER_WARN		"Notice: You are being forced to Part All Channels"
#define JAIL_OP_NOTICE		"\002%s\002 has jailed \002%s\002"
#define JAIL_FATAL_ERROR	"Error: There was an unexpected error."

/* ========================== */
/* DO NOT EDIT PAST THIS LINE */
/* ========================== */

/* Config Settings */
char *J_CHANNEL = NULL;

/* Main Module */
int load_config();
int reload_config(int ac, char **av);
int do_jail(int ac, char **av);
int do_userjail(User *u);
int lol_test(int ac, char **av);
int myOperServOperHelp(User *u);
void myOperServHelp(User*u);

/* Methods for responding to !jail */
void do_jail_user(User *u, char *arg);

/**
 * Module Load
 **/
int AnopeInit(void)
{
  Command *c;
  int status = 0;
  c = createCommand("JAIL", do_userjail, NULL, -1, -1, -1, -1, -1);
  moduleAddHelp(c, myOperServOperHelp);
  moduleSetOperHelp(myOperServHelp);

  status = moduleAddCommand(OPERSERV, c, MOD_HEAD);
  if (status != MOD_ERR_OK) {
	return MOD_STOP;
  } else {
	alog("%s: Providing command '\2/msg OperServ HELP JAIL\2'", s_OperServ);
  }

  EvtHook *hook;
  
  moduleAddAuthor(AUTHOR);
  moduleAddVersion(VERSION);

  hook = createEventHook(EVENT_BOT_FANTASY, &do_jail);
  if (moduleAddEventHook(hook) != MOD_ERR_OK)
  {
	alog("[\002os_jail\002] Can't hook to EVENT_BOT_FANTASY event");
	return MOD_STOP;
  }

  hook = createEventHook(EVENT_RELOAD, reload_config);
  if (moduleAddEventHook(hook) != MOD_ERR_OK)
  {
	alog("[\002os_jail\002] Can't hook to EVENT_RELOAD event");
	return MOD_STOP;
  }

  /* Load config before we go any further */
  if(load_config() == MOD_STOP) return MOD_STOP;

  /* No Errors, Module Loading Successful! */
  alog("[\002os_jail\002] Module Loaded Successfully!");
  return MOD_CONT;
}

void myOperServHelp(User *u)
{
	notice(s_OperServ, u->nick, "    JAIL        Jail a User (Part channels & join %s)", J_CHANNEL);
}

int myOperServOperHelp(User *u)
{
	notice(s_OperServ, u->nick, "Syntax: \2JAIL \037nick\037\2");
	notice(s_OperServ, u->nick, " ");
	notice(s_OperServ, u->nick, "The JAIL command is used to jail a user.");
	notice(s_OperServ, u->nick, "This will force the user to part all channels");
	notice(s_OperServ, u->nick, "and force them to join %s.", J_CHANNEL);
	notice(s_OperServ, u->nick, " ");
	notice(s_OperServ, u->nick, "Restricted to IRC Operators");
return MOD_CONT;
}

/**
 * Module unload
 **/
void AnopeFini(void)
{
  alog("[\002os_jail\002] Module Un-Loaded Successfully!");
}

/**
 * Load Config
 **/
int load_config(void)
{
  Directive confvalues[][1] = {
	{{ "JailChannel",	{ { PARAM_STRING, PARAM_RELOAD, &J_CHANNEL } } }}
  };

/* Get each config option */
 unsigned int i = 0;
 for (i = 0; i < 1; i++)
 {
	moduleGetConfigDirective(confvalues[i]);
 }

/* Check the Values... */
 if(!J_CHANNEL)
 {
 	alog("[\002os_jail\002] Missing Configuration Value - JailChannel");
 	return MOD_STOP;
 } else {
 	alog("[\002os_jail\002] Configuration Value - JailChannel (J_CHANNEL): %s", J_CHANNEL);
 }

 return MOD_CONT;
}

/**
 * Reload Configuration on /os reload
 **/
int reload_config(int ac, char **av)
{
  if(ac >= 1)
  {
    if(!stricmp(av[0], EVENT_START))
    {
	alog("[\002os_jail\002] Reloading configuration directives...");
	return load_config();
    }
  }
  return MOD_CONT;
}

int do_userjail(User *u)
{
	User *u2;
	NickAlias *na;
	char *buf = moduleGetLastBuffer();
	char *nick = myStrGetToken(buf, ' ', 0);
	char *vHost;
	char *vIdent = NULL;
	
	if (!nick) {
		notice(s_OperServ, u->nick, "Syntax: \2JAIL \037nick\037\2");
	} else if (nickIsServices(nick, 1)) {
		notice(s_OperServ, u->nick, "Nick \2%s\2 is part of this Network Services!", nick);
	} else if (!(u2 = finduser(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
	} else if (na->status & NS_VERBOTEN) {
		notice_lang(s_OperServ, u, NICK_X_FORBIDDEN, na->nick);
	} else {
                anope_cmd_svsjoin(s_OperServ, nick, "#1,0", NULL);
                anope_cmd_svsjoin(s_OperServ, nick, J_CHANNEL, NULL);
                anope_cmd_svsjoin(s_OperServ, u->nick, J_CHANNEL, NULL);
		if (WallOSGlobal)
		        anope_cmd_global(s_OperServ, JAIL_OP_NOTICE,
                         u->nick, nick);
        }
}

int do_jail(int ac, char **av)
{
  User *u;
  ChannelInfo *ci;
  Channel *c;

  /* Some basic error checking... should never match */
  if (ac < 3) return MOD_CONT;

  if (!(ci = cs_findchan(av[2]))) return MOD_CONT;
  if (!(u = finduser(av[1])))     return MOD_CONT;
  if (!(c = findchan(ci->name)))  return MOD_CONT;

  /* Only check !jail commands */
  if (stricmp(av[0], "jail") != 0) return MOD_CONT;

  if (is_oper(u))
  {
  /* Only our messages from now on */
  /* Just !jail on it's own? */
  if (ac == 3)
  {
    anope_cmd_notice2(ci->bi->nick, u->nick, JAIL_SYNTAX);
    /** char * string = (char*)malloc(sizeof(char)*512);
     *sprintf(string, "[\002os_jail\002] Debug Marker - Bot Nick: %s User Nick: %s On Channel: %s", ci->bi->nick, u->nick, ci->name);
     *alog(string);
     *free(string);
    **/
    return MOD_CONT;
  } else {
    char * jailstring = (char*)malloc(sizeof(char)*512);
    sprintf(jailstring, JAIL_USER_INFO, u->nick);
    anope_cmd_notice2(s_OperServ, av[3], jailstring);
    anope_cmd_notice2(s_OperServ, av[3], JAIL_USER_WARN);
    anope_cmd_svsjoin(s_OperServ, av[3], "#1,0", NULL);
    anope_cmd_svsjoin(s_OperServ, av[3], J_CHANNEL, NULL);
    anope_cmd_svsjoin(s_OperServ, u->nick, J_CHANNEL, NULL);
    if (WallOSGlobal)
        anope_cmd_global(s_OperServ, JAIL_OP_NOTICE,
                         u->nick, av[3]);
    alog("[\002os_jail\002] Jail for \002%s\002 made by \002%s\002", av[3], u->nick);
  }
  }
}

