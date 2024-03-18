/***************************************************************************************/
/* Anope Module : unreal_access_levels.c : v1.x                                        */
/* Scott Seufert                                                                       */
/* katsklaw@ircmojo.net                                                                */
/*                                                                                     */
/* Anope (c) 2000-2002 Anope.org                                                       */
/*                                                                                     */
/* This program is free software; you can redistribute it and/or modify it under the   */
/* terms of the GNU General Public License as published by the Free Software           */
/* Foundation; either version 1, or (at your option) any later version.                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful, but WITHOUT ANY    */
/*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A    */
/*  PARTICULAR PURPOSE.  See the GNU General Public License for more details.          */
/*                                                                                     */
/***************************************************************************************/

#include "module.h"
#define AUTHOR "katsklaw"
#define VERSION "1.0.0"

/* User Modes */
#define UMODE_a_unreal32 0x00000001  /* a Services Admin */
#define UMODE_h_unreal32 0x00000002  /* h Available for help (HelpOp) */
#define UMODE_A_unreal32 0x00000040  /* A Server Admin */
#define UMODE_N_unreal32 0x00000080  /* N Network Administrator */
#define UMODE_C_unreal32 0x00000200  /* C Co-Admin */

/*
 * This module more or less emulates a numerical access system for OperServ commands.
 * To use simply add the directives listed in levels.conf to your services.conf
 *
 */

int GlobalAccess;
int StatsAccess;
int StaffAccess;
int OperAccess;
int AdminAccess;
int ModeAccess;
int KickAccess;
int ClearModesAccess;
int AkillAccess;
int SGlineAccess;
int SQlineAccess;
int SZlineAccess;
int ChanListAccess;
int UserListAccess;
int LogonNewsAccess;
int RandomNewsAccess;
int OperNewsAccess;
int SessionAccess;
int ExceptionAccess;
int NoopAccess;
int JupeAccess;
int IgnoreAccess;
int SetAccess;
int ReloadAccess;
int UpdateAccess;
int RestartAccess;
int ModLoadAccess;
int ModUnLoadAccess;
int ModInfoAccess;
int ModListAccess;
int QuitAccess;
int ShutDownAccess;
int DefConAccess;
int ChanKillAccess;

int m_do_global(User * u);
int m_do_stats(User * u);
int m_do_staff(User * u);
int m_do_mode(User * u);
int m_do_kick(User * u);
int m_do_clearmodes(User * u);
int m_do_akill(User * u);
int m_do_sgline(User * u);
int m_do_sqline(User * u);
int m_do_szline(User * u);
int m_do_chanlist(User * u);
int m_do_userlist(User * u);
int m_do_logonnews(User * u);
int m_do_randomnews(User * u);
int m_do_opernews(User * u);
int m_do_reload(User * u);
int m_do_restart(User * u);
int m_do_modload(User * u);
int m_do_modunload(User * u);
int m_do_modinfo(User * u);
int m_do_modlist(User * u);
int m_do_quit(User * u);
int m_do_shutdown(User * u);
int m_do_defcon(User * u);
int m_do_chankill(User * u);
int m_do_oper(User * u);
int m_do_admin(User * u);
int m_do_session(User * u);
int m_do_exception(User * u);
int m_do_noop(User * u);
int m_do_jupe(User * u);
int m_do_ignore(User * u);
int m_do_set(User * u);
int m_do_update(User * u);

int load_config(void);
int do_reload(int argc, char **argv);
int m_levels(User * u);

void Module_Help_OPERSERV_LEVELS(User *u);
int Module_Help_OPERSERV_LEVELS_FULL(User *u);

/***************************************************************************************/

int AnopeInit(int argc, char **argv) 
{
	EvtHook *hook = NULL;
	int status;

 	Command *c;
	c = createCommand("levels",m_levels, is_services_oper,-1,-1,-1,-1,-1);
		moduleAddCommand(OPERSERV,c,MOD_HEAD);
		moduleAddHelp(c, Module_Help_OPERSERV_LEVELS_FULL);
		moduleSetOperHelp(Module_Help_OPERSERV_LEVELS);


	c = createCommand("global",m_do_global,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("stats",m_do_stats,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("staff",m_do_staff,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("mode",m_do_mode,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("kick",m_do_kick,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("clearmodes",m_do_clearmodes,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("akill",m_do_akill,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("sgline",m_do_sgline,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("sqline",m_do_sqline,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("szline",m_do_szline,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("chanlist",m_do_chanlist,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("userlist",m_do_userlist,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("logonnews",m_do_logonnews,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("randomnews",m_do_randomnews,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("opernews",m_do_opernews,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("reload",m_do_reload,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("restart",m_do_restart,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("modload",m_do_modload,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("modunload",m_do_modunload,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("modinfo",m_do_modinfo,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("modlist",m_do_modlist,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("quit",m_do_quit,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("shutdown",m_do_shutdown,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("defcon",m_do_defcon,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("chankill",m_do_chankill,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("oper",m_do_oper,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("admin",m_do_admin,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("session",m_do_session,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("exception",m_do_exception,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("noop",m_do_noop,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("jupe",m_do_jupe,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("ignore",m_do_ignore,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("set",m_do_set,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("update",m_do_update,NULL,-1,-1,-1,-1,-1);
        moduleAddCommand(OPERSERV,c,MOD_HEAD);

	hook = createEventHook(EVENT_RELOAD, do_reload);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("Unable to hook to EVENT_RELOAD. Unloading module [%d]", status);
		return MOD_STOP;
	}
	/* works on Unreal3.2 only! */
	if (stricmp(IRCDModule, "unreal32")) {
		alog("You are not using Unreal3.2, this module will NOT work!");
		return MOD_STOP;
	}
	/* Found os_raw, sorry */
	if (findModule("os_raw")) {
		alog("I found os_raw loaded!, Sorry it didn't work out.");
        return MOD_STOP;
	}

	load_config();

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
	return MOD_CONT;
}
/* Unreal oper levels */
int is_helper(User *u) {
    if (!u) 
    return 0; 
    if (!stricmp(IRCDModule, "unreal32")) 
    return u->mode & UMODE_h_unreal32;
    return 0; 
}

int is_svsadmin(User *u) {
    if (!u) 
    return 0; 
    if (!stricmp(IRCDModule, "unreal32")) 
    return u->mode & UMODE_a_unreal32;
    return 0; 
}

int is_servadmin(User *u) {
    if (!u) 
    return 0; 
    if (!stricmp(IRCDModule, "unreal32")) 
    return u->mode & UMODE_A_unreal32;
    return 0; 
}

int is_netadmin(User *u) {
    if (!u) 
    return 0; 
    if (!stricmp(IRCDModule, "unreal32")) 
    return u->mode & UMODE_N_unreal32;
    return 0; 
}

/***************************************************************************************/

void AnopeFini(void) 
{
  alog("Unloading unreal_access_levels");
}

/***************************************************************************************/

int do_reload(int argc, char **argv)
{
    load_config();
    return MOD_CONT;
}

/***************************************************************************************/

void Module_Help_OPERSERV_LEVELS(User *u)
{
   notice(s_OperServ,u->nick, "    LEVELS   Lists OperServ Access Levels.");
}

/***************************************************************************************/

int Module_Help_OPERSERV_LEVELS_FULL(User *u)
{
	notice(s_OperServ, u->nick, "==================================================================");
        notice(s_OperServ, u->nick, " Syntax: LEVELS");
        notice(s_OperServ, u->nick, "   ");
        notice(s_OperServ, u->nick, " Allows Opers to see various %s Access LEVELS",s_OperServ);
	notice(s_OperServ, u->nick, "==================================================================");

        return MOD_CONT;
}

/***************************************************************************************/

int m_levels (User * u)
{
	char *my_GlobalAccess = NULL;
	char *my_StatsAccess = NULL;
	char *my_StaffAccess = NULL;
	char *my_OperAccess = NULL;
	char *my_AdminAccess = NULL;
	char *my_ModeAccess = NULL;
	char *my_KickAccess = NULL;
	char *my_ClearModesAccess = NULL;
	char *my_AkillAccess = NULL;
	char *my_SGlineAccess = NULL;
	char *my_SQlineAccess = NULL;
	char *my_SZlineAccess = NULL;
	char *my_ChanListAccess = NULL;
	char *my_UserListAccess = NULL;
	char *my_LogonNewsAccess = NULL;
	char *my_RandomNewsAccess = NULL;
	char *my_OperNewsAccess = NULL;
	char *my_SessionAccess = NULL;
	char *my_ExceptionAccess = NULL;
	char *my_NoopAccess = NULL;
	char *my_JupeAccess = NULL;
	char *my_IgnoreAccess = NULL;
	char *my_SetAccess = NULL;
	char *my_ReloadAccess = NULL;
	char *my_UpdateAccess = NULL;
	char *my_RestartAccess = NULL;
	char *my_ModLoadAccess = NULL;
	char *my_ModUnLoadAccess = NULL;
	char *my_ModInfoAccess = NULL;
	char *my_ModListAccess = NULL;
	char *my_QuitAccess = NULL;
	char *my_ShutDownAccess = NULL;
	char *my_DefConAccess = NULL;
	char *my_ChanKillAccess = NULL;

	Directive confvalues[] = {
       { "StatsAccess", { { PARAM_INT, PARAM_RELOAD, &my_StatsAccess } } }
   };
	Directive confvalues1[] = {
       { "StaffAccess", { { PARAM_INT, PARAM_RELOAD, &my_StaffAccess } } }
   };
	Directive confvalues2[] = {
       { "OperAccess", { { PARAM_INT, PARAM_RELOAD, &my_OperAccess } } }
   };
	Directive confvalues3[] = {
       { "AdminAccess", { { PARAM_INT, PARAM_RELOAD, &my_AdminAccess } } }
   };
	Directive confvalues4[] = {
       { "ModeAccess", { { PARAM_INT, PARAM_RELOAD, &my_ModeAccess } } }
   };
	Directive confvalues5[] = {
       { "KickAccess", { { PARAM_INT, PARAM_RELOAD, &my_KickAccess } } }
   };
	Directive confvalues6[] = {
       { "ClearModesAccess", { { PARAM_INT, PARAM_RELOAD, &my_ClearModesAccess } } }
   };
	Directive confvalues7[] = {
       { "AkillAccess", { { PARAM_INT, PARAM_RELOAD, &my_AkillAccess } } }
   };
	Directive confvalues9[] = {
       { "SGlineAccess", { { PARAM_INT, PARAM_RELOAD, &my_SGlineAccess } } }
   };
	Directive confvalues10[] = {
       { "SQlineAccess", { { PARAM_INT, PARAM_RELOAD, &my_SQlineAccess } } }
   };
	Directive confvalues11[] = {
       { "SZlineAccess", { { PARAM_INT, PARAM_RELOAD, &my_SZlineAccess } } }
   };
	Directive confvalues12[] = {
       { "ChanListAccess", { { PARAM_INT, PARAM_RELOAD, &my_ChanListAccess } } }
   };
	Directive confvalues13[] = {
       { "UserListAccess", { { PARAM_INT, PARAM_RELOAD, &my_UserListAccess } } }
   };
	Directive confvalues14[] = {
       { "LogonNewsAccess", { { PARAM_INT, PARAM_RELOAD, &my_LogonNewsAccess } } }
   };
	Directive confvalues15[] = {
       { "RandomNewsAccess", { { PARAM_INT, PARAM_RELOAD, &my_RandomNewsAccess } } }
   };
	Directive confvalues16[] = {
       { "OperNewsAccess", { { PARAM_INT, PARAM_RELOAD, &my_OperNewsAccess } } }
   };
	Directive confvalues17[] = {
       { "SessionAccess", { { PARAM_INT, PARAM_RELOAD, &my_SessionAccess } } }
   };
	Directive confvalues18[] = {
       { "ExceptionAccess", { { PARAM_INT, PARAM_RELOAD, &my_ExceptionAccess } } }
   };
	Directive confvalues19[] = {
       { "NoopAccess", { { PARAM_INT, PARAM_RELOAD, &my_NoopAccess } } }
   };
	Directive confvalues20[] = {
       { "JupeAccess", { { PARAM_INT, PARAM_RELOAD, &my_JupeAccess } } }
   };
	Directive confvalues21[] = {
       { "IgnoreAccess", { { PARAM_INT, PARAM_RELOAD, &my_IgnoreAccess } } }
   };
	Directive confvalues22[] = {
       { "SetAccess", { { PARAM_INT, PARAM_RELOAD, &my_SetAccess } } }
   };
	Directive confvalues23[] = {
       { "ReloadAccess", { { PARAM_INT, PARAM_RELOAD, &my_ReloadAccess } } }
   };
	Directive confvalues24[] = {
       { "UpdateAccess", { { PARAM_INT, PARAM_RELOAD, &my_UpdateAccess } } }
   };
	Directive confvalues25[] = {
       { "RestartAccess", { { PARAM_INT, PARAM_RELOAD, &my_RestartAccess } } }
   };
	Directive confvalues26[] = {
       { "ModLoadAccess", { { PARAM_INT, PARAM_RELOAD, &my_ModLoadAccess } } }
   };
	Directive confvalues27[] = {
       { "ModUnLoadAccess", { { PARAM_INT, PARAM_RELOAD, &my_ModUnLoadAccess } } }
   };
	Directive confvalues28[] = {
       { "ModInfoAccess", { { PARAM_INT, PARAM_RELOAD, &my_ModInfoAccess } } }
   };
	Directive confvalues29[] = {
       { "ModListAccess", { { PARAM_INT, PARAM_RELOAD, &my_ModListAccess } } }
   };
	Directive confvalues30[] = {
       { "QuitAccess", { { PARAM_INT, PARAM_RELOAD, &my_QuitAccess } } }
   };
	Directive confvalues31[] = {
       { "ShutDownAccess", { { PARAM_INT, PARAM_RELOAD, &my_ShutDownAccess } } }
   };
	Directive confvalues32[] = {
       { "DefConAccess", { { PARAM_INT, PARAM_RELOAD, &my_DefConAccess } } }
   };
	Directive confvalues33[] = {
       { "ChanKillAccess", { { PARAM_INT, PARAM_RELOAD, &my_ChanKillAccess } } }
   };
	Directive confvalues34[] = {
       { "GlobalAccess", { { PARAM_INT, PARAM_RELOAD, &my_GlobalAccess } } }
   };


	moduleGetConfigDirective(confvalues);
	moduleGetConfigDirective(confvalues1);
	moduleGetConfigDirective(confvalues2);
	moduleGetConfigDirective(confvalues3);
	moduleGetConfigDirective(confvalues4);
	moduleGetConfigDirective(confvalues5);
	moduleGetConfigDirective(confvalues6);
	moduleGetConfigDirective(confvalues7);
	moduleGetConfigDirective(confvalues9);
	moduleGetConfigDirective(confvalues10);
	moduleGetConfigDirective(confvalues11);
	moduleGetConfigDirective(confvalues12);
	moduleGetConfigDirective(confvalues13);
	moduleGetConfigDirective(confvalues14);
	moduleGetConfigDirective(confvalues15);
	moduleGetConfigDirective(confvalues16);
	moduleGetConfigDirective(confvalues17);
	moduleGetConfigDirective(confvalues18);
	moduleGetConfigDirective(confvalues19);
	moduleGetConfigDirective(confvalues20);
	moduleGetConfigDirective(confvalues21);
	moduleGetConfigDirective(confvalues22);
	moduleGetConfigDirective(confvalues23);
	moduleGetConfigDirective(confvalues24);
	moduleGetConfigDirective(confvalues25);
	moduleGetConfigDirective(confvalues26);
	moduleGetConfigDirective(confvalues27);
	moduleGetConfigDirective(confvalues28);
	moduleGetConfigDirective(confvalues29);
	moduleGetConfigDirective(confvalues30);
	moduleGetConfigDirective(confvalues31);
	moduleGetConfigDirective(confvalues32);
	moduleGetConfigDirective(confvalues33);
	moduleGetConfigDirective(confvalues34);

	/* convert numbers to names 
	 * Lets do levels:
	 * 1 = helpop
	 * 2 = ircop 
	 * 3 = svsadmin
	 * 4 = servadmin
	 * 5 = netadmin
	 * 6 = SO
	 * 7 = SA
	 * 8 = SRA
	 * 9 = disabled
	 */
	if (GlobalAccess == 1) {
		my_GlobalAccess = sstrdup("HelpOp");
	} else if (GlobalAccess == 2) {
		my_GlobalAccess = sstrdup("IRCop");
	} else if (GlobalAccess == 3) {
		my_GlobalAccess = sstrdup("SVSAdmin");
	} else if (GlobalAccess == 4) {
		my_GlobalAccess = sstrdup("ServAdmin");
	} else if (GlobalAccess == 5) {
		my_GlobalAccess = sstrdup("NetAdmin");
	} else if (GlobalAccess == 6) {
		my_GlobalAccess = sstrdup("CSop");
	} else if (GlobalAccess == 7) {
		my_GlobalAccess = sstrdup("CSAdmin");
	} else if (GlobalAccess == 8) {
		my_GlobalAccess = sstrdup("Root");
	} else if (GlobalAccess == 9) {
		my_GlobalAccess = sstrdup("Disabled");
	}
	if (StatsAccess == 1) {
		my_StatsAccess = sstrdup("HelpOp");
	} else if (StatsAccess == 2) {
		my_StatsAccess = sstrdup("IRCop");
	} else if (StatsAccess == 3) {
		my_StatsAccess = sstrdup("SVSAdmin");
	} else if (StatsAccess == 4) {
		my_StatsAccess = sstrdup("ServAdmin");
	} else if (StatsAccess == 5) {
		my_StatsAccess = sstrdup("NetAdmin");
	} else if (StatsAccess == 6) {
		my_StatsAccess = sstrdup("CSop");
	} else if (StatsAccess == 7) {
		my_StatsAccess = sstrdup("CSAdmin");
	} else if (StatsAccess == 8) {
		my_StatsAccess = sstrdup("Root");
	} else if (StatsAccess == 9) {
		my_StatsAccess = sstrdup("Disabled");
	}
	if (OperAccess == 1) {
		my_OperAccess = sstrdup("HelpOp");
	} else if (OperAccess == 2) {
		my_OperAccess = sstrdup("IRCop");
	} else if (OperAccess == 3) {
		my_OperAccess = sstrdup("SVSAdmin");
	} else if (OperAccess == 4) {
		my_OperAccess = sstrdup("ServAdmin");
	} else if (OperAccess == 5) {
		my_OperAccess = sstrdup("NetAdmin");
	} else if (OperAccess == 6) {
		my_OperAccess = sstrdup("CSop");
	} else if (OperAccess == 7) {
		my_OperAccess = sstrdup("CSAdmin");
	} else if (OperAccess == 8) {
		my_OperAccess = sstrdup("Root");
	} else if (OperAccess == 9) {
		my_OperAccess = sstrdup("Disabled");
	}
	if (AdminAccess == 1) {
		my_AdminAccess = sstrdup("HelpOp");
	} else if (AdminAccess == 2) {
		my_AdminAccess = sstrdup("IRCop");
	} else if (AdminAccess == 3) {
		my_AdminAccess = sstrdup("SVSAdmin");
	} else if (AdminAccess == 4) {
		my_AdminAccess = sstrdup("ServAdmin");
	} else if (AdminAccess == 5) {
		my_AdminAccess = sstrdup("NetAdmin");
	} else if (AdminAccess == 6) {
		my_AdminAccess = sstrdup("CSop");
	} else if (AdminAccess == 7) {
		my_AdminAccess = sstrdup("CSAdmin");
	} else if (AdminAccess == 8) {
		my_AdminAccess = sstrdup("Root");
	} else if (AdminAccess == 9) {
		my_AdminAccess = sstrdup("Disabled");
	}
	if (ModeAccess == 1) {
		my_ModeAccess = sstrdup("HelpOp");
	} else if (ModeAccess == 2) {
		my_ModeAccess = sstrdup("IRCop");
	} else if (ModeAccess == 3) {
		my_ModeAccess = sstrdup("SVSAdmin");
	} else if (ModeAccess == 4) {
		my_ModeAccess = sstrdup("ServAdmin");
	} else if (ModeAccess == 5) {
		my_ModeAccess = sstrdup("NetAdmin");
	} else if (ModeAccess == 6) {
		my_ModeAccess = sstrdup("CSop");
	} else if (ModeAccess == 7) {
		my_ModeAccess = sstrdup("CSAdmin");
	} else if (ModeAccess == 8) {
		my_ModeAccess = sstrdup("Root");
	} else if (ModeAccess == 9) {
		my_ModeAccess = sstrdup("Disabled");
	}
	if (KickAccess == 1) {
		my_KickAccess = sstrdup("HelpOp");
	} else if (KickAccess == 2) {
		my_KickAccess = sstrdup("IRCop");
	} else if (KickAccess == 3) {
		my_KickAccess = sstrdup("SVSAdmin");
	} else if (KickAccess == 4) {
		my_KickAccess = sstrdup("ServAdmin");
	} else if (KickAccess == 5) {
		my_KickAccess = sstrdup("NetAdmin");
	} else if (KickAccess == 6) {
		my_KickAccess = sstrdup("CSop");
	} else if (KickAccess == 7) {
		my_KickAccess = sstrdup("CSAdmin");
	} else if (KickAccess == 8) {
		my_KickAccess = sstrdup("Root");
	} else if (KickAccess == 9) {
		my_KickAccess = sstrdup("Disabled");
	}
	if (ClearModesAccess == 1) {
		my_ClearModesAccess = sstrdup("HelpOp");
	} else if (ClearModesAccess == 2) {
		my_ClearModesAccess = sstrdup("IRCop");
	} else if (ClearModesAccess == 3) {
		my_ClearModesAccess = sstrdup("SVSAdmin");
	} else if (ClearModesAccess == 4) {
		my_ClearModesAccess = sstrdup("ServAdmin");
	} else if (ClearModesAccess == 5) {
		my_ClearModesAccess = sstrdup("NetAdmin");
	} else if (ClearModesAccess == 6) {
		my_ClearModesAccess = sstrdup("CSop");
	} else if (ClearModesAccess == 7) {
		my_ClearModesAccess = sstrdup("CSAdmin");
	} else if (ClearModesAccess == 8) {
		my_ClearModesAccess = sstrdup("Root");
	} else if (ClearModesAccess == 9) {
		my_ClearModesAccess = sstrdup("Disabled");
	}
	if (AkillAccess == 1) {
		my_AkillAccess = sstrdup("HelpOp");
	} else if (AkillAccess == 2) {
		my_AkillAccess = sstrdup("IRCop");
	} else if (AkillAccess == 3) {
		my_AkillAccess = sstrdup("SVSAdmin");
	} else if (AkillAccess == 4) {
		my_AkillAccess = sstrdup("ServAdmin");
	} else if (AkillAccess == 5) {
		my_AkillAccess = sstrdup("NetAdmin");
	} else if (AkillAccess == 6) {
		my_AkillAccess = sstrdup("CSop");
	} else if (AkillAccess == 7) {
		my_AkillAccess = sstrdup("CSAdmin");
	} else if (AkillAccess == 8) {
		my_AkillAccess = sstrdup("Root");
	} else if (AkillAccess == 9) {
		my_AkillAccess = sstrdup("Disabled");
	}
	if (SGlineAccess == 1) {
		my_SGlineAccess = sstrdup("HelpOp");
	} else if (SGlineAccess == 2) {
		my_SGlineAccess = sstrdup("IRCop");
	} else if (SGlineAccess == 3) {
		my_SGlineAccess = sstrdup("SVSAdmin");
	} else if (SGlineAccess == 4) {
		my_SGlineAccess = sstrdup("ServAdmin");
	} else if (SGlineAccess == 5) {
		my_SGlineAccess = sstrdup("NetAdmin");
	} else if (SGlineAccess == 6) {
		my_SGlineAccess = sstrdup("CSop");
	} else if (SGlineAccess == 7) {
		my_SGlineAccess = sstrdup("CSAdmin");
	} else if (SGlineAccess == 8) {
		my_SGlineAccess = sstrdup("Root");
	} else if (SGlineAccess == 9) {
		my_SGlineAccess = sstrdup("Disabled");
	}
	if (SQlineAccess == 1) {
		my_SQlineAccess = sstrdup("HelpOp");
	} else if (SQlineAccess == 2) {
		my_SQlineAccess = sstrdup("IRCop");
	} else if (SQlineAccess == 3) {
		my_SQlineAccess = sstrdup("SVSAdmin");
	} else if (SQlineAccess == 4) {
		my_SQlineAccess = sstrdup("ServAdmin");
	} else if (SQlineAccess == 5) {
		my_SQlineAccess = sstrdup("NetAdmin");
	} else if (SQlineAccess == 6) {
		my_SQlineAccess = sstrdup("CSop");
	} else if (SQlineAccess == 7) {
		my_SQlineAccess = sstrdup("CSAdmin");
	} else if (SQlineAccess == 8) {
		my_SQlineAccess = sstrdup("Root");
	} else if (SQlineAccess == 9) {
		my_SQlineAccess = sstrdup("Disabled");
	}
	if (SZlineAccess == 1) {
		my_SZlineAccess = sstrdup("HelpOp");
	} else if (SZlineAccess == 2) {
		my_SZlineAccess = sstrdup("IRCop");
	} else if (SZlineAccess == 3) {
		my_SZlineAccess = sstrdup("SVSAdmin");
	} else if (SZlineAccess == 4) {
		my_SZlineAccess = sstrdup("ServAdmin");
	} else if (SZlineAccess == 5) {
		my_SZlineAccess = sstrdup("NetAdmin");
	} else if (SZlineAccess == 6) {
		my_SZlineAccess = sstrdup("CSop");
	} else if (SZlineAccess == 7) {
		my_SZlineAccess = sstrdup("CSAdmin");
	} else if (SZlineAccess == 8) {
		my_SZlineAccess = sstrdup("Root");
	} else if (SZlineAccess == 9) {
		my_SZlineAccess = sstrdup("Disabled");
	}
	if (ChanListAccess == 1) {
		my_ChanListAccess = sstrdup("HelpOp");
	} else if (ChanListAccess == 2) {
		my_ChanListAccess = sstrdup("IRCop");
	} else if (ChanListAccess == 3) {
		my_ChanListAccess = sstrdup("SVSAdmin");
	} else if (ChanListAccess == 4) {
		my_ChanListAccess = sstrdup("ServAdmin");
	} else if (ChanListAccess == 5) {
		my_ChanListAccess = sstrdup("NetAdmin");
	} else if (ChanListAccess == 6) {
		my_ChanListAccess = sstrdup("CSop");
	} else if (ChanListAccess == 7) {
		my_ChanListAccess = sstrdup("CSAdmin");
	} else if (ChanListAccess == 8) {
		my_ChanListAccess = sstrdup("Root");
	} else if (ChanListAccess == 9) {
		my_ChanListAccess = sstrdup("Disabled");
	}
	if (UserListAccess == 1) {
		my_UserListAccess = sstrdup("HelpOp");
	} else if (UserListAccess == 2) {
		my_UserListAccess = sstrdup("IRCop");
	} else if (UserListAccess == 3) {
		my_UserListAccess = sstrdup("SVSAdmin");
	} else if (UserListAccess == 4) {
		my_UserListAccess = sstrdup("ServAdmin");
	} else if (UserListAccess == 5) {
		my_UserListAccess = sstrdup("NetAdmin");
	} else if (UserListAccess == 6) {
		my_UserListAccess = sstrdup("CSop");
	} else if (UserListAccess == 7) {
		my_UserListAccess = sstrdup("CSAdmin");
	} else if (UserListAccess == 8) {
		my_UserListAccess = sstrdup("Root");
	} else if (UserListAccess == 9) {
		my_UserListAccess = sstrdup("Disabled");
	}
	if (LogonNewsAccess == 1) {
		my_LogonNewsAccess = sstrdup("HelpOp");
	} else if (LogonNewsAccess == 2) {
		my_LogonNewsAccess = sstrdup("IRCop");
	} else if (LogonNewsAccess == 3) {
		my_LogonNewsAccess = sstrdup("SVSAdmin");
	} else if (LogonNewsAccess == 4) {
		my_LogonNewsAccess = sstrdup("ServAdmin");
	} else if (LogonNewsAccess == 5) {
		my_LogonNewsAccess = sstrdup("NetAdmin");
	} else if (LogonNewsAccess == 6) {
		my_LogonNewsAccess = sstrdup("CSop");
	} else if (LogonNewsAccess == 7) {
		my_LogonNewsAccess = sstrdup("CSAdmin");
	} else if (LogonNewsAccess == 8) {
		my_LogonNewsAccess = sstrdup("Root");
	} else if (LogonNewsAccess == 9) {
		my_LogonNewsAccess = sstrdup("Disabled");
	}
	if (RandomNewsAccess == 1) {
		my_RandomNewsAccess = sstrdup("HelpOp");
	} else if (RandomNewsAccess == 2) {
		my_RandomNewsAccess = sstrdup("IRCop");
	} else if (RandomNewsAccess == 3) {
		my_RandomNewsAccess = sstrdup("SVSAdmin");
	} else if (RandomNewsAccess == 4) {
		my_RandomNewsAccess = sstrdup("ServAdmin");
	} else if (RandomNewsAccess == 5) {
		my_RandomNewsAccess = sstrdup("NetAdmin");
	} else if (RandomNewsAccess == 6) {
		my_RandomNewsAccess = sstrdup("CSop");
	} else if (RandomNewsAccess == 7) {
		my_RandomNewsAccess = sstrdup("CSAdmin");
	} else if (RandomNewsAccess == 8) {
		my_RandomNewsAccess = sstrdup("Root");
	} else if (RandomNewsAccess == 9) {
		my_RandomNewsAccess = sstrdup("Disabled");
	}
	if (OperNewsAccess == 1) {
		my_OperNewsAccess = sstrdup("HelpOp");
	} else if (OperNewsAccess == 2) {
		my_OperNewsAccess = sstrdup("IRCop");
	} else if (OperNewsAccess == 3) {
		my_OperNewsAccess = sstrdup("SVSAdmin");
	} else if (OperNewsAccess == 4) {
		my_OperNewsAccess = sstrdup("ServAdmin");
	} else if (OperNewsAccess == 5) {
		my_OperNewsAccess = sstrdup("NetAdmin");
	} else if (OperNewsAccess == 6) {
		my_OperNewsAccess = sstrdup("CSop");
	} else if (OperNewsAccess == 7) {
		my_OperNewsAccess = sstrdup("CSAdmin");
	} else if (OperNewsAccess == 8) {
		my_OperNewsAccess = sstrdup("Root");
	} else if (OperNewsAccess == 9) {
		my_OperNewsAccess = sstrdup("Disabled");
	}
	if (SessionAccess == 1) {
		my_SessionAccess = sstrdup("HelpOp");
	} else if (SessionAccess == 2) {
		my_SessionAccess = sstrdup("IRCop");
	} else if (SessionAccess == 3) {
		my_SessionAccess = sstrdup("SVSAdmin");
	} else if (SessionAccess == 4) {
		my_SessionAccess = sstrdup("ServAdmin");
	} else if (SessionAccess == 5) {
		my_SessionAccess = sstrdup("NetAdmin");
	} else if (SessionAccess == 6) {
		my_SessionAccess = sstrdup("CSop");
	} else if (SessionAccess == 7) {
		my_SessionAccess = sstrdup("CSAdmin");
	} else if (SessionAccess == 8) {
		my_SessionAccess = sstrdup("Root");
	} else if (SessionAccess == 9) {
		my_SessionAccess = sstrdup("Disabled");
	}
	if (ExceptionAccess == 1) {
		my_ExceptionAccess = sstrdup("HelpOp");
	} else if (ExceptionAccess == 2) {
		my_ExceptionAccess = sstrdup("IRCop");
	} else if (ExceptionAccess == 3) {
		my_ExceptionAccess = sstrdup("SVSAdmin");
	} else if (ExceptionAccess == 4) {
		my_ExceptionAccess = sstrdup("ServAdmin");
	} else if (ExceptionAccess == 5) {
		my_ExceptionAccess = sstrdup("NetAdmin");
	} else if (ExceptionAccess == 6) {
		my_ExceptionAccess = sstrdup("CSop");
	} else if (ExceptionAccess == 7) {
		my_ExceptionAccess = sstrdup("CSAdmin");
	} else if (ExceptionAccess == 8) {
		my_ExceptionAccess = sstrdup("Root");
	} else if (ExceptionAccess == 9) {
		my_ExceptionAccess = sstrdup("Disabled");
	}
	if (NoopAccess == 1) {
		my_NoopAccess = sstrdup("HelpOp");
	} else if (NoopAccess == 2) {
		my_NoopAccess = sstrdup("IRCop");
	} else if (NoopAccess == 3) {
		my_NoopAccess = sstrdup("SVSAdmin");
	} else if (NoopAccess == 4) {
		my_NoopAccess = sstrdup("ServAdmin");
	} else if (NoopAccess == 5) {
		my_NoopAccess = sstrdup("NetAdmin");
	} else if (NoopAccess == 6) {
		my_NoopAccess = sstrdup("CSop");
	} else if (NoopAccess == 7) {
		my_NoopAccess = sstrdup("CSAdmin");
	} else if (NoopAccess == 8) {
		my_NoopAccess = sstrdup("Root");
	} else if (NoopAccess == 9) {
		my_NoopAccess = sstrdup("Disabled");
	}
	if (JupeAccess == 1) {
		my_JupeAccess = sstrdup("HelpOp");
	} else if (JupeAccess == 2) {
		my_JupeAccess = sstrdup("IRCop");
	} else if (JupeAccess == 3) {
		my_JupeAccess = sstrdup("SVSAdmin");
	} else if (JupeAccess == 4) {
		my_JupeAccess = sstrdup("ServAdmin");
	} else if (JupeAccess == 5) {
		my_JupeAccess = sstrdup("NetAdmin");
	} else if (JupeAccess == 6) {
		my_JupeAccess = sstrdup("CSop");
	} else if (JupeAccess == 7) {
		my_JupeAccess = sstrdup("CSAdmin");
	} else if (JupeAccess == 8) {
		my_JupeAccess = sstrdup("Root");
	} else if (JupeAccess == 9) {
		my_JupeAccess = sstrdup("Disabled");
	}
	if (IgnoreAccess == 1) {
		my_IgnoreAccess = sstrdup("HelpOp");
	} else if (IgnoreAccess == 2) {
		my_IgnoreAccess = sstrdup("IRCop");
	} else if (IgnoreAccess == 3) {
		my_IgnoreAccess = sstrdup("SVSAdmin");
	} else if (IgnoreAccess == 4) {
		my_IgnoreAccess = sstrdup("ServAdmin");
	} else if (IgnoreAccess == 5) {
		my_IgnoreAccess = sstrdup("NetAdmin");
	} else if (IgnoreAccess == 6) {
		my_IgnoreAccess = sstrdup("CSop");
	} else if (IgnoreAccess == 7) {
		my_IgnoreAccess = sstrdup("CSAdmin");
	} else if (IgnoreAccess == 8) {
		my_IgnoreAccess = sstrdup("Root");
	} else if (IgnoreAccess == 9) {
		my_IgnoreAccess = sstrdup("Disabled");
	}
	if (SetAccess == 1) {
		my_SetAccess = sstrdup("HelpOp");
	} else if (SetAccess == 2) {
		my_SetAccess = sstrdup("IRCop");
	} else if (SetAccess == 3) {
		my_SetAccess = sstrdup("SVSAdmin");
	} else if (SetAccess == 4) {
		my_SetAccess = sstrdup("ServAdmin");
	} else if (SetAccess == 5) {
		my_SetAccess = sstrdup("NetAdmin");
	} else if (SetAccess == 6) {
		my_SetAccess = sstrdup("CSop");
	} else if (SetAccess == 7) {
		my_SetAccess = sstrdup("CSAdmin");
	} else if (SetAccess == 8) {
		my_SetAccess = sstrdup("Root");
	} else if (SetAccess == 9) {
		my_SetAccess = sstrdup("Disabled");
	}
	if (ReloadAccess == 1) {
		my_ReloadAccess = sstrdup("HelpOp");
	} else if (ReloadAccess == 2) {
		my_ReloadAccess = sstrdup("IRCop");
	} else if (ReloadAccess == 3) {
		my_ReloadAccess = sstrdup("SVSAdmin");
	} else if (ReloadAccess == 4) {
		my_ReloadAccess = sstrdup("ServAdmin");
	} else if (ReloadAccess == 5) {
		my_ReloadAccess = sstrdup("NetAdmin");
	} else if (ReloadAccess == 6) {
		my_ReloadAccess = sstrdup("CSop");
	} else if (ReloadAccess == 7) {
		my_ReloadAccess = sstrdup("CSAdmin");
	} else if (ReloadAccess == 8) {
		my_ReloadAccess = sstrdup("Root");
	} else if (ReloadAccess == 9) {
		my_ReloadAccess = sstrdup("Disabled");
	}
	if (UpdateAccess == 1) {
		my_UpdateAccess = sstrdup("HelpOp");
	} else if (UpdateAccess == 2) {
		my_UpdateAccess = sstrdup("IRCop");
	} else if (UpdateAccess == 3) {
		my_UpdateAccess = sstrdup("SVSAdmin");
	} else if (UpdateAccess == 4) {
		my_UpdateAccess = sstrdup("ServAdmin");
	} else if (UpdateAccess == 5) {
		my_UpdateAccess = sstrdup("NetAdmin");
	} else if (UpdateAccess == 6) {
		my_UpdateAccess = sstrdup("CSop");
	} else if (UpdateAccess == 7) {
		my_UpdateAccess = sstrdup("CSAdmin");
	} else if (UpdateAccess == 8) {
		my_UpdateAccess = sstrdup("Root");
	} else if (UpdateAccess == 9) {
		my_UpdateAccess = sstrdup("Disabled");
	}
	if (RestartAccess == 1) {
		my_RestartAccess = sstrdup("HelpOp");
	} else if (RestartAccess == 2) {
		my_RestartAccess = sstrdup("IRCop");
	} else if (RestartAccess == 3) {
		my_RestartAccess = sstrdup("SVSAdmin");
	} else if (RestartAccess == 4) {
		my_RestartAccess = sstrdup("ServAdmin");
	} else if (RestartAccess == 5) {
		my_RestartAccess = sstrdup("NetAdmin");
	} else if (RestartAccess == 6) {
		my_RestartAccess = sstrdup("CSop");
	} else if (RestartAccess == 7) {
		my_RestartAccess = sstrdup("CSAdmin");
	} else if (RestartAccess == 8) {
		my_RestartAccess = sstrdup("Root");
	} else if (RestartAccess == 9) {
		my_RestartAccess = sstrdup("Disabled");
	}
	if (ModLoadAccess == 1) {
		my_ModLoadAccess = sstrdup("HelpOp");
	} else if (ModLoadAccess == 2) {
		my_ModLoadAccess = sstrdup("IRCop");
	} else if (ModLoadAccess == 3) {
		my_ModLoadAccess = sstrdup("SVSAdmin");
	} else if (ModLoadAccess == 4) {
		my_ModLoadAccess = sstrdup("ServAdmin");
	} else if (ModLoadAccess == 5) {
		my_ModLoadAccess = sstrdup("NetAdmin");
	} else if (ModLoadAccess == 6) {
		my_ModLoadAccess = sstrdup("CSop");
	} else if (ModLoadAccess == 7) {
		my_ModLoadAccess = sstrdup("CSAdmin");
	} else if (ModLoadAccess == 8) {
		my_ModLoadAccess = sstrdup("Root");
	} else if (ModLoadAccess == 9) {
		my_ModLoadAccess = sstrdup("Disabled");
	}
	if (ModUnLoadAccess == 1) {
		my_ModUnLoadAccess = sstrdup("HelpOp");
	} else if (ModUnLoadAccess == 2) {
		my_ModUnLoadAccess = sstrdup("IRCop");
	} else if (ModUnLoadAccess == 3) {
		my_ModUnLoadAccess = sstrdup("SVSAdmin");
	} else if (ModUnLoadAccess == 4) {
		my_ModUnLoadAccess = sstrdup("ServAdmin");
	} else if (ModUnLoadAccess == 5) {
		my_ModUnLoadAccess = sstrdup("NetAdmin");
	} else if (ModUnLoadAccess == 6) {
		my_ModUnLoadAccess = sstrdup("CSop");
	} else if (ModUnLoadAccess == 7) {
		my_ModUnLoadAccess = sstrdup("CSAdmin");
	} else if (ModUnLoadAccess == 8) {
		my_ModUnLoadAccess = sstrdup("Root");
	} else if (ModUnLoadAccess == 9) {
		my_ModUnLoadAccess = sstrdup("Disabled");
	}
	if (ModInfoAccess == 1) {
		my_ModInfoAccess = sstrdup("HelpOp");
	} else if (ModInfoAccess == 2) {
		my_ModInfoAccess = sstrdup("IRCop");
	} else if (ModInfoAccess == 3) {
		my_ModInfoAccess = sstrdup("SVSAdmin");
	} else if (ModInfoAccess == 4) {
		my_ModInfoAccess = sstrdup("ServAdmin");
	} else if (ModInfoAccess == 5) {
		my_ModInfoAccess = sstrdup("NetAdmin");
	} else if (ModInfoAccess == 6) {
		my_ModInfoAccess = sstrdup("CSop");
	} else if (ModInfoAccess == 7) {
		my_ModInfoAccess = sstrdup("CSAdmin");
	} else if (ModInfoAccess == 8) {
		my_ModInfoAccess = sstrdup("Root");
	} else if (ModInfoAccess == 9) {
		my_ModInfoAccess = sstrdup("Disabled");
	}
	if (ModListAccess == 1) {
		my_ModListAccess = sstrdup("HelpOp");
	} else if (ModListAccess == 2) {
		my_ModListAccess = sstrdup("IRCop");
	} else if (ModListAccess == 3) {
		my_ModListAccess = sstrdup("SVSAdmin");
	} else if (ModListAccess == 4) {
		my_ModListAccess = sstrdup("ServAdmin");
	} else if (ModListAccess == 5) {
		my_ModListAccess = sstrdup("NetAdmin");
	} else if (ModListAccess == 6) {
		my_ModListAccess = sstrdup("CSop");
	} else if (ModListAccess == 7) {
		my_ModListAccess = sstrdup("CSAdmin");
	} else if (ModListAccess == 8) {
		my_ModListAccess = sstrdup("Root");
	} else if (ModListAccess == 9) {
		my_ModListAccess = sstrdup("Disabled");
	}
	if (QuitAccess == 1) {
		my_QuitAccess = sstrdup("HelpOp");
	} else if (QuitAccess == 2) {
		my_QuitAccess = sstrdup("IRCop");
	} else if (QuitAccess == 3) {
		my_QuitAccess = sstrdup("SVSAdmin");
	} else if (QuitAccess == 4) {
		my_QuitAccess = sstrdup("ServAdmin");
	} else if (QuitAccess == 5) {
		my_QuitAccess = sstrdup("NetAdmin");
	} else if (QuitAccess == 6) {
		my_QuitAccess = sstrdup("CSop");
	} else if (QuitAccess == 7) {
		my_QuitAccess = sstrdup("CSAdmin");
	} else if (QuitAccess == 8) {
		my_QuitAccess = sstrdup("Root");
	} else if (QuitAccess == 9) {
		my_QuitAccess = sstrdup("Disabled");
	}
	if (ShutDownAccess == 1) {
		my_ShutDownAccess = sstrdup("HelpOp");
	} else if (ShutDownAccess == 2) {
		my_ShutDownAccess = sstrdup("IRCop");
	} else if (ShutDownAccess == 3) {
		my_ShutDownAccess = sstrdup("SVSAdmin");
	} else if (ShutDownAccess == 4) {
		my_ShutDownAccess = sstrdup("ServAdmin");
	} else if (ShutDownAccess == 5) {
		my_ShutDownAccess = sstrdup("NetAdmin");
	} else if (ShutDownAccess == 6) {
		my_ShutDownAccess = sstrdup("CSop");
	} else if (ShutDownAccess == 7) {
		my_ShutDownAccess = sstrdup("CSAdmin");
	} else if (ShutDownAccess == 8) {
		my_ShutDownAccess = sstrdup("Root");
	} else if (ShutDownAccess == 9) {
		my_ShutDownAccess = sstrdup("Disabled");
	}
	if (DefConAccess == 1) {
		my_DefConAccess = sstrdup("HelpOp");
	} else if (DefConAccess == 2) {
		my_DefConAccess = sstrdup("IRCop");
	} else if (DefConAccess == 3) {
		my_DefConAccess = sstrdup("SVSAdmin");
	} else if (DefConAccess == 4) {
		my_DefConAccess = sstrdup("ServAdmin");
	} else if (DefConAccess == 5) {
		my_DefConAccess = sstrdup("NetAdmin");
	} else if (DefConAccess == 6) {
		my_DefConAccess = sstrdup("CSop");
	} else if (DefConAccess == 7) {
		my_DefConAccess = sstrdup("CSAdmin");
	} else if (DefConAccess == 8) {
		my_DefConAccess = sstrdup("Root");
	} else if (DefConAccess == 9) {
		my_DefConAccess = sstrdup("Disabled");
	}
	if (ChanKillAccess == 1) {
		my_ChanKillAccess = sstrdup("HelpOp");
	} else if (ChanKillAccess == 2) {
		my_ChanKillAccess = sstrdup("IRCop");
	} else if (ChanKillAccess == 3) {
		my_ChanKillAccess = sstrdup("SVSAdmin");
	} else if (ChanKillAccess == 4) {
		my_ChanKillAccess = sstrdup("ServAdmin");
	} else if (ChanKillAccess == 5) {
		my_ChanKillAccess = sstrdup("NetAdmin");
	} else if (ChanKillAccess == 6) {
		my_ChanKillAccess = sstrdup("CSop");
	} else if (ChanKillAccess == 7) {
		my_ChanKillAccess = sstrdup("CSAdmin");
	} else if (ChanKillAccess == 8) {
		my_ChanKillAccess = sstrdup("Root");
	} else if (ChanKillAccess == 9) {
		my_ChanKillAccess = sstrdup("Disabled");
	}

	notice(s_OperServ, u->nick, "LEVELS values are:");
	notice(s_OperServ, u->nick, "%s ==========================================================", s_OperServ);
	notice(s_OperServ, u->nick, "Global: %s Stats: %s Oper: %s", (my_GlobalAccess ? my_GlobalAccess : "Default"), (my_StatsAccess ? my_StatsAccess : "Default"), (my_OperAccess ? my_OperAccess : "Default"));
	notice(s_OperServ, u->nick, "Admin: %s Mode: %s Staff: %s", (my_AdminAccess ?  my_AdminAccess : "Default"), (my_ModeAccess ?  my_ModeAccess : "Default"), (my_StaffAccess ? my_StaffAccess : "Default"));
	notice(s_OperServ, u->nick, "Kick: %s ClearModes: %s Akill: %s", (my_KickAccess ?  my_KickAccess : "Default"), (my_ClearModesAccess ?  my_ClearModesAccess : "Default"), (my_AkillAccess ?  my_AkillAccess : "Default"));
	notice(s_OperServ, u->nick, "SGline: %s SQline: %s SZline: %s", (my_SGlineAccess ?  my_SGlineAccess : "Default"), (my_SQlineAccess ?  my_SQlineAccess : "Default"), (my_SZlineAccess ?  my_SZlineAccess : "Default"));
	notice(s_OperServ, u->nick, "ChanList: %s UserList: %s LogonNews: %s", (my_ChanListAccess ?  my_ChanListAccess : "Default"), (my_UserListAccess ?  my_UserListAccess : "Default"), (my_LogonNewsAccess ?  my_LogonNewsAccess : "Default"));
	notice(s_OperServ, u->nick, "RandomNews: %s OperNews: %s Session: %s", (my_RandomNewsAccess ?  my_RandomNewsAccess : "Default"), (my_OperNewsAccess ?  my_OperNewsAccess : "Default"), (my_SessionAccess ?  my_SessionAccess : "Default"));
	notice(s_OperServ, u->nick, "Exception: %s Noop: %s Jupe: %s", (my_ExceptionAccess ?  my_ExceptionAccess : "Default"), (my_NoopAccess ?  my_NoopAccess : "Default"), (my_JupeAccess ?  my_JupeAccess : "Default"));
	notice(s_OperServ, u->nick, "Ignore: %s Set: %s Reload: %s", (my_IgnoreAccess ?  my_IgnoreAccess : "Default"), (my_SetAccess ?  my_SetAccess : "Default"), (my_ReloadAccess ?  my_ReloadAccess : "Default"));
	notice(s_OperServ, u->nick, "Update: %s Restart: %s ModLoad: %s", (my_UpdateAccess ?  my_UpdateAccess : "Default"), (my_RestartAccess ?  my_RestartAccess : "Default"), (my_ModLoadAccess ?  my_ModLoadAccess : "Default"));
	notice(s_OperServ, u->nick, "ModUnLoad: %s ModInfo: %s ModList: %s", (my_ModUnLoadAccess ?  my_ModUnLoadAccess : "Default"), (my_ModInfoAccess ?  my_ModInfoAccess : "Default"), (my_ModListAccess ?  my_ModListAccess : "Default"));
	notice(s_OperServ, u->nick, "Quit: %s ShutDown: %s DefCon: %s", (my_QuitAccess ?  my_QuitAccess : "Default"), (my_ShutDownAccess ?  my_ShutDownAccess : "Default"), (my_DefConAccess ?  my_DefConAccess : "Default"));
	notice(s_OperServ, u->nick, "ChanKill: %s", (my_ChanKillAccess ?  my_ChanKillAccess : "Default"));
	notice(s_OperServ, u->nick, "==================================================================");
	notice(s_OperServ, u->nick, "End of LEVELS values");

   free(my_GlobalAccess);
   free(my_StatsAccess);
   free(my_StaffAccess);
   free(my_OperAccess);
   free(my_AdminAccess);
   free(my_ModeAccess);
   free(my_KickAccess);
   free(my_ClearModesAccess);
   free(my_AkillAccess);
   free(my_SGlineAccess);
   free(my_SQlineAccess);
   free(my_SZlineAccess);
   free(my_ChanListAccess);
   free(my_UserListAccess);
   free(my_LogonNewsAccess);
   free(my_RandomNewsAccess);
   free(my_OperNewsAccess);
   free(my_SessionAccess);
   free(my_ExceptionAccess);
   free(my_NoopAccess);
   free(my_KickAccess);
   free(my_ClearModesAccess);
   free(my_JupeAccess);
   free(my_IgnoreAccess);
   free(my_SetAccess);
   free(my_ReloadAccess);
   free(my_UpdateAccess);
   free(my_RestartAccess);
   free(my_ModLoadAccess);
   free(my_ModUnLoadAccess);
   free(my_ModInfoAccess);
   free(my_ModListAccess);
   free(my_QuitAccess);
   free(my_ShutDownAccess);
   free(my_DefConAccess);
   free(my_ChanKillAccess);

   return MOD_CONT;
}


/***************************************************************************************/

	/* convert numbers to names 
	 * Lets do levels:
	 * 1 = helpop
	 * 2 = ircop 
	 * 3 = svsadmin
	 * 4 = servadmin
	 * 5 = netadmin
	 * 6 = SO
	 * 7 = SA
	 * 8 = SRA
	 * 9 = disabled
	 */

int m_do_global(User * u)
{
	  	/* Allows helper (umode +h) */
 if (GlobalAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (GlobalAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (GlobalAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (GlobalAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (GlobalAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (GlobalAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (GlobalAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (GlobalAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (GlobalAccess == 9) {
	notice(s_OperServ, u->nick, "GLOBAL command is disabled.");
 } else if (!GlobalAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_stats(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (StatsAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (StatsAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (StatsAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (StatsAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (StatsAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (StatsAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (StatsAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (StatsAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (StatsAccess == 9) {
	notice(s_OperServ, u->nick, "STATS command is disabled.");
 } else if (!StatsAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_staff(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (StaffAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (StaffAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (StaffAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (StaffAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (StaffAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (StaffAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (StaffAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (StaffAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (StaffAccess == 9) {
	notice(s_OperServ, u->nick, "STAFF command is disabled.");
 } else if (!StaffAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_mode(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (ModeAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ModeAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ModeAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ModeAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ModeAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ModeAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ModeAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ModeAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ModeAccess == 9) {
	notice(s_OperServ, u->nick, "MODE command is disabled.");
 } else if (!ModeAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_kick(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (KickAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (KickAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (KickAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (KickAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (KickAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (KickAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (KickAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (KickAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (KickAccess == 9) {
	notice(s_OperServ, u->nick, "KICK command is disabled.");
 } else if (!KickAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}
/***************************************************************************************/

int m_do_clearmodes(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (ClearModesAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ClearModesAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ClearModesAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ClearModesAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ClearModesAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ClearModesAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ClearModesAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ClearModesAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ClearModesAccess == 9) {
	notice(s_OperServ, u->nick, "CLEARMODES command is disabled.");
 } else if (!ClearModesAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_akill(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (AkillAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (AkillAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (AkillAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (AkillAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (AkillAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (AkillAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (AkillAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (AkillAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (AkillAccess == 9) {
	notice(s_OperServ, u->nick, "AKILL command is disabled.");
 } else if (!AkillAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_sgline(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (SGlineAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (SGlineAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (SGlineAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (SGlineAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (SGlineAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (SGlineAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (SGlineAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (SGlineAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (SGlineAccess == 9) {
	notice(s_OperServ, u->nick, "SGLINE command is disabled.");
 } else if (!SGlineAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_sqline(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (SQlineAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (SQlineAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (SQlineAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (SQlineAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (SQlineAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (SQlineAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (SQlineAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (SQlineAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (SQlineAccess == 9) {
	notice(s_OperServ, u->nick, "SQLINE command is disabled.");
 } else if (!SQlineAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_szline(User * u)
{ 
	  	/* Allows helper (umode +h) */
 if (SZlineAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (SZlineAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (SZlineAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (SZlineAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (SZlineAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (SZlineAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (SZlineAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (SZlineAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (SZlineAccess == 9) {
	notice(s_OperServ, u->nick, "SZLINE command is disabled.");
 } else if (!SZlineAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_chanlist(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ChanListAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ChanListAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ChanListAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ChanListAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ChanListAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ChanListAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ChanListAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ChanListAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ChanListAccess == 9) {
	notice(s_OperServ, u->nick, "CHANLIST command is disabled.");
 } else if (!ChanListAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_userlist(User * u)
{
	  	/* Allows helper (umode +h) */
 if (UserListAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (UserListAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (UserListAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (UserListAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (UserListAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (UserListAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (UserListAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (UserListAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (UserListAccess == 9) {
	notice(s_OperServ, u->nick, "USERLIST command is disabled.");
 } else if (!UserListAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_logonnews(User * u)
{
	  	/* Allows helper (umode +h) */
 if (LogonNewsAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (LogonNewsAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (LogonNewsAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (LogonNewsAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (LogonNewsAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (LogonNewsAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (LogonNewsAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (LogonNewsAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (LogonNewsAccess == 9) {
	notice(s_OperServ, u->nick, "LOGONNEWS command is disabled.");
 } else if (!LogonNewsAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_randomnews(User * u)
{
	  	/* Allows helper (umode +h) */
 if (RandomNewsAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (RandomNewsAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (RandomNewsAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (RandomNewsAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (RandomNewsAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (RandomNewsAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (RandomNewsAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (RandomNewsAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (RandomNewsAccess == 9) {
	notice(s_OperServ, u->nick, "RANDOMNEWS command is disabled.");
 } else if (!RandomNewsAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_opernews(User * u)
{
	  	/* Allows helper (umode +h) */
 if (OperNewsAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (OperNewsAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (OperNewsAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (OperNewsAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (OperNewsAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (OperNewsAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (OperNewsAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (OperNewsAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (OperNewsAccess == 9) {
	notice(s_OperServ, u->nick, "OPERNEWS command is disabled.");
 } else if (!OperNewsAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_reload(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ReloadAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ReloadAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ReloadAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ReloadAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ReloadAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ReloadAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ReloadAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ReloadAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ReloadAccess == 9) {
	notice(s_OperServ, u->nick, "RELOAD command is disabled.");
 } else if (!ReloadAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_restart(User * u)
{
	  	/* Allows helper (umode +h) */
 if (RestartAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (RestartAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (RestartAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (RestartAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (RestartAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (RestartAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (RestartAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (RestartAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (RestartAccess == 9) {
	notice(s_OperServ, u->nick, "RESTART command is disabled.");
 } else if (!RestartAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_modload(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ModLoadAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ModLoadAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ModLoadAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ModLoadAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ModLoadAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ModLoadAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ModLoadAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ModLoadAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ModLoadAccess == 9) {
	notice(s_OperServ, u->nick, "MODLOAD command is disabled.");
 } else if (!ModLoadAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_modunload(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ModUnLoadAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ModUnLoadAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ModUnLoadAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ModUnLoadAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ModUnLoadAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ModUnLoadAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ModUnLoadAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ModUnLoadAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ModUnLoadAccess == 9) {
	notice(s_OperServ, u->nick, "MODUNLOAD command is disabled.");
 } else if (!ModUnLoadAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_modinfo(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ModInfoAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ModInfoAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ModInfoAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ModInfoAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ModInfoAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ModInfoAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ModInfoAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ModInfoAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ModInfoAccess == 9) {
	notice(s_OperServ, u->nick, "MODINFO command is disabled.");
 } else if (!ModInfoAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_modlist(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ModListAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ModListAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ModListAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ModListAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ModListAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ModListAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ModListAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ModListAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ModListAccess == 9) {
	notice(s_OperServ, u->nick, "MODLIST command is disabled.");
 } else if (!ModListAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_quit(User * u)
{
	  	/* Allows helper (umode +h) */
 if (QuitAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (QuitAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (QuitAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (QuitAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (QuitAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (QuitAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (QuitAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (QuitAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (QuitAccess == 9) {
	notice(s_OperServ, u->nick, "QUIT command is disabled.");
 } else if (!QuitAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_shutdown(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ShutDownAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ShutDownAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ShutDownAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ShutDownAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ShutDownAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ShutDownAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ShutDownAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ShutDownAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ShutDownAccess == 9) {
	notice(s_OperServ, u->nick, "SHOWDOWN command is disabled.");
 } else if (!ShutDownAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_defcon(User * u)
{
	  	/* Allows helper (umode +h) */
 if (DefConAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (DefConAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (DefConAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (DefConAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (DefConAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (DefConAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (DefConAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (DefConAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (DefConAccess == 9) {
	notice(s_OperServ, u->nick, "DEFCON command is disabled.");
 } else if (!DefConAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_chankill(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ChanKillAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ChanKillAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ChanKillAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ChanKillAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ChanKillAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ChanKillAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ChanKillAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ChanKillAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ChanKillAccess == 9) {
	notice(s_OperServ, u->nick, "CHANKILL command is disabled.");
 } else if (!ChanKillAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_oper(User * u)
{
	  	/* Allows helper (umode +h) */
 if (OperAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (OperAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (OperAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (OperAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (OperAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (OperAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (OperAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (OperAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (OperAccess == 9) {
	notice(s_OperServ, u->nick, "OPER command is disabled.");
 } else if (!OperAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_admin(User * u)
{
	  	/* Allows helper (umode +h) */
 if (AdminAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (AdminAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (AdminAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (AdminAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (AdminAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (AdminAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (AdminAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (AdminAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (AdminAccess == 9) {
	notice(s_OperServ, u->nick, "ADMIN command is disabled.");
 } else if (!AdminAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}
/***************************************************************************************/

int m_do_session(User * u)
{
	  	/* Allows helper (umode +h) */
 if (SessionAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (SessionAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (SessionAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (SessionAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (SessionAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (SessionAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (SessionAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (SessionAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (SessionAccess == 9) {
	notice(s_OperServ, u->nick, "SESSION command is disabled.");
 } else if (!SessionAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_exception(User * u)
{
	  	/* Allows helper (umode +h) */
 if (ExceptionAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (ExceptionAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (ExceptionAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (ExceptionAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (ExceptionAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (ExceptionAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (ExceptionAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (ExceptionAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (ExceptionAccess == 9) {
	notice(s_OperServ, u->nick, "EXCEPTION command is disabled.");
 } else if (!ExceptionAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_noop(User * u)
{
	  	/* Allows helper (umode +h) */
 if (NoopAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (NoopAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (NoopAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (NoopAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (NoopAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (NoopAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (NoopAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (NoopAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (NoopAccess == 9) {
	notice(s_OperServ, u->nick, "NOOP command is disabled.");
 } else if (!NoopAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_jupe(User * u)
{
	  	/* Allows helper (umode +h) */
 if (JupeAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (JupeAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (JupeAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (JupeAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (JupeAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (JupeAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (JupeAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (JupeAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (JupeAccess == 9) {
	notice(s_OperServ, u->nick, "JUPE command is disabled.");
 } else if (!JupeAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_ignore(User * u)
{
	  	/* Allows helper (umode +h) */
 if (IgnoreAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (IgnoreAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (IgnoreAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (IgnoreAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (IgnoreAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (IgnoreAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (IgnoreAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (IgnoreAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (IgnoreAccess == 9) {
	notice(s_OperServ, u->nick, "IGNORE command is disabled.");
 } else if (!IgnoreAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_set(User * u)
{
	  	/* Allows helper (umode +h) */
 if (SetAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (SetAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (SetAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (SetAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (SetAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (SetAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (SetAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (SetAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (SetAccess == 9) {
	notice(s_OperServ, u->nick, "SET command is disabled.");
 } else if (!SetAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int m_do_update(User * u)
{
	  	/* Allows helper (umode +h) */
 if (UpdateAccess == 1 && is_helper(u)) {
  return MOD_CONT;
  		/* Allows Oper (umode +o) */
 } else if (UpdateAccess == 2 && is_oper(u)) {
  return MOD_CONT;
  		/* Allows Services Admin (umode +a) */
 } else if (UpdateAccess == 3 && is_svsadmin(u)) {
  return MOD_CONT;
  		/* Allows Server Admin (umode +A) */
 } else if (UpdateAccess == 4 && is_servadmin(u)) {
  return MOD_CONT;
		/* Allows NetAdmin (umode +N) */
 } else if (UpdateAccess == 5 && is_netadmin(u)) {
  return MOD_CONT;
		/* Allows Services Oper (SO) */
 } else if (UpdateAccess == 6 && is_services_oper(u)) {
  return MOD_CONT;
		/* Allows Services Admin (SA) */
 } else if (UpdateAccess == 7 && is_services_admin(u)) {
  return MOD_CONT;
		/* Allows Services Root Admin (SRA) */
 } else if (UpdateAccess == 8 && is_services_root(u)) {
  return MOD_CONT;
		/* Command Disabled */
 } else if (UpdateAccess == 9) {
	notice(s_OperServ, u->nick, "UPDATE command is disabled.");
 } else if (!UpdateAccess) {
  return MOD_CONT;
 } else {
	 notice_lang(s_OperServ, u, PERMISSION_DENIED);
 }
 return MOD_STOP;
}

/***************************************************************************************/

int load_config(void)
{
	int i;
	
    Directive confvalues[][1] = {

		{{"OperAccess", {{PARAM_INT, PARAM_RELOAD, &OperAccess}}}},
		{{"StatsAccess", {{PARAM_INT, PARAM_RELOAD, &StatsAccess}}}},
		{{"StaffAccess", {{PARAM_INT, PARAM_RELOAD, &StaffAccess}}}},
		{{"ModeAccess", {{PARAM_INT, PARAM_RELOAD, &ModeAccess}}}},
		{{"KickAccess", {{PARAM_INT, PARAM_RELOAD, &KickAccess}}}},
		{{"ClearModesAccess", {{PARAM_INT, PARAM_RELOAD, &ClearModesAccess}}}},
		{{"AkillAccess", {{PARAM_INT, PARAM_RELOAD, &AkillAccess}}}},
		{{"SGlineAccess", {{PARAM_INT, PARAM_RELOAD, &SGlineAccess}}}},
		{{"SQlineAccess", {{PARAM_INT, PARAM_RELOAD, &SQlineAccess}}}},
		{{"SZlineAccess", {{PARAM_INT, PARAM_RELOAD, &SZlineAccess}}}},
		{{"ChanListAccess", {{PARAM_INT, PARAM_RELOAD, &ChanListAccess}}}},
		{{"UserListAccess", {{PARAM_INT, PARAM_RELOAD, &UserListAccess}}}},
		{{"LogonNewsAccess", {{PARAM_INT, PARAM_RELOAD, &LogonNewsAccess}}}},
		{{"RandomNewsAccess", {{PARAM_INT, PARAM_RELOAD, &RandomNewsAccess}}}},
		{{"OperNewsAccess", {{PARAM_INT, PARAM_RELOAD, &OperNewsAccess}}}},
		{{"ReloadAccess", {{PARAM_INT, PARAM_RELOAD, &ReloadAccess}}}},
		{{"RestartAccess", {{PARAM_INT, PARAM_RELOAD, &RestartAccess}}}},
		{{"ModLoadAccess", {{PARAM_INT, PARAM_RELOAD, &ModLoadAccess}}}},
		{{"ModUnLoadAccess", {{PARAM_INT, PARAM_RELOAD, &ModUnLoadAccess}}}},
		{{"ModInfoAccess", {{PARAM_INT, PARAM_RELOAD, &ModInfoAccess}}}},
		{{"ModListAccess", {{PARAM_INT, PARAM_RELOAD, &ModListAccess}}}},
		{{"QuitAccess", {{PARAM_INT, PARAM_RELOAD, &QuitAccess}}}},
		{{"ShutDownAccess", {{PARAM_INT, PARAM_RELOAD, &ShutDownAccess}}}},
		{{"DefConAccess", {{PARAM_INT, PARAM_RELOAD, &DefConAccess}}}},
		{{"ChanKillAccess", {{PARAM_INT, PARAM_RELOAD, &ChanKillAccess}}}},
		{{"OperAccess", {{PARAM_INT, PARAM_RELOAD, &OperAccess}}}},
		{{"AdminAccess", {{PARAM_INT, PARAM_RELOAD, &AdminAccess}}}},
		{{"SessionAccess", {{PARAM_INT, PARAM_RELOAD, &SessionAccess}}}},
		{{"ExceptionAccess", {{PARAM_INT, PARAM_RELOAD, &ExceptionAccess}}}},
		{{"NoopAccess", {{PARAM_INT, PARAM_RELOAD, &NoopAccess}}}},
		{{"JupeAccess", {{PARAM_INT, PARAM_RELOAD, &JupeAccess}}}},
		{{"IgnoreAccess", {{PARAM_INT, PARAM_RELOAD, &IgnoreAccess}}}},
		{{"SetAccess", {{PARAM_INT, PARAM_RELOAD, &SetAccess}}}},
		{{"UpdateAccess", {{PARAM_INT, PARAM_RELOAD, &UpdateAccess}}}},
    };
	
	for (i = 0; i < 34; i++)
    	moduleGetConfigDirective(confvalues[i]);

	return 1;
}
/* EOF */

