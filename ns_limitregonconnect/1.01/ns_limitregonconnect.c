/* -----------------------------------------------------------
 * Name: ns_limitregonconnect
 * Author: djGrrr <djgrrr@p2p-network.net> +
 *         SGR <Alex_SGR@ntlworld.com>
 * Date: 2006/10/05 (Last update: 2006/10/05)
 * -----------------------------------------------------------
 * Limitations: NONE KNOWN.
 * Tested: Anope 1.7.15 + UnrealIRCd(3.2.5)
 * -----------------------------------------------------------
 *
 * This version has been tested on Unreal 3.2.5. All IRCd's
 * should be compatible with this module.
 *
 * This module is a slight rewrite of the original 
 * ns_limitregonconnect module by SGR. Most of the core code is
 * the same as the original module, but i have changed most of
 * the triggering functions. Many thanks to SGR for writting
 * the original module :)
 *
 * This module now uses config file settings rather than hard coded
 * defines for all settings (except the nick buffer size) to make
 * it much easier to change settings by a simple /OS RELOAD
 *
 * If you find any bugs, please let me know asap via email or on IRC at
 * irc.p2p-network.net with the nick djGrrr :)
 *
 * -----------------------------------------------------------
 *  ChangeLog
 *
 *  1.00 - First release by me
 *       - Fixed memory leak
 *       - Now using EVENT hooks rather than raw messages
 *       - Added protection on the GROUP command as well
 *       - Removed defines for custom messages, i may add
 *         them back in a later version (in a different manner)
 *       - All settings (except the nickbuffer size) can now
 *         be changed with a config varible, rather than
 *         hard coded defines
 *
 *  1.01 - Added in some initialization checks to make sure the 
 *         hooks and commands are added properly
 *       - Fixed version number
 *
 * -----------------------------------------------------------*/

/* -------------------------------------------------------------
 * Place this in your services.conf file                       *
 * -------------------------------------------------------------

# LimitRegOnConnect [OPTIONAL]
# Module: ns_limitregonconnect
#
# Enables protection against users who  try to register
# immeadiately after connecting, this can be very usefull for
# preventing botnets from registering a lot of nicknames as soon
# as they connect. The REGISTER and GROUP commands in nickserv will
# be limited for the ammount of time set in the LimitRegTimeDeny
# config setting
#
LimitRegOnConnect

# LimitRegTimeDeny [OPTIONAL]
# Module: ns_limitregonconnect
#
# This is the ammount of time users will have to wait before they can
# register with NickServ after connecting to the Network. This value
# should be somewhere between 10 seconds and 15 minutes. 3 mminutes
# is recommended. If LimitRegTimeDeny is not specified, a default of
# 1 minute is assumed.
#
LimitRegTimeDeny 3m

# LimitRegSelfRemoval [OPTIONAL]
# Module: ns_limitregonconnect
#
# Enables users to "force expire" the timeout phrobiting them from
# registering. This will allow users to issue the command
# /NickServ REGISTER UNBLOCK or /NickServ GROUP UNBLOCK. When this is done,
# the user will be removed from the temporary 'deny list' before the timeout,
# and will thus allow them to register early.
#
# I recommend AGAINST uncommenting this as it would make it extreamly easy
# for botnets to bypass the protection
#
#LimitRegSelfRemoval

# LimitRegUseNickTracking [OPTIONAL]
# Module: ns_limitregonconnect
#
# If this is enabled and a user /nicks's within the LimitRegTimeDeny
# time; the counter for them will be reset and they will have to wait the
# full LimitRegTimeDeny again. If this not enabled, a user can simply /nick
# to bypass the LimitRegTimeDeny lockout on them registering their nick.
#
# I HIGHLY recommend enabling this, as disabling it would make it very
# simple for botnets to bypass the protection
#
LimitRegUseNickTracking

 *----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
 * Advanced settings.
 * ---------------------------------------------------------------------- */

/* This setting controls how many 'slots' to reserve for new clients connecting.
 * everytime a client connects, they are added to a free slot. After the timeout,
 * they are removed from the slot. When a user tries to register their nick
 * we 1st check if they are in one of the slots, if so, the registration is
 * rejected. It is NOT recommended you change this setting unless you are
 * running services with a small ammount of ram and on a small [<1000 user] or
 * large [>5000 user] network. [DISCOURAGED] [Must be an integer value]
 * *** MUST NOT BE LOWER THAN 80, ideally keep to a power of 2 *** */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 @ I have set this to 8192 as my default because i run a fairly large network @
 @ and i have plenty of ram on my services box. If you run a small sized      @
 @ network (<1000 users on avg) or have very limited ram, you should change   @
 @ this to something more like the default in the original module (1024)      @
 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define NICKBUFFER 8192 // Here we assume that normally, there are no more
                        // than 8192 clients who have connected in the last
		                // LimitRegTimeDeny time limit. Don't worry if there
			            // are sometimes more or less - but feel free to
			            // change this if you are reguraly seeing the
			            // "Hash Table Full" notice.

/* ---------------------------------------------------------------------- */
/* DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING         */
/* ---------------------------------------------------------------------- */

#include "module.h"

#define AUTHOR "djGrrr + SGR"
#define VERSION "1.01"

typedef struct connectnicks CN;

struct connectnicks {
	CN *next, *prev;
	char *nick;
};

CN *cnsh[NICKBUFFER];
int ncns    = 0;
int NSRLock = 0;

CN *find_entry(char *nick);
CN *create_entry(char *nick);
int del_entry(CN *cn);
int rtncount(void);
void add_entry(CN *cn);
void m_regconnecttimeoutloghash(void);

int my_nickservreglock_change(int argc, char **argv);
int my_nickservreglock_connect(int argc, char **argv);
int m_ns_remove_deny_entry(int argc, char **argv);
int m_ns_check_recentconnection(User *u);
int m_ns_someuserquit(int argc, char **argv);

int reload_config(int argc, char **argv);
void load_config(void);

int LimitRegOnConnect = 0;
int LimitRegSelfRemoval = 0;
int LimitRegTimeDeny = 0;
int LimitRegUseNickTracking = 0;

int AnopeInit(int argc, char **argv)
{
	int status = 0;
	Command *c;
	EvtHook *hook = NULL;

	if (!moduleMinVersion(1,7,10,810)) {
		alog("%s: [ns_limitregonconnect]: Your version of Anope isn't supported.", s_OperServ);
		return MOD_STOP;
	}

	load_config();

	hook = createEventHook(EVENT_RELOAD, reload_config); status += moduleAddEventHook(hook);
	hook = createEventHook(EVENT_NEWNICK, my_nickservreglock_connect); status += moduleAddEventHook(hook);
	hook = createEventHook(EVENT_CHANGE_NICK, my_nickservreglock_change); status += moduleAddEventHook(hook);
	hook = createEventHook(EVENT_USER_LOGOFF, m_ns_someuserquit); status += moduleAddEventHook(hook);

	c = createCommand("REGISTER", m_ns_check_recentconnection, NULL,-1,-1,-1,-1,-1);
	status += moduleAddCommand(NICKSERV, c, MOD_HEAD);
	c = createCommand("GROUP", m_ns_check_recentconnection, NULL,-1,-1,-1,-1,-1);
	status += moduleAddCommand(NICKSERV, c, MOD_HEAD);

	if (status != 0) {
		alog("%s: [ns_limitregonconnect]: Module could not load successfully", s_OperServ);
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	moduleSetType(THIRD);

	alog("%s: [ns_limitregonconnect]: Module loaded successfully", s_OperServ);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: [ns_limitregonconnect]: Module unloaded successfully", s_OperServ);
}

int m_ns_check_recentconnection(User *u)
{
	CN *cn;
	if (!LimitRegOnConnect)
		return MOD_CONT;

	if (LimitRegSelfRemoval) {
		char *buffer = moduleGetLastBuffer();
		if (buffer) {
			buffer = myStrGetToken(buffer, ' ', 0);
			if (!stricmp(buffer,"UNBLOCK")) {
				if ((cn = find_entry(u->nick))) {
					del_entry(cn);
					notice(s_NickServ, u->nick, "The timout preventing you from REGISTERing / GROUPing has been removed.");
					notice(s_NickServ, u->nick, "You may now register your nick. See /msg %s HELP REGISTER", s_NickServ);
					notice(s_NickServ, u->nick, "for further information on how to register.");
				}
				else
					notice(s_NickServ, u->nick, "No matching timeouts were found. You may now register your nick.");

				free(buffer);

				return MOD_STOP;
			}

			free(buffer);
		}
	}

	if ((cn = find_entry(u->nick))) {
		notice(s_NickServ, u->nick, "You must be connected for more than %d seconds before you", LimitRegTimeDeny);
		notice(s_NickServ, u->nick, "may REGISTER / GROUP your nickname with NickServ. If you have changed");
		notice(s_NickServ, u->nick, "your nick within %d seconds the timeout will of been reset.", LimitRegTimeDeny);

		if (LimitRegSelfRemoval) {
			notice(s_NickServ, u->nick, " ");
			notice(s_NickServ, u->nick, "If you do not wish to wait, you can lift this temporary block");
			notice(s_NickServ, u->nick, "using /msg %s REGISTER UNBLOCK.", s_NickServ);
		}
		notice(s_NickServ, u->nick, " ");
		notice(s_NickServ, u->nick, "For more information please join #help");
		return MOD_STOP;
	}
	else
		return MOD_CONT;
}

int m_ns_remove_deny_entry(int argc, char **argv)
{
	char *peep = argv[0];
	CN *cn;
	if (peep) {
		while((cn = find_entry(peep)))
			del_entry(cn);
	}

	return MOD_CONT;
}

int m_ns_someuserquit(int argc, char **argv)
{
	CN *cn;
	if (argc < 1)
		return MOD_CONT;

	if ((cn = find_entry(argv[0])))
		del_entry(cn);

	return MOD_CONT;
}


int my_nickservreglock_change(int argc, char **argv)
{
	char NSRLockX[16];
	char *av[1];
	CN *cn;
	int XM = 0;
    
	if (!LimitRegOnConnect || !LimitRegUseNickTracking || argc < 1)
		return MOD_CONT;

	if ((cn = find_entry(argv[0]))) {
		del_entry(cn);
		XM = rtncount();
		if (XM >= NICKBUFFER) {
			m_regconnecttimeoutloghash();
			return MOD_CONT;
		}
		if ((cn = create_entry(argv[0]))) {
			av[0] = sstrdup(argv[0]);
			NSRLock = NSRLock + 1;
			snprintf(NSRLockX, sizeof(NSRLockX), "CNRD-%d", NSRLock);
			moduleAddCallback(NSRLockX, time(NULL) + LimitRegTimeDeny, m_ns_remove_deny_entry,1,av);
			free(av[0]);
		}
	}

	return MOD_CONT;
}

int my_nickservreglock_connect(int argc, char **argv)
{
	char NSRLockX[16];
	char *av[1];
	CN *cn;
	int XM = 0;
    
	if (!LimitRegOnConnect || argc != 1)
		return MOD_CONT;

	if ((cn = find_entry(argv[0])))
		del_entry(cn);

	XM = rtncount();
	if (XM >= NICKBUFFER) {
		m_regconnecttimeoutloghash();
		return MOD_CONT;
	}
	if ((cn = create_entry(argv[0]))) {
		av[0] = sstrdup(argv[0]);
		NSRLock = NSRLock + 1;
		snprintf(NSRLockX, sizeof(NSRLockX), "CNRD-%d", NSRLock);
		moduleAddCallback(NSRLockX, time(NULL) + LimitRegTimeDeny, m_ns_remove_deny_entry,1,av);
		free(av[0]);
	}
	if (NSRLock > 9999999)
		NSRLock = 1;

	return MOD_CONT;
}

int del_entry(CN *cn)
{
	if (cn->next)
		cn->next->prev = cn->prev;
	if (cn->prev)
		cn->prev->next = cn->next;
	else
		cnsh[tolower(*cn->nick)] = cn->next;
	ncns--;
	free(cn->nick);
	free(cn);
	return 1;
}

CN *find_entry(char *nick)
{
	CN *cn;
	if (!nick || !*nick)
        return NULL;

	for (cn = cnsh[tolower(*nick)]; cn; cn = cn->next) {
		if (!stricmp(nick, cn->nick))
			return cn;
	}
	return NULL;
}

void add_entry(CN * cn)
{
	CN *next, *prev;

	for (prev = NULL, next = cnsh[tolower(*cn->nick)]; next != NULL && stricmp(next->nick, cn->nick) < 0; prev = next, next = next->next);
	cn->prev = prev;
	cn->next = next;
	if (!prev)
		cnsh[tolower(*cn->nick)] = cn;
	else
		prev->next = cn;
	if (next)
		next->prev = cn;

	return;
}

CN *create_entry(char *nick)
{
	CN *cn;
	cn = scalloc(sizeof(CN), 1);
	cn->nick = sstrdup(nick);
	add_entry(cn);
	ncns++;
	return cn;
}

int rtncount(void)
{
	int i = 0;
	CN *cn;
	int count = 0;

	if (!ncns)
		return 0;

	for (i = 0; i < NICKBUFFER; i++) {
		for (cn = cnsh[i]; cn; cn = cn->next) {
			count++;
			if (count > NICKBUFFER)
				break;
		}
	}
	return count;
}

void m_regconnecttimeoutloghash(void)
{
	int D = 0;
	D = LimitRegTimeDeny * 4;
	alog("%s: [ns_limitregonconnect]: Hash Table Full - This isn't too much of a problem, but if you are getting", s_OperServ);
	alog("%s: [ns_limitregonconnect]: this notice reguraly (and outside of %d secs of the 1st of these messages)", s_OperServ, D);
	alog("%s: [ns_limitregonconnect]: considder recompiling the module with a larger NICKBUFFER setting.", s_OperServ);
	alog("%s: [ns_limitregonconnect]: Note: this error is common when servers are relinking, or you are recieving", s_OperServ);
	alog("%s: [ns_limitregonconnect]: many new client connections e.g. when a botnet joins your network.", s_OperServ);
}

int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("%s: [ns_limitregonconnect]: Reloading configuration directives...", s_OperServ);
			load_config();
		}
	}
	return MOD_CONT;
}

void load_config(void)
{
	int i = 0;

	LimitRegOnConnect = 0;
	LimitRegSelfRemoval = 0;
	LimitRegUseNickTracking = 0;
	LimitRegTimeDeny = 0;

	Directive confvalues[][1] = {
		{{"LimitRegOnConnect", {{PARAM_SET, PARAM_RELOAD, &LimitRegOnConnect}}}},
		{{"LimitRegSelfRemoval", {{PARAM_SET, PARAM_RELOAD, &LimitRegSelfRemoval}}}},
		{{"LimitRegUseNickTracking", {{PARAM_SET, PARAM_RELOAD, &LimitRegUseNickTracking}}}},
		{{"LimitRegTimeDeny", {{PARAM_TIME, PARAM_RELOAD, &LimitRegTimeDeny}}}}
	};

	for (i = 0; i < 4; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (LimitRegOnConnect && (!LimitRegTimeDeny || LimitRegTimeDeny < 0))
		LimitRegTimeDeny = 60;
}
