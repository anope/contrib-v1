/**
 * -----------------------------------------------------------
 * Name: os_svsjoinpart
 * Author: Viper <Viper@Absurd-IRC.net>
 * Date: 28/07/2006
 * -----------------------------------------------------------
 * Functions: do_svsjoin, do_svspart
 * Limitations: IRCD must support svsjoin and svspart
 * Tested: Anope 1.7.15 + UnrealIRCd 3.2.3
 * -----------------------------------------------------------
 * This module adds an SVSJOIN and SVSPART command to operserv.
 *
 * Module only tested on UnrealIRCd, but should work on any ircd.
 * This module will only work on Anope 1.7.15+.
 *
 * This module is based on the original os_svsjoinpart by SGR, so
 * a lot of credit goes to him.
 * Used the IRCd check from Trystan's version of this module.
 * -----------------------------------------------------------
 * Changelog:
 *   3.0  -  First release by me, reworked and updated SGR's code
 * -----------------------------------------------------------
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "3.0"

/* Language defines */
#define LANG_NUM_STRINGS 					13

#define LANG_SVSJOIN_INFO					0
#define LANG_SVSJOIN_SYNTAX					1
#define LANG_SVSJOIN_SYNTAX_EXT				2
#define LANG_SVSPART_INFO					3
#define LANG_SVSPART_SYNTAX					4
#define LANG_SVSPART_SYNTAX_EXT				5
#define LANG_NO_SUCH_USER					6
#define LANG_CHANNAME_TOO_LONG				7
#define LANG_CHANNAME_ILLEGAL				8
#define LANG_FORCE_JOINED					9
#define LANG_FORCE_PARTED					10
#define LANG_USER_ALLREADY_IN_CHAN			11
#define LANG_USER_NOT_IN_CHAN				12

/* Functions */
int help_svsjoin(User *u);
int help_svspart(User *u);
int do_svsjoin(User * u);
int do_svspart(User * u);
void list_svsjoinpart(User *u);
int valid_ircd(void);
void add_languages(void);


int AnopeInit(int argc, char **argv) {
	Command *c;
	alog("Trying to load module os_svsjoinpart.so...");

    if (!valid_ircd()) {
        alog("[os_svsjoinpart] ERROR: IRCd not supported by this module");
        alog("[os_svsjoinpart] ERROR: No new commands available.");
        alog("[os_svsjoinpart] Auto-Unloading module.");
        return MOD_STOP;
    }

	c = createCommand("SVSJOIN",do_svsjoin,is_services_admin,-1,-1,-1,-1,-1);
	moduleAddCommand(OPERSERV,c,MOD_HEAD);
	moduleAddHelp(c,help_svsjoin);

	c = createCommand("SVSPART",do_svspart,is_services_admin,-1,-1,-1,-1,-1);
	moduleAddCommand(OPERSERV,c,MOD_HEAD);
	moduleAddHelp(c,help_svspart);
	moduleSetOperHelp(list_svsjoinpart);

	add_languages();
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002os_svsjoinpart\002] Module loaded...");

	return MOD_CONT;
}

void AnopeFini(void) {
	alog("[\002os_svsjoinpart\002] Unloading module...");
}


/**
 * Help functions
 **/
int help_svsjoin(User *u) {
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, LANG_SVSJOIN_SYNTAX_EXT);
	} else
   		notice_lang(s_OperServ, u, PERMISSION_DENIED);
	return MOD_STOP;
}

int help_svspart(User *u) {
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, LANG_SVSPART_SYNTAX_EXT);
	} else
   		notice_lang(s_OperServ, u, PERMISSION_DENIED);
	return MOD_STOP;
}

void list_svsjoinpart(User *u) {
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, LANG_SVSJOIN_INFO);
		moduleNoticeLang(s_OperServ, u, LANG_SVSPART_INFO);
	} else
   		notice_lang(s_OperServ, u, PERMISSION_DENIED);
}


/**
 * We handle SVSJOIN
 **/
int do_svsjoin(User * u) {
	char *tobejoined = strtok(NULL, " ");
	char *joinchan = strtok(NULL, " ");
	char *issilent = strtok(NULL, "");
	User *target;
	ChannelInfo *ci;
	double chanmaxtot;
	double actualchanname;


	/* check whether all parameters are present */
	if (!tobejoined || !joinchan) {
		moduleNoticeLang(s_OperServ, u, LANG_SVSJOIN_SYNTAX);
		return MOD_STOP;
	}

	/* Only allow this if SuperAdmin is enabled */
	if (!u->isSuperAdmin) {
		notice_lang(s_OperServ, u, OPER_SUPER_ADMIN_ONLY);
		return MOD_STOP;
	}

	/* Check the 1st argument [nick] is a valid (in use) nick -
	 * This will filter out services/botserv bots.
	 */
	if (!(target = finduser(tobejoined))) {
		moduleNoticeLang(s_OperServ, u, LANG_NO_SUCH_USER, tobejoined);
		return MOD_STOP;
	}

	/* If channel name is too long, reject.
	 *
	 * joinchan is not truncated (which would allow the command to contine) for a reason ;)
	 *
	 * be glad chanmaxtot and actualchanname are declared as double - took me 40 mins to
	 * figure out why it segfaulted on > 62 chan with this code having them def'd as int.
	 */
	chanmaxtot = CHANMAX - 2;
	actualchanname = strlen(joinchan);
	if (actualchanname > chanmaxtot) {
		moduleNoticeLang(s_OperServ, u, LANG_CHANNAME_TOO_LONG);
		return MOD_STOP;
	}

	/* Check Parameter 1 is a valid channel name. */
	if (joinchan[0] != '#') {
		/* if not - bitch and quit */
		moduleNoticeLang(s_OperServ, u, LANG_CHANNAME_ILLEGAL, joinchan);
		return MOD_STOP;
	}

	/* Check for forbidden/suspended Channel - there is no point joining a user to
	 * a channel there just going to get kick-banned from ;)
	 */
	if (!(ci = cs_findchan(joinchan))) {
		if (ci->flags & CI_VERBOTEN) {
			notice_lang(s_OperServ, u, CHAN_X_FORBIDDEN, joinchan);
			return MOD_STOP;
		}
	}

	/* Check whether user is allready on channel */
	if (is_on_chan(ci->c, target)) {
		moduleNoticeLang(s_OperServ, u, LANG_USER_ALLREADY_IN_CHAN, tobejoined);
		return MOD_STOP;
	}

	if (!issilent) {
		notice(s_OperServ, u->nick, "Forcing %s to join %s", tobejoined, joinchan);

		/* make some noise to prevent abusers */
		wallops(s_OperServ, "%s used SVSJOIN to join %s to %s", u->nick, tobejoined, joinchan);
		moduleNoticeLang(s_OperServ, u, LANG_FORCE_JOINED, joinchan, u->nick);

		/* Force them in */
		// we ll use anope's provided function here...
		anope_cmd_svsjoin(s_OperServ, target->nick, ci->name);

	} else {
		if (!stricmp(issilent,"silent"))  {
			notice(s_OperServ, u->nick, "Forcing %s to join %s", tobejoined, joinchan);

			/* make some noise to prevent abusers */
			wallops(s_OperServ, "%s used SVSJOIN to join %s to %s", u->nick, tobejoined, joinchan);

			/* Force them in */
			// we ll use anope's provided function here...
			anope_cmd_svsjoin(s_OperServ, target->nick, ci->name);

		} else
			moduleNoticeLang(s_OperServ, u, LANG_SVSJOIN_SYNTAX);
	}
	return MOD_STOP;
}


/**
 * We handle SVSPART
 **/
int do_svspart(User * u) {
	char *tobeparted = strtok(NULL, " ");
	char *partchan = strtok(NULL, " ");
	char *issilent = strtok(NULL, " ");
	User *target;
	ChannelInfo *ci;
	double chanmaxtot;
	double actualchanname;


	/* check whether all parameters are present */
	if (!tobeparted || !partchan) {
		moduleNoticeLang(s_OperServ, u, LANG_SVSJOIN_SYNTAX);
		return MOD_STOP;
	}

	/* Only allow this if SuperAdmin is enabled */
	if (!u->isSuperAdmin) {
		notice_lang(s_OperServ, u, OPER_SUPER_ADMIN_ONLY);
		return MOD_STOP;
	}

	/* Check the 1st argument [nick] is a valid (in use) nick -
	 * This will filter out services/botserv bots.
	 */
	if (!(target = finduser(tobeparted))) {
		moduleNoticeLang(s_OperServ, u, LANG_NO_SUCH_USER, tobeparted);
		return MOD_STOP;
	}

	/* If channel name is too long, reject.
	 *
	 * joinchan is not truncated (which would allow the command to contine) for a reason ;)
	 *
	 * be glad chanmaxtot and actualchanname are declared as double - took me 40 mins to
	 * figure out why it segfaulted on > 62 chan with this code having them def'd as int.
	 */
	chanmaxtot = CHANMAX - 2;
	actualchanname = strlen(partchan);
	if (actualchanname > chanmaxtot) {
		moduleNoticeLang(s_OperServ, u, LANG_CHANNAME_TOO_LONG);
		return MOD_STOP;
	}

	/* Check Parameter 1 is a valid channel name. */
	if (partchan[0] != '#') {
		/* if not - bitch and quit */
		moduleNoticeLang(s_OperServ, u, LANG_CHANNAME_ILLEGAL, partchan);
		return MOD_STOP;
	}

	if (!(ci = cs_findchan(partchan))) {
		if (ci->flags & CI_VERBOTEN) {
			notice_lang(s_OperServ, u, CHAN_X_FORBIDDEN, partchan);
			return MOD_STOP;
		}
	}

	/* Check whether user is allready on channel */
	if (!is_on_chan(ci->c, target)) {
		moduleNoticeLang(s_OperServ, u, LANG_USER_NOT_IN_CHAN, tobeparted);
		return MOD_STOP;
	}

	if (!issilent) {
		notice(s_OperServ, u->nick, "Forcing %s to part %s", tobeparted, partchan);

		/* make some noise to prevent abusers */
		wallops(s_OperServ, "%s used SVSPART to part %s to %s", u->nick, tobeparted, partchan);
		moduleNoticeLang(s_OperServ, u, LANG_FORCE_PARTED, partchan, u->nick);

		/* Force them out */
		// we ll use anope's provided function here...
		anope_cmd_svspart(s_OperServ, target->nick, ci->name);

	} else {
		if (stricmp(issilent,"silent") == 0)  {
			notice(s_OperServ, u->nick, "Forcing %s to part %s", tobeparted, partchan);
			/* make some noise to prevent abusers */
			wallops(s_OperServ, "%s used SVSPART to part %s to %s", u->nick, tobeparted, partchan);

			/* Force them out */
			// we ll use anope's provided function here...
			anope_cmd_svspart(s_OperServ, target->nick, ci->name);

		} else
			moduleNoticeLang(s_OperServ, u, LANG_SVSPART_SYNTAX);
	}
	return MOD_STOP;
}

int valid_ircd(void) {
	if (!stricmp(IRCDModule, "unreal32"))
		return 1;

	if (!stricmp(IRCDModule, "viagra"))
		return 1;

	if (!stricmp(IRCDModule, "ptlink"))
		return 1;

	if (!stricmp(IRCDModule, "ultimate2"))
		return 1;
	return 0;
}


/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_SVSJOIN_INFO */
		"    SVSJOIN     Force-Join a user to a channel",
		/* LANG_SVSJOIN_SYNTAX */
		" Syntax: SVSJOIN Nick #channel [SILENT]",
		/* LANG_SVSJOIN_SYNTAX_EXT */
		" Syntax: SVSJOIN nick #channel [SILENT]\n"
		" \n"
		" Allows \002SuperAdmins\002 to force a user into a channel. The user is\n"
		" informed who used the command and what room they were forced to join to join.\n"
		" \n"
		" If the silent argument is used, the user is NOT informed.\n"
		" \n"
		" Please DO NOT abuse this command. This command is Super-Admin only.",
		/* LANG_SVSPART_INFO */
		"    SVSPART     Force-Part a user from a channel",
		/* LANG_SVSPART_SYNTAX */
		" Syntax: SVSJOIN Nick #channel [SILENT]",
		/* LANG_SVSPART_SYNTAX_EXT */
		" Syntax: SVSPART Nick #channel [SILENT]\n"
		" \n"
		" Allows \2SuperAdmins\2 to force a user out of a channel. The user is\n"
		" informed who used the command and what room they were forced to part.\n"
		" \n"
		" If the SILENT option is used, the user is NOT informed \n"
		" \n"
		" Please DO NOT abuse this command. This command is Super-Admin only.\n",
		/* LANG_NO_SUCH_USER */
		" User [%s] does not exist or is a services client",
		/* LANG_CHANNAME_TOO_LONG */
		" That channel name is too long.",
		/* LANG_CHANNAME_ILLEGAL */
		" Illegal channel name: %s - Please ensure channel names begin with #.",
		/* LANG_FORCE_JOINED */
		" You were forced to join %s by %s \n"
		" If you feel this was done without a valid reason, please report it to another IRC operator",
		/* LANG_FORCE_PARTED */
		" You were forced out of %s by %s \n"
		" If you feel this was done without a valid reason, please report it to another IRC operator",
		/* LANG_USER_ALLREADY_IN_CHAN */
		" The user [%s] is already in the specified channel.",
		/* LANG_USER_NOT_IN_CHAN */
		" The user [%s] is not in the specified channel."
	};

	char *langtable_nl[] = {
		/* LANG_SVSJOIN_INFO */
		"    SVSJOIN     Verplicht een gebruiker een kanaal te joinen",
		/* LANG_SVSJOIN_SYNTAX */
		" Gebruik: SVSJOIN Nick #kanaal [SILENT]",
		/* LANG_SVSJOIN_SYNTAX_EXT */
		" Gebruik: SVSJOIN nick #kanaal [SILENT]\n"
		" \n"
		" Stelt \002SuperAdmins\002 in staat een gebruiker in een kanaal te forceren.\n"
		" De gebruiker wordt geinformeerd over wie het commando heeft gegevenen en in welk \n"
		" kanaal hij werd geforceerd.\n"
		" \n"
		" Als 'silent' (stil) werd opgegeven, wordt de gebruiker NIET geinformeerd.\n"
		" \n"
		" Gelieve dit commando NIET te misbruiken. Dit commando is alleen voor Super-Admins.",
		/* LANG_SVSPART_INFO */
		"    SVSPART     Verplicht een gebruiker een kanaal te verlaten.",
		/* LANG_SVSPART_SYNTAX */
		" Gebruik: SVSJOIN Nick #kanaal [SILENT]",
		/* LANG_SVSPART_SYNTAX_EXT */
		" Gebruik: SVSPART Nick #kanaal [SILENT]\n"
		" \n"
		" Stelt \002SuperAdmins\002 in staat een gebruiker uit een kanaal te forceren.\n"
		" De gebruiker wordt geinformeerd over wie het commando heeft gegevenen en uit welk \n"
		" kanaal hij werd verwijderd.\n"
		" \n"
		" Als 'silent' (stil) werd opgegeven, wordt de gebruiker NIET geinformeerd.\n"
		" \n"
		" Gelieve dit commando NIET te misbruiken. Dit commando is alleen voor Super-Admins.",
		/* LANG_NO_SUCH_USER */
		" De gebruiker [%s] bestaat niet of is een services client",
		/* LANG_CHANNAME_TOO_LONG */
		" De naam van het kanaal is te lang.",
		/* LANG_CHANNAME_ILLEGAL */
		" Illegale naam voor het kanaal: %s - Gelieve ervoor te zorgen dat de naam begint met #.",
		/* LANG_FORCE_JOINED */
		" U werd verplicht het kanaal %s  te joinen door %s \n"
		" Als u vindt dat dit is gebeurd voor een ongeldige reden, gelieve dit bij een andere IRC operator te melden.",
		/* LANG_FORCE_PARTED */
		" U werd verplicht het kanaal %s te verlaten door %s \n"
		" Als u vindt dat dit is gebeurd voor een ongeldige reden, gelieve dit bij een andere IRC operator te melden.",
		/* LANG_USER_ALLREADY_IN_CHAN */
		" De gebruiker [%s] is reeds aanwezig in het opgegeven kanaal.",
		/* LANG_USER_NOT_IN_CHAN */
		" De gebruiker [%s] is niet aanwezig in het opgegeven kanaal."
	};
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* EOF */
