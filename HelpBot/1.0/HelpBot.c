/* ----------------------------------------------------------------------------
 * Name    : 	HelpBot.c (my second anope module)
 * Author  : 	bert in IRC
		IRC: channel #cit in irc.khmerchat.net
		Web: http://www.khmerchat.net
 * Version : 	1.0
 * Date    :  	April 3rt, 2010 
 -------------------------------------------------------------------------------
  * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 1, or (at your option) any later
 * version.
 
* Tested: Inspircd 1.20 and Anope 1.8.3 (other IRCD may works, let me know if it does not support it)
 Credit: 	- the author and code diceserv_1.8.c
		- The people of Anope and module site
		- You for reading or using it ;)
		
Feel free to comment it
 -------------------------------------------------------------------------------
 Brief Description: This module will load a new pseudo-client bot HelpBot. It has some available commands
		- join/part: for requesting HelpBot to join/part channel. Limited access to channel founder and SA
		- bjoin/bpart: for requesting other available service bot to join/part channel. Limited access to channel founder and SA
		- bajoin/bapart: for requesting all service bot to join/part the channel. Limited access to SA
 -------------------------------------------------------------------------------
  * Changelog:
  1.0: First release
 
 */

#include "module.h"

#define AUTHOR "bert"
#define VERSION "1.0"

/* bot will auto join this channel by default when loaded */
#define MYSUPPORTCHAN "#support"

/* bot name HelpBot */
static char *const s_HelpBot = "HelpBot";
static CommandHash *HelpBot_cmdTable[MAX_CMD_HASH]; /* Commands for HelpBot */

void helpbot(User * u, char *buf);
int my_privmsg(char *source, int ac, char **av);

int do_helpbothelp(User *u);

int do_helpbotjoin(User *u);
int do_helpbotpart(User *u);
int do_helpbotforceid(User *u);
int do_botjoin(User *u);
int do_botpart(User *u);
int do_bajoin(User *u);
int do_bapart(User *u);


/** Module initialization.
 * @param argc The number of arguments passed to the module
 * @param argv The arguments themselves
 *
 * Initalizes HelpBot by hooking into PRIVMSG, creating the pseudo-client, creating the commands, hooking to events,
 * loading the configuration, adding the language strings, and loading the database.
 */
int AnopeInit(int argc, char **argv)
{
	Command *c = NULL;
	Message *msg = NULL;

	int status;

	msg = createMessage("PRIVMSG", my_privmsg);
	status = moduleAddMessage(msg, MOD_HEAD);
	if (status != MOD_ERR_OK)
	{
		alog("Unable to bind to PRIVMSG!");
		return MOD_STOP;
	}

	anope_cmd_bot_nick(s_HelpBot, ServiceUser, ServiceHost, "General HelpBot", "+");
	
	c = createCommand("JOIN", do_helpbotjoin, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(HelpBot_cmdTable, c, MOD_UNIQUE);
	
	c = createCommand("PART", do_helpbotpart, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(HelpBot_cmdTable, c, MOD_UNIQUE);
	
	c = createCommand("BJOIN", do_botjoin, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(HelpBot_cmdTable, c, MOD_UNIQUE);
	
	c = createCommand("BPART", do_botpart, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(HelpBot_cmdTable, c, MOD_UNIQUE);

	c = createCommand("BAJOIN", do_bajoin, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddCommand(HelpBot_cmdTable, c, MOD_UNIQUE);
	
	c = createCommand("BAPART", do_bapart, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddCommand(HelpBot_cmdTable, c, MOD_UNIQUE);
	
	c = createCommand("HELP", do_helpbothelp, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(HelpBot_cmdTable, c, MOD_UNIQUE);
	
    alog("HelpBot.so: loaded, message status [%d]", status);
    anope_cmd_join(s_HelpBot, MYSUPPORTCHAN, time(NULL));

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}


//help
int do_helpbothelp(User *u)
{
    char *cmd = strtok(NULL, "");
    if (cmd){
      if (!stricmp(cmd, "join")) {
			notice(s_HelpBot, u->nick, "Syntax: \2JOIN <#Channel>\2");
			notice(s_HelpBot, u->nick, "");
			notice(s_HelpBot, u->nick,	"Request %s to join your channel.", s_HelpBot);
	   } else if (!stricmp(cmd, "part")) {
			notice(s_HelpBot, u->nick, "Syntax: \2PART <#Channel>\2");
			notice(s_HelpBot, u->nick, "");
			notice(s_HelpBot, u->nick,	"Request %s to part from your channel.", s_HelpBot);
	   } else if (!stricmp(cmd, "bjoin")) {
			notice(s_HelpBot, u->nick, "Syntax: \2BJOIN BOTNAME <#Channel>\2");
			notice(s_HelpBot, u->nick, "");
			notice(s_HelpBot, u->nick,	"Request a service bot to join your channel.");
	   } else if (!stricmp(cmd, "bpart")) {
			notice(s_HelpBot, u->nick, "Syntax: \2BPART BOTNAME <#Channel>\2");
			notice(s_HelpBot, u->nick, "");
			notice(s_HelpBot, u->nick,	"Request a service bot to part from your channel.");
	   } else if (!stricmp(cmd, "bajoin")) {
			notice(s_HelpBot, u->nick, "Syntax: \2BAJOIN <#Channel>\2");
			notice(s_HelpBot, u->nick, "");
			notice(s_HelpBot, u->nick,	"Request all service bots to join a channel.");
	   } else if (!stricmp(cmd, "bapart")) {
			notice(s_HelpBot, u->nick, "Syntax: \2BAPART <#Channel>\2");
			notice(s_HelpBot, u->nick, "");
			notice(s_HelpBot, u->nick,	"Request all service bots to part from a channel.");
	   } else {
			notice(s_HelpBot, u->nick, "Syntax: HELP");
	   }
    } else {
	    notice(s_HelpBot, u->nick, "     JOIN           Let HelpBot join your registered channel");
	    notice(s_HelpBot, u->nick, "");
	    notice(s_HelpBot, u->nick, "     PART           Let HelpBot part from your registered channel");
	    notice(s_HelpBot, u->nick, "");
	    notice(s_HelpBot, u->nick, "     BJOIN          Let a service bot join your registered channel");
	    notice(s_HelpBot, u->nick, "");
	    notice(s_HelpBot, u->nick, "     BPART          Let a service bot part from your registered channel");
	    notice(s_HelpBot, u->nick, "");
		// extra help for oper
		if (is_oper(u)) {
		    notice(s_HelpBot, u->nick, "     BAJOIN         Request all service bots to join a registered channel");
		    notice(s_HelpBot, u->nick, "");
		    notice(s_HelpBot, u->nick, "     BAPART         Request all service bots to part from a registered channel");
		    notice(s_HelpBot, u->nick, "");
		}
	    notice(s_HelpBot, u->nick, " HelpBot is for additional service bot only for supporting Global IRC Network.");	
	}
	return MOD_CONT;
}


/** Handle PRIVMSG sent to HelpBot
 * @param source The source of the message
 * @param ac Number of arguments, must be 2 (target and message)
 * @param av Arguments, should be target and message
 */
int my_privmsg(char *source, int ac, char **av)
{
	User *u;
    char *s;
	Uid *bid;
	
	/* First, some basic checks */
	if (ac != 2) {
		return MOD_CONT; /* Not enough parameters */
	}
	
	// I forgot Inspircd 1.2 uses uid
    if (UseTS6 && ircd->ts6) { 
        u = find_byuid(source);
        if (!u) u = finduser(source);
    } else {
        u = finduser(source);
	}
	
	if (!u) {
		return MOD_CONT; /* Could not find the user */
	}
    /* Channel message */
    /* we should prolly honour the ignore list here, but i cba for this... */
    s = strchr(av[0], '@');
    if (s) {
        *s++ = 0;
        if (stricmp(s, ServerName) != 0)
            return MOD_CONT;
    }
	
    if (*av[0] == '#') {
        return MOD_CONT;
    }

	// inspircd 1.20 uses UID not nick
    if (UseTS6 && ircd->ts6) {
		bid = find_uid(s_HelpBot);
		if (bid && ((stricmp(av[0], bid->uid)) == 0))
		{
			helpbot(u, av[1]);
			return MOD_STOP; /* Was for us, don't allow other modules to override */
		}
	} else {
		// not inspircd 1.20
	    if ((stricmp(av[0], s_HelpBot)) == 0) {     /* its for US! */
	        helpbot(u, av[1]);
	        return MOD_STOP;
	    }
	}
	return MOD_CONT; /* Wasn't for us, return it to the core */
}


//do helpbot join
int do_helpbotjoin(User *u)
{
	ChannelInfo *ci = NULL;
    char *chan = NULL;
    char *text = NULL;
	
    /* Get the last buffer anope recived */
    text = moduleGetLastBuffer();
	if (text) {
	    chan = myStrGetToken(text, ' ', 0);
		if (chan) {
			if (ci = cs_findchan(chan)) {
				// find out if user is real founder or SA
				if (!is_real_founder(u,ci) && !is_services_admin(u)) {
					notice_lang(s_HelpBot, u, PERMISSION_DENIED);
				} else {
					anope_cmd_join(s_HelpBot, chan, time(NULL));
					alog("%s ordered %s to join %s", u->nick, s_HelpBot, chan);
				}
			} else {
				notice_lang(s_HelpBot, u, CHAN_X_NOT_REGISTERED, chan);
			}
		}	
	}
	return MOD_CONT;
}

//do helpbot bot part
int do_helpbotpart(User *u)
{
	ChannelInfo *ci = NULL;
    char *chan = NULL;
    char *text = NULL;
	
    /* Get the last buffer anope recived */
    text = moduleGetLastBuffer();
	if (text) {
	    chan = myStrGetToken(text, ' ', 0);
		if (chan) {
			if (ci = cs_findchan(chan)) {
				// find out if user is real founder or SA
				if (!is_real_founder(u,ci) && !is_services_admin(u)) {
					notice_lang(s_HelpBot, u, PERMISSION_DENIED);
				} else {
					anope_cmd_part(s_HelpBot, chan, NULL);
					alog("%s ordered %s to part %s", u->nick, s_HelpBot, chan);
				}
			} else {
				notice_lang(s_HelpBot, u, CHAN_X_NOT_REGISTERED, chan);
			}
		}	
	}
	return MOD_CONT;
}

//do service bot join
int do_botjoin(User *u)
{
    BotInfo *bi;
	ChannelInfo *ci = NULL;
    char *chan = NULL;
    char *text = NULL;
	char *botname = NULL;
	
    /* Get the last buffer anope recived */
    text = moduleGetLastBuffer();
	if (text) {
	    botname = myStrGetToken(text, ' ', 0);
	    chan = myStrGetToken(text, ' ', 1);
		if (chan && botname) {
			if (ci = cs_findchan(chan)) {
				// find out if user is real founder or SA
				if (!is_real_founder(u,ci) && !is_services_admin(u)) {
					notice_lang(s_HelpBot, u, PERMISSION_DENIED);
				} else {
					bi = findbot(botname);
					if (!bi) {
						notice_lang(s_HelpBot, u, BOT_DOES_NOT_EXIST, botname);
						return MOD_STOP;
					} else if (bi->flags & BI_PRIVATE && !is_oper(u)) {
						notice_lang(s_HelpBot, u, PERMISSION_DENIED);
						return MOD_STOP;
					} else if (ci->flags & CI_VERBOTEN) {
						notice_lang(s_HelpBot, u, CHAN_X_FORBIDDEN, chan);
						return MOD_STOP;
					} else if ((ci->bi) && (stricmp(ci->bi->nick, botname) == 0)) {
						notice(s_HelpBot, u->nick, "Bot %s is already there!", ci->bi->nick);
						return MOD_STOP;
					} else {
						anope_cmd_join(bi->nick, chan, time(NULL));
					}
					alog("%s ordered %s to join %s", u->nick, bi->nick, chan);
				}
			} else {
				notice_lang(s_HelpBot, u, CHAN_X_NOT_REGISTERED, chan);
			}
		}	
	}
	return MOD_CONT;
}

//do service bot part
int do_botpart(User *u)
{
    BotInfo *bi;
	ChannelInfo *ci = NULL;
    char *chan = NULL;
    char *text = NULL;
	char *botname = NULL;
	
    /* Get the last buffer anope recived */
    text = moduleGetLastBuffer();
	if (text) {
	    botname = myStrGetToken(text, ' ', 0);
	    chan = myStrGetToken(text, ' ', 1);
		if (chan && botname) {
			if (ci = cs_findchan(chan)) {
				// find out if user is real founder or SA
				if (!is_real_founder(u,ci) && !is_services_admin(u)) {
					notice_lang(s_HelpBot, u, PERMISSION_DENIED);
				} else {
					bi = findbot(botname);
					if (!bi) {
						notice_lang(s_HelpBot, u, BOT_DOES_NOT_EXIST, botname);
						return MOD_STOP;
					} else if (bi->flags & BI_PRIVATE && !is_oper(u)) {
						notice_lang(s_HelpBot, u, PERMISSION_DENIED);
						return MOD_STOP;
					} else if (ci->flags & CI_VERBOTEN) {
						notice_lang(s_HelpBot, u, CHAN_X_FORBIDDEN, chan);
						return MOD_STOP;
					} else if ((ci->bi) && (stricmp(ci->bi->nick, botname) == 0)) {
						notice(s_HelpBot, u->nick, "%s is assigned bot. You can not part it!", ci->bi->nick);
						return MOD_STOP;
					} else {
						anope_cmd_part(bi->nick, chan, NULL);
					}
					alog("%s ordered %s to join %s", u->nick, bi->nick, chan);
				}
			} else {
				notice_lang(s_HelpBot, u, CHAN_X_NOT_REGISTERED, chan);
			}
		}	
	}
	return MOD_CONT;
}


//all service bots join
int do_bajoin(User *u)
{
    BotInfo *bi;
	ChannelInfo *ci = NULL;
    char *chan = NULL;
    char *text = NULL;
	int i;
	
    /* Get the last buffer anope recived */
    text = moduleGetLastBuffer();
	if (text) {
	    chan = myStrGetToken(text, ' ', 0);
		if (chan) {
			if (ci = cs_findchan(chan)) {
				// find out if user SA
				if (!is_services_admin(u)) {
					notice_lang(s_HelpBot, u, PERMISSION_DENIED);
				} else {
				       for (i = 0; i < 256; i++) {
				            for (bi = botlists[i]; bi; bi = bi->next) {
				                 if (bi->nick == NULL) {
				                     break;
				                 }
								anope_cmd_join(bi->nick, chan, time(NULL));

				           }
				     }
					alog("%s ordered all service bots to join %s", u->nick, chan);
				}
			} else {
				notice_lang(s_HelpBot, u, CHAN_X_NOT_REGISTERED, chan);
			}
		}	
	}
	return MOD_CONT;
}


//all service bots part
int do_bapart(User *u)
{
    BotInfo *bi;
	ChannelInfo *ci = NULL;
    char *chan = NULL;
    char *text = NULL;
	int i;
	
    /* Get the last buffer anope recived */
    text = moduleGetLastBuffer();
	if (text) {
	    chan = myStrGetToken(text, ' ', 0);
		if (chan) {
			if (ci = cs_findchan(chan)) {
				// find out if SA
				if (!is_services_admin(u)) {
					notice_lang(s_HelpBot, u, PERMISSION_DENIED);
				} else {
				       for (i = 0; i < 256; i++) {
				            for (bi = botlists[i]; bi; bi = bi->next) {
				                 if (bi->nick == NULL) {
				                     break;
				                 }
								 // if it is assigned bot
								anope_cmd_part(bi->nick, chan, NULL);
								if (ci->bi) {
									anope_cmd_join(ci->bi->nick, chan, time(NULL));
								}
				           }
				     }
					alog("%s ordered all service bots to part from %s", u->nick, chan);
				}
			} else {
				notice_lang(s_HelpBot, u, CHAN_X_NOT_REGISTERED, chan);
			}
		}	
	}
	return MOD_CONT;
}

/** HelpBot message handler, same as the other pseudo-clients
 * @param u User that invoked HelpBot
 * @param buf The buffer to use for all other commands
 */
void helpbot(User * u, char *buf)
{
    char *cmd, *s;

    cmd = strtok(buf, " ");

    if (!cmd) {
        return;
    } else if (stricmp(cmd, "\1PING") == 0) {
        if (!(s = strtok(NULL, "")))
            s = "\1";
        notice(s_HelpBot, u->nick, "\1PING %s", s);
    } else if (skeleton) {
        notice_lang(s_HelpBot, u, SERVICE_OFFLINE, s_HelpBot);
    } else {
        mod_run_cmd(s_HelpBot, u, HelpBot_cmdTable, cmd);
    }
}

/** Module deinitialization.
 *
 * Upon Anope shutdown or unload of this module, we make the pesudo-client quit and save it's database, as well as do memory cleanup.
 */
void AnopeFini(void)
{
	anope_cmd_quit(s_HelpBot, "Module Unloaded!");
}
