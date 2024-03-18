/* ----------------------------------------------------------------------------------------------
 * Name		: cs_join.c 
 * Author	: n00bie
 * Version	: v1.0.3
 * Date		: 11/08/2006
 * Update	: 29/09/2006
 * ----------------------------------------------------------------------------------------------
 * <Description>
 *		This module will make ChanServ JOIN / PART a channel & will automatically
 *		set correct modes for itself.
 *
 *		This module is designed for ppl who wanted to have @ChanServ sits on their
 *		channel along with a BotServ @Bots; and without the need of using ugly RAW commands for
 *		making ChanServ join a channel.
 *
 *		It is recommended not to use this module with cs_fantasy.c or
 *		cs_inhabitregistered.c loaded as those modules already does the job ;)
 *
 *		Also, when using this module i'd like to suggest the SYMBIOSIS mode turned off
 *		(if there is a bot assigned on a channel). See /BotServ HELP SET SYMBIOSIS for reasons.
 * </Description>
 *
 * NOTE: Only channel founder can use this command.
 * ----------------------------------------------------------------------------------------------
 * Providing Commands:
 *		/msg ChanServ JOIN #Channel
 *		/msg ChanServ PART #Channel
 *
 * Providing IRCD Handler for: KICK
 * ----------------------------------------------------------------------------------------------
 * Tested: UnrealIRCd 3.2.5 & Anope-1.7.13+above
 *
 * Change Log:
 * Ver1.0.0 - First public release
 * Ver1.0.2 - 1) Removed strtok();
 *            2) Fixed mem leak
 * Ver1.0.3 - Changed mode +oq to correct modes depending on the IRCd
 *
 * TODO   	: CS_JOIN_ALREADY
 * ----------------------------------------------------------------------------------------------
 * Thanks	: Tom65789 for his comments/suggestions ^^
 *            chaz for compiling the mod for windows users
 * ----------------------------------------------------------------------------------------------
*/

#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_join.c v1.0.3 29-09-2006 n00bie $"

int do_cs_join(User *u);
int do_cs_part(User *u);
int do_cs_help_join(User *u);
int do_cs_help_part(User *u);
int do_kick_rejoin(char *source, int ac, char **av);
void myChanServHelp(User *u);

int AnopeInit(int argc, char **argv)
{
	Command *c;
	Message *msg = NULL;
	c = createCommand("JOIN", do_cs_join, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_cs_help_join);
	alog("%s: cs_join: Added command 'JOIN", s_ChanServ);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleSetChanHelp(myChanServHelp);
	c = createCommand("PART", do_cs_part, NULL, -1, -1, -1, -1, -1);
	alog("%s: cs_join: Added command 'PART'", s_ChanServ);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleAddHelp(c, do_cs_help_part);
	msg = createMessage("KICK", do_kick_rejoin);
	moduleAddMessage(msg, MOD_TAIL);
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	alog("%s: cs_join: Successfully loaded module.", s_ChanServ);
	return MOD_CONT;
}

int do_cs_join(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(s_ChanServ, u->nick, "You did not specify a channel to join.");
		notice(s_ChanServ, u->nick, "Syntax: \2JOIN\2 \037#CHANNEL\037");
	} else if (!(ci = cs_findchan(chan))) {
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (!nick_identified(u)) {
		notice_lang(s_ChanServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	} else if (!is_founder(u, ci)) {
		notice(s_ChanServ, u->nick, "Permission denied. You must be a channel founder on '%s'", chan);
		if (LogChannel) {
			alog("%s: Access denied for \2%s\2 with command JOIN %s", s_ChanServ, u->nick, chan);
		}
	} else {
		anope_cmd_join(s_ChanServ, chan, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") || 
			!stricmp(IRCDModule, "plexus") || 
			!stricmp(IRCDModule, "ptlink") || 
			!stricmp(IRCDModule, "inspircd") || 
			!stricmp(IRCDModule, "ultimate2") || 
			!stricmp(IRCDModule, "unreal32") || 
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_ChanServ, ci->name, "+ao %s %s", s_ChanServ, s_ChanServ);
				notice(s_ChanServ, u->nick, "Successfully joined %s", chan);
		} else {
			anope_cmd_mode(s_ChanServ, ci->name, "+o %s", s_ChanServ);
			notice(s_ChanServ, u->nick, "Successfully joined %s", chan);
		}
	}
	if (chan) free(chan);
	return MOD_CONT;
}

int do_cs_part(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(s_ChanServ, u->nick, "Syntax: \2PART\2 \037#CHANNEL\037");
	} else if (!(ci = cs_findchan(chan))) {
		return MOD_CONT;
	} else if (!nick_identified(u)) {
		notice_lang(s_ChanServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if (!is_founder(u, ci)) {
		notice(s_ChanServ, u->nick, "Permission denied. You must be a channel founder on '%s'", chan);
		if (LogChannel) {
			alog("%s: Access denied for \2%s\2 with command PART %s", s_ChanServ, u->nick, chan);
		}
	} else {
		anope_cmd_part(s_ChanServ, chan, "PART command from %s", u->nick);
		notice(s_ChanServ, u->nick, "Successfully parted %s", chan);
	}
	if (chan) free(chan);
	return MOD_CONT;
}

int do_kick_rejoin(char *source, int ac, char **av)
{
	if ((!(stricmp(av[1], s_ChanServ) == 0))) {
		return MOD_CONT;
	}
	if (LogChannel) {
		alog("%s got kicked from '%s' by %s (Auto re-joining)", s_ChanServ, av[0], source);
	}
	anope_cmd_join(s_ChanServ, av[0], time(NULL));
	if (!stricmp(IRCDModule, "inspircd") || 
		!stricmp(IRCDModule, "plexus") || 
		!stricmp(IRCDModule, "ptlink") || 
		!stricmp(IRCDModule, "inspircd") || 
		!stricmp(IRCDModule, "ultimate2") || 
		!stricmp(IRCDModule, "unreal32") || 
		!stricmp(IRCDModule, "viagra")) { 
			anope_cmd_mode(s_ChanServ, av[0], "+ao %s %s", s_ChanServ, s_ChanServ);
	} else {
		anope_cmd_mode(s_ChanServ, av[0], "+o %s", s_ChanServ);
	}
	return MOD_CONT;
}

int do_cs_help_join(User *u)
{
	notice(s_ChanServ, u->nick, "Syntax: \2JOIN\2 \037#Channel\037");
	notice(s_ChanServ, u->nick, " ");
	notice(s_ChanServ, u->nick, "This command will make \2%s\2 join the channel you specified.", s_ChanServ);
	notice(s_ChanServ, u->nick, "Note that when %s joins the channel, it will automatically Opped", s_ChanServ);
	notice(s_ChanServ, u->nick, "itselves on the channel and will remain on the channel untill parted");
	notice(s_ChanServ, u->nick, "using the \2PART\2 command or when Services get restarted.");
	notice(s_ChanServ, u->nick, " ");
	notice(s_ChanServ, u->nick, "\2NOTE\2: In order to make \2%s\2 join a channel,", s_ChanServ);
	notice(s_ChanServ, u->nick, "you must be a Channel Founder on that channel.");
	return MOD_CONT;
}

int do_cs_help_part(User *u)
{
	notice(s_ChanServ, u->nick, "Syntax: \2PART\2 \037#Channel\037");
	notice(s_ChanServ, u->nick, " ");
	notice(s_ChanServ, u->nick, "This command will make \2%s\2 part the channel you specified.", s_ChanServ);
	return MOD_CONT;
}

void myChanServHelp(User *u)
{
	notice(s_ChanServ, u->nick, "    JOIN       Makes ChanServ join a channel");
	notice(s_ChanServ, u->nick, "    PART       Makes ChanServ part a channel");
}

void AnopeFini(void)
{
	alog("%s: cs_join: module unloaded.", s_ChanServ);
}

/* EOF */

