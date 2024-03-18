/***************************************************************************************/
/* Anope Module : os_confinfo.c : v2.x                                                 */
/* Scott Seufert - katsklaw@ircmojo.org                                                */
/*                                                                                     */
/* ircmojo (c) 2010 ircmojo.org                                                        */
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
#include <module.h>

#define AUTHOR "katsklaw"
#define VERSION "2.5"

int do_ircdconf(User *u);
int do_operconf(User *u);
int do_nickconf(User *u);
int do_hostconf(User *u);
int do_botconf(User *u);
int do_chanconf(User *u);
int do_memoconf(User *u);


void Module_Help_OPERSERV(User *u);
int Module_Help_OPERSERV_CONFINFO(User *u);

void Module_Help_NICKSERV(User *u);
int Module_Help_NICKSERV_CONFINFO(User *u);

void Module_Help_HOSTSERV(User *u);
int Module_Help_HOSTSERV_CONFINFO(User *u);

void Module_Help_MEMOSERV(User *u);
int Module_Help_MEMOSERV_CONFINFO(User *u);

void Module_Help_BOTSERV(User *u);
int Module_Help_BOTSERV_CONFINFO(User *u);

void Module_Help_CHANSERV(User *u);
int Module_Help_CHANSERV_CONFINFO(User *u);

int AnopeInit(int argc, char **argv)
{
	Command *c;
	c = createCommand("CONFINFO", do_operconf, is_services_admin,-1,-1,-1,-1,-1);
	moduleAddCommand(OPERSERV,c,MOD_HEAD);
	moduleAddHelp(c, Module_Help_OPERSERV_CONFINFO);
	moduleSetOperHelp(Module_Help_OPERSERV);

	c = createCommand("IRCDINFO", do_ircdconf, is_services_root,-1,-1,-1,-1,-1);
	moduleAddCommand(OPERSERV,c,MOD_HEAD);

	c = createCommand("CONFINFO", do_nickconf, NULL,-1,-1,-1,-1,-1);
	moduleAddCommand(NICKSERV,c,MOD_HEAD);
	moduleAddHelp(c, Module_Help_NICKSERV_CONFINFO);
	moduleSetNickHelp(Module_Help_NICKSERV);
	
	c = createCommand("CONFINFO", do_hostconf, NULL,-1,-1,-1,-1,-1);
	moduleAddCommand(HOSTSERV,c,MOD_HEAD);
	moduleAddHelp(c, Module_Help_HOSTSERV_CONFINFO);
	moduleSetHostHelp(Module_Help_HOSTSERV);
	
	c = createCommand("CONFINFO", do_memoconf, NULL,-1,-1,-1,-1,-1);
	moduleAddCommand(MEMOSERV,c,MOD_HEAD);
	moduleAddHelp(c, Module_Help_MEMOSERV_CONFINFO);
	moduleSetMemoHelp(Module_Help_MEMOSERV);
	
	c = createCommand("CONFINFO", do_chanconf, NULL,-1,-1,-1,-1,-1);
	moduleAddCommand(CHANSERV,c,MOD_HEAD);
	moduleAddHelp(c, Module_Help_CHANSERV_CONFINFO);
	moduleSetChanHelp(Module_Help_CHANSERV);
	
	c = createCommand("CONFINFO", do_botconf, NULL,-1,-1,-1,-1,-1);
	moduleAddCommand(BOTSERV,c,MOD_HEAD);
	moduleAddHelp(c, Module_Help_BOTSERV_CONFINFO);
	moduleSetBotHelp(Module_Help_BOTSERV);

    	alog("Loading module this will add CONFINFO to %s",s_OperServ);


    	moduleAddAuthor(AUTHOR);
    	moduleAddVersion(VERSION);

	return MOD_CONT;

}

/***************************************************************************************/

void Module_Help_OPERSERV(User *u)
{
   notice(s_OperServ,u->nick, "    CONFINFO   Lists various configuration info.");
}

/***************************************************************************************/

int Module_Help_OPERSERV_CONFINFO(User *u)
{
        notice(s_OperServ, u->nick, " Syntax: CONFINFO");
        notice(s_OperServ, u->nick, "   ");
        notice(s_OperServ, u->nick, " Allows Services Administrators and up see various config");
        notice(s_OperServ, u->nick, " settings that don't normally have shell access to services. ");
        notice(s_OperServ, u->nick, "   ");
        notice(s_OperServ, u->nick, " Requires Services Admin");
        return MOD_CONT;
}

/***************************************************************************************/

void Module_Help_NICKSERV(User *u)
{
   notice(s_NickServ,u->nick, "    CONFINFO   Lists various configuration info.");
}

/***************************************************************************************/

int Module_Help_NICKSERV_CONFINFO(User *u)
{
        notice(s_NickServ, u->nick, " Syntax: CONFINFO");
        notice(s_NickServ, u->nick, "   ");
        notice(s_NickServ, u->nick, " Allows users see various config settings.");

		return MOD_CONT;
}

/***************************************************************************************/

void Module_Help_BOTSERV(User *u)
{
   notice(s_BotServ,u->nick, "    CONFINFO   Lists various configuration info.");
}

/***************************************************************************************/

int Module_Help_BOTSERV_CONFINFO(User *u)
{
        notice(s_BotServ, u->nick, " Syntax: CONFINFO");
        notice(s_BotServ, u->nick, "   ");
        notice(s_BotServ, u->nick, " Allows users and up see various config settings.");
        
		return MOD_CONT;
}

/***************************************************************************************/

void Module_Help_HOSTSERV(User *u)
{
   notice(s_HostServ,u->nick, "    CONFINFO   Lists various configuration info.");
}

/***************************************************************************************/

int Module_Help_HOSTSERV_CONFINFO(User *u)
{
        notice(s_HostServ, u->nick, " Syntax: CONFINFO");
        notice(s_HostServ, u->nick, "   ");
        notice(s_HostServ, u->nick, " Allows users see various config settings.");

		return MOD_CONT;
}

/***************************************************************************************/

void Module_Help_MEMOSERV(User *u)
{
   notice(s_MemoServ,u->nick, "    CONFINFO   Lists various configuration info.");
}

/***************************************************************************************/

int Module_Help_MEMOSERV_CONFINFO(User *u)
{
        notice(s_MemoServ, u->nick, " Syntax: CONFINFO");
        notice(s_MemoServ, u->nick, "   ");
        notice(s_MemoServ, u->nick, " Allows users see various config settings.");

		return MOD_CONT;
}

/***************************************************************************************/

void Module_Help_CHANSERV(User *u)
{
   notice(s_ChanServ,u->nick, "    CONFINFO   Lists various configuration info.");
}

/***************************************************************************************/

int Module_Help_CHANSERV_CONFINFO(User *u)
{
        notice(s_ChanServ, u->nick, " Syntax: CONFINFO");
        notice(s_ChanServ, u->nick, "   ");
        notice(s_ChanServ, u->nick, " Allows users see various config settings.");

		return MOD_CONT;
}

/***************************************************************************************/

void AnopeFini(void) {
  alog("Unloading os_confinfo");
}

int do_ircdconf (User *u)
{
	notice(ServerName, u->nick, " - ");
	notice(ServerName, u->nick, "%s Configuration", ServerName);
	notice(ServerName, u->nick, "Chanmode +aq: %s Owner: %s Admin: %s", (ircd->owner ? "Enabled" : "Disabled" ), (ircd->owner ? ircd->ownerset : "Disabled" ), (ircd->owner ? ircd->adminset : "Disabled" ));
	notice(ServerName, u->nick, "Registration modes: On Nick: %s OnReg: %s OnUnreg: %s", ircd->modeonnick, ircd->modeonreg, ircd->modeonunreg);
	notice(ServerName, u->nick, " - ");
	notice(ServerName, u->nick, "End of Configuration");

					return MOD_CONT;

}

int do_operconf (User *u)
{
	char *my_encmodule = NULL;
	char *my_ircdmodule = NULL;
	char *my_services_root = NULL;
	char *my_update_timeout = NULL;
	char *my_pidfile = NULL;
	char *my_motdfile = NULL;
	char *my_numeric = NULL;
	char *my_helpchan = NULL;
	char *my_logchan = NULL;
	char *my_newscount = NULL;
	char *my_3rdmodules = NULL;
	char *my_servername = NULL;
	char *my_networkname = NULL;
	char *my_ulineservers = NULL;
	char *my_remoteserver = NULL;
	char *my_3rdmodules2 = NULL;
	int my_superadmin = 0;
	char *my_mysqlhost = NULL;
	char *my_mysqluser = NULL;
	char *my_mysqlpass = NULL;
	char *my_mysqlname = NULL;
	char *my_mysqlsock = NULL;
	char *my_mysqlport = NULL;
	char *my_mysqlsecure = NULL;
	char *my_mysqlretries = NULL;
	char *my_mysqlretrygap = NULL;
	int my_userdb = 0;
	int my_osopersonly = 0;
	char *my_opercoremodules = NULL;
	char *my_akillexpire = NULL;
	char *my_chankillexpire = NULL;
	char *my_sglineexpire = NULL;
	char *my_sqlineexpire = NULL;
	char *my_szlineexpire = NULL;
	char *my_exceptionexpire = NULL;
	char *my_sessionakillexpire = NULL;
	int my_limitsessions = 0;
	char *my_defsessionlimit = NULL;
	char *my_maxsessionlimit = NULL;
	char *my_sessionlimitdetailsloc = NULL;
	char *my_sessionlimitexceeded = NULL;

	Directive confvalues[] = {
		{ "OSOpersOnly", { { PARAM_SET, PARAM_RELOAD, &my_osopersonly } } }
	};

	Directive confvalues1[] = {
		{ "OperCoreModules", { { PARAM_STRING, PARAM_RELOAD, &my_opercoremodules } } }
	};

	Directive confvalues2[] = {
		{ "AutoKillExpiry", { { PARAM_STRING, PARAM_RELOAD, &my_akillexpire } } }
	};

	Directive confvalues3[] = {
		{ "ChanKillExpiry", { { PARAM_STRING, PARAM_RELOAD, &my_chankillexpire } } }
	};

	Directive confvalues4[] = {
		{ "SGLineExpiry", { { PARAM_STRING, PARAM_RELOAD, &my_sglineexpire } } }
	};

	Directive confvalues5[] = {
		{ "SQLineExpiry", { { PARAM_STRING, PARAM_RELOAD, &my_sqlineexpire } } }
	};

	Directive confvalues6[] = {
		{ "SZLineExpiry", { { PARAM_STRING, PARAM_RELOAD, &my_szlineexpire } } }
	};

	Directive confvalues7[] = {
		{ "ExceptionExpiry", { { PARAM_STRING, PARAM_RELOAD, &my_exceptionexpire } } }
	};

	Directive confvalues8[] = {
		{ "SessionAutoKillExpiry", { { PARAM_STRING, PARAM_RELOAD, &my_sessionakillexpire } } }
	};

	Directive confvalues9[] = {
		{ "LimitSessions", { { PARAM_SET, PARAM_RELOAD, &my_limitsessions } } }
	};

	Directive confvalues10[] = {
		{ "DefSessionLimit", { { PARAM_STRING, PARAM_RELOAD, &my_defsessionlimit } } }
	};

	Directive confvalues11[] = {
		{ "MaxSessionLimit", { { PARAM_STRING, PARAM_RELOAD, &my_maxsessionlimit } } }
	};

	Directive confvalues12[] = {
		{ "SessionLimitDetailsLoc", { { PARAM_STRING, PARAM_RELOAD, &my_sessionlimitdetailsloc } } }
	};

	Directive confvalues13[] = {
		{ "SessionLimitExceeded", { { PARAM_STRING, PARAM_RELOAD, &my_sessionlimitexceeded } } }
	};

	Directive confvalues14[] = {
       { "ServicesRoot", { { PARAM_STRING, PARAM_RELOAD, &my_services_root } } }
	};

	Directive confvalues15[] = {
       { "UpdateTimeout", { { PARAM_STRING, PARAM_RELOAD, &my_update_timeout } } }
	};

	Directive confvalues16[] = {
       { "PIDFile", { { PARAM_STRING, PARAM_RELOAD, &my_pidfile } } }
	};
 
	Directive confvalues17[] = {
       { "MOTDFile", { { PARAM_STRING, PARAM_RELOAD, &my_motdfile } } }
	};

	Directive confvalues18[] = {
       { "Numeric", { { PARAM_STRING, PARAM_RELOAD, &my_numeric } } }
	};

	Directive confvalues19[] = {
       { "HelpChannel", { { PARAM_STRING, PARAM_RELOAD, &my_helpchan } } }
	};

	Directive confvalues20[] = {
       { "LogChannel", { { PARAM_STRING, PARAM_RELOAD, &my_logchan } } }
	};
  
	Directive confvalues21[] = {
       { "ENCModule", { { PARAM_STRING, PARAM_RELOAD, &my_encmodule } } }
	};

	Directive confvalues22[] = {
       { "IRCDModule", { { PARAM_STRING, PARAM_RELOAD, &my_ircdmodule } } }
	};

	Directive confvalues23[] = {
       { "NewsCount", { { PARAM_STRING, PARAM_RELOAD, &my_newscount } } }
	};

	Directive confvalues24[] = {
       { "ModuleDelayedAutoload", { { PARAM_STRING, PARAM_RELOAD, &my_3rdmodules } } }
	};

	Directive confvalues25[] = {
       { "ServerName", { { PARAM_STRING, PARAM_RELOAD, &my_servername } } }
	};

	Directive confvalues26[] = {
       { "NetworkName", { { PARAM_STRING, PARAM_RELOAD, &my_networkname } } }
	};

	Directive confvalues27[] = {
       { "UlineServers", { { PARAM_STRING, PARAM_RELOAD, &my_ulineservers } } }
	};

	Directive confvalues28[] = {
       { "RemoteServer", { { PARAM_STRING, PARAM_RELOAD, &my_remoteserver } } }
	};

	Directive confvalues29[] = {
       { "ModuleAutoload", { { PARAM_STRING, PARAM_RELOAD, &my_3rdmodules2 } } }
	};
   
	Directive confvalues30[] = {
       { "SuperAdmin", { { PARAM_SET, PARAM_RELOAD, &my_superadmin } } }
	};

	Directive confvalues31[] = {
       { "MysqlHost", { { PARAM_STRING, PARAM_RELOAD, &my_mysqlhost } } }
	};
   
	Directive confvalues32[] = {
       { "MysqlUser", { { PARAM_STRING, PARAM_RELOAD, &my_mysqluser } } }
	};

	Directive confvalues33[] = {
       { "MysqlPass", { { PARAM_STRING, PARAM_RELOAD, &my_mysqlpass } } }
	};
	Directive confvalues34[] = {
       { "MysqlName", { { PARAM_STRING, PARAM_RELOAD, &my_mysqlname } } }
	};

	Directive confvalues35[] = {
       { "MysqlSock", { { PARAM_STRING, PARAM_RELOAD, &my_mysqlsock } } }
	};

	Directive confvalues36[] = {
       { "MysqlPort", { { PARAM_STRING, PARAM_RELOAD, &my_mysqlport } } }
	};

	Directive confvalues37[] = {
       { "MysqlSecure", { { PARAM_STRING, PARAM_RELOAD, &my_mysqlsecure } } }
	};
   
	Directive confvalues38[] = {
       { "MysqlRetries", { { PARAM_STRING, PARAM_RELOAD, &my_mysqlretries } } }
	};
   
	Directive confvalues39[] = {
       { "MysqRetryGap", { { PARAM_STRING, PARAM_RELOAD, &my_mysqlretrygap } } }
	};
   
	Directive confvalues40[] = {
       { "UseRDB", { { PARAM_SET, PARAM_RELOAD, &my_userdb } } }
	};

   moduleGetConfigDirective(confvalues);
   moduleGetConfigDirective(confvalues1);
   moduleGetConfigDirective(confvalues2);
   moduleGetConfigDirective(confvalues3);
   moduleGetConfigDirective(confvalues4);
   moduleGetConfigDirective(confvalues5);
   moduleGetConfigDirective(confvalues6);
   moduleGetConfigDirective(confvalues7);
   moduleGetConfigDirective(confvalues8);
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
   moduleGetConfigDirective(confvalues35);
   moduleGetConfigDirective(confvalues36);
   moduleGetConfigDirective(confvalues37);
   moduleGetConfigDirective(confvalues38);
   moduleGetConfigDirective(confvalues39);
   moduleGetConfigDirective(confvalues40);


	notice(s_OperServ, u->nick, "Basic Configuration - (%s) ", my_servername);
	notice(s_OperServ, u->nick, "IRCd Protocol: %s              Numeric: %s", my_ircdmodule, (my_numeric ? my_numeric : "Disabled"));
	notice(s_OperServ, u->nick, "Services Root: %s             Update Timeout: %s                          PIDFile: %s", my_services_root, my_update_timeout, my_pidfile);
	notice(s_OperServ, u->nick, "MOTDFile: %s                  Help Channel: %s                            Log Channel: %s", my_motdfile, (my_helpchan ? my_helpchan : "Disabled"), (my_logchan ? my_logchan : "Disabled"));
	notice(s_OperServ, u->nick, "News Count: %s               Network Name: %s                          U:Lined Servers: %s", (my_newscount ? my_newscount : "Disabled" ), my_networkname, my_ulineservers);
	notice(s_OperServ, u->nick, "Super Admin: %s", (my_superadmin ? "Enabled" : "Disabled" ));
	
	if (is_services_root(u)) {
		notice(s_OperServ, u->nick, " ");
		notice(s_OperServ, u->nick, "^Root Only section^");
		notice(s_OperServ, u->nick, "Encryption Type: %s  Connect to: %s", my_encmodule, my_remoteserver);
		notice(s_OperServ, u->nick, "MySQL Host: %s               MySQL User: %s                   MySQL Pass: %s", my_mysqlhost, my_mysqluser, my_mysqlpass);
		notice(s_OperServ, u->nick, "MySQL DB Name: %s        MySQL Sock: %s                  MySQL Port: %s", my_mysqlname, my_mysqlsock, my_mysqlport);
		notice(s_OperServ, u->nick, "MySQL Secure: %s           MySQL Retries: %s               MySQL Retry Gap: %s", (my_mysqlsecure ? my_mysqlsecure : "Disabled" ), (my_mysqlretries ? my_mysqlretries : "Disabled" ), (my_mysqlretrygap ? my_mysqlretrygap : "Disabled" ));
		notice(s_OperServ, u->nick, "Use Rational DB: Enabled");
		notice(s_OperServ, u->nick, "^End Root Only section^");
	}
	notice(s_OperServ, u->nick, " - ");
	notice(s_OperServ, u->nick, "%s Configuration - Modes:%s",s_OperServ, ircd->operservmode);
		
	if (OSOpersOnly) {
		notice(s_OperServ, u->nick, "OperServ Opers Only: Enabled");
	} else {
		notice(s_OperServ, u->nick, "OperServ Opers Only: Disabled");
	}
	notice(s_OperServ, u->nick, " - ");
	notice(s_OperServ, u->nick, "Expirations");
	notice(s_OperServ, u->nick, "AutoKill: %s                              ChanKill: %s                                         SGLine: %s", my_akillexpire, my_chankillexpire, my_sglineexpire);
	notice(s_OperServ, u->nick, "SQLine: %s                              SZLine: %s                                           Exception: %s", my_sqlineexpire, my_szlineexpire, my_exceptionexpire);
	notice(s_OperServ, u->nick, "Session autokill: %s", my_sessionakillexpire);
	notice(s_OperServ, u->nick, " - ");
		
	if (LimitSessions) {
		notice(s_OperServ, u->nick, "Sessions");
		notice(s_OperServ, u->nick, "Default session limit: %s                  Max session limit: %s", my_defsessionlimit, my_maxsessionlimit);
		notice(s_OperServ, u->nick, "Limit exceeded kill message: %s", my_sessionlimitexceeded);
		notice(s_OperServ, u->nick, "Limit details URL: %s", my_sessionlimitdetailsloc);
	}
	notice(s_OperServ, u->nick, "Core Modules: %s", my_opercoremodules);
	notice(s_OperServ, u->nick, " ");
	notice(s_OperServ, u->nick, "Bot Nicks: %s, %s, %s, %s, %s, %s", s_OperServ, s_MemoServ, s_ChanServ, s_NickServ, s_HostServ, s_BotServ);
	notice(s_OperServ, u->nick, "3rd Party Modules: %s", (my_3rdmodules2 ? my_3rdmodules2 : "None" ));
	notice(s_OperServ, u->nick, "3rd Party Modules (delayed): %s", (my_3rdmodules ? my_3rdmodules : "None" ));
	notice(s_OperServ, u->nick, " - ");
	notice(s_OperServ, u->nick, "End of Configuration");

	free(my_services_root);
	free(my_update_timeout);
	free(my_pidfile);
	free(my_motdfile);
	free(my_numeric);
	free(my_helpchan);
	free(my_logchan);
	free(my_ircdmodule);
	free(my_encmodule);
	free(my_newscount);
	free(my_3rdmodules);
	free(my_servername);
	free(my_remoteserver);
	free(my_3rdmodules2);
	free(my_mysqlhost);
	free(my_mysqluser);
	free(my_mysqlpass);
	free(my_mysqlname);
	free(my_mysqlsock);
	free(my_mysqlport);
	free(my_mysqlsecure);
	free(my_mysqlretries);
	free(my_mysqlretrygap);
	free(my_opercoremodules);
	free(my_akillexpire);
	free(my_chankillexpire);
	free(my_sglineexpire);
	free(my_sqlineexpire);
	free(my_szlineexpire);
	free(my_exceptionexpire);
	free(my_sessionakillexpire);
	free(my_defsessionlimit);
	free(my_maxsessionlimit);
	free(my_sessionlimitdetailsloc);
	free(my_sessionlimitexceeded);
	
	return MOD_CONT;

}

int do_chanconf (User *u)
{

						char *my_csexpire = NULL;
						char *my_csregmax = NULL;
						char *my_csaccessmax = NULL; 
						int my_csopersonly = 0;
						char *my_csinhabit = NULL;
						char *my_csdefbantype = NULL;
						char *my_cslistopersonly = NULL;
						char *my_cslistmax = NULL;
						char *my_csakickmax = NULL;
						char *my_csgetpass = NULL;
						char *my_chancoremodules = NULL;
						char *my_csautokickreason = NULL;

				Directive confvalues[] = {
					{ "CSRestrictGetPass", { { PARAM_SET, PARAM_RELOAD, &my_csgetpass } } }
				};
				Directive confvalues1[] = {
					{ "CSExpire", { { PARAM_STRING, PARAM_RELOAD, &my_csexpire } } }
				};
				Directive confvalues2[] = {
					{ "CSMaxReg", { { PARAM_STRING, PARAM_RELOAD, &my_csregmax } } }
				};

				Directive confvalues3[] = {
					{ "CSAccessMax", { { PARAM_STRING, PARAM_RELOAD, &my_csaccessmax } } }      
				};

				Directive confvalues4[] = {
					{ "CSAutoKickMax", { { PARAM_STRING, PARAM_RELOAD, &my_csakickmax } } }
				};

				Directive confvalues5[] = {
					{ "CSListOpersOnly", { { PARAM_SET, PARAM_RELOAD, &my_cslistopersonly } } }
				};

				Directive confvalues6[] = {
					{ "CSInhabit", { { PARAM_STRING, PARAM_RELOAD, &my_csinhabit } } }
				};

				Directive confvalues7[] = {
					{ "CSDefBantype", { { PARAM_STRING, PARAM_RELOAD, &my_csdefbantype } } }
				};

				Directive confvalues8[] = {
					{ "CSOpersOnly", { { PARAM_SET, PARAM_RELOAD, &my_csopersonly } } }
				};

				Directive confvalues9[] = {
					{ "CSListMax", { { PARAM_STRING, PARAM_RELOAD, &my_cslistmax } } }
				};

				Directive confvalues10[] = {
					{ "CSAutokickReason", { { PARAM_STRING, PARAM_RELOAD, &my_csautokickreason } } }
				};

				Directive confvalues11[] = {
					{ "ChanCoreModules", { { PARAM_STRING, PARAM_RELOAD, &my_chancoremodules } } }
				};

				moduleGetConfigDirective(confvalues);
				moduleGetConfigDirective(confvalues1);
				moduleGetConfigDirective(confvalues2);
				moduleGetConfigDirective(confvalues3);
				moduleGetConfigDirective(confvalues4);
				moduleGetConfigDirective(confvalues5);
				moduleGetConfigDirective(confvalues6);
				moduleGetConfigDirective(confvalues7);
				moduleGetConfigDirective(confvalues8);
				moduleGetConfigDirective(confvalues9);
				moduleGetConfigDirective(confvalues10);
				moduleGetConfigDirective(confvalues11);

				notice(s_ChanServ, u->nick, " - ");
				notice(s_ChanServ, u->nick, "%s Configuration - Modes:%s",s_ChanServ, ircd->chanservmode);
				notice(s_ChanServ, u->nick, "Chan Expires: %s             Registration Max: %s                 Access List Max: %s", my_csexpire, my_csregmax, my_csaccessmax);
				notice(s_ChanServ, u->nick, "Opers only: %s            Inhabit length: %s                             Default bantype: %s", (my_csopersonly ? "Enabled" : "Disabled" ), my_csinhabit, my_csdefbantype);
				notice(s_ChanServ, u->nick, "List for opers only: %s            Max entries per list: %s                 Akick List Max: %s", (my_cslistopersonly ? "Enabled" : "Disabled" ), my_cslistmax, my_csakickmax);
				notice(s_ChanServ, u->nick, "Default akick reason: %s", my_csautokickreason);

				if (is_oper(u)) {
					if (findModule("cs_getpass")) {
						notice(s_ChanServ, u->nick, "Chan GETPASS: %s", (my_csgetpass ? "Root Only" : "Admin" ));
					} else {
						notice(s_ChanServ, u->nick, "Chan GETPASS: Disabled");
					}
					notice(s_ChanServ, u->nick, " - ");
					notice(s_ChanServ, u->nick, "Core Modules: %s", my_chancoremodules);
				}
				notice(s_ChanServ, u->nick, " - ");
				notice(s_ChanServ, u->nick, "End of Configuration");

				free(my_csexpire);
				free(my_csregmax);
				free(my_csaccessmax);
				free(my_csinhabit);
				free(my_csdefbantype);
				free(my_cslistopersonly);
				free(my_cslistmax);
				free(my_csakickmax);
				free(my_csgetpass);
				free(my_chancoremodules);
				free(my_csautokickreason);

					return MOD_CONT;

}

int do_memoconf (User *u)
{

					char *my_msmaxmemos = NULL;
					char *my_memocoremodules = NULL;
					char *my_mssenddelay = NULL;

				Directive confvalues[] = {
					{ "MSMaxMemos", { { PARAM_STRING, PARAM_RELOAD, &my_msmaxmemos } } }
				};

				Directive confvalues1[] = {
				     { "MemoCoreModules", { { PARAM_STRING, PARAM_RELOAD, &my_memocoremodules } } }
			   };

				Directive confvalues2[] = {
				     { "MSSendDelay", { { PARAM_STRING, PARAM_RELOAD, &my_mssenddelay } } }
			   };

				moduleGetConfigDirective(confvalues);
				moduleGetConfigDirective(confvalues1);
				moduleGetConfigDirective(confvalues2);

					notice(s_MemoServ, u->nick, " - ");
					notice(s_MemoServ, u->nick, "%s Configuration - Modes:%s",s_MemoServ, ircd->memoservmode);
					notice(s_MemoServ, u->nick, "Max Memos: %s         Send Delay: %s", my_msmaxmemos, my_mssenddelay);

					if (is_oper(u)) {
						notice(s_MemoServ, u->nick, "Core Modules: %s", my_memocoremodules);
					}
					notice(s_MemoServ, u->nick, " - ");
					notice(s_MemoServ, u->nick, "End of Configuration");

				free(my_msmaxmemos);
				free(my_memocoremodules);
				free(my_mssenddelay);

					return MOD_CONT;

}

int do_hostconf (User *u)
{

						char *my_hostcoremodules = NULL;
						char *my_hostsetters = NULL;

				Directive confvalues[] = {
					{ "HostCoreModules", { { PARAM_STRING, PARAM_RELOAD, &my_hostcoremodules } } }
				};
				Directive confvalues1[] = {
					{ "HostSetters", { { PARAM_STRING, PARAM_RELOAD, &my_hostsetters } } }
				};

				moduleGetConfigDirective(confvalues);
				moduleGetConfigDirective(confvalues1);

						notice(s_HostServ, u->nick, " - ");
						notice(s_HostServ, u->nick, "%s Configuration - Modes:%s",s_HostServ, ircd->hostservmode);
						notice(s_HostServ, u->nick, "HostSetters: %s", (my_hostsetters ? my_hostsetters : "None" ));

						if (is_oper(u)) {
							notice(s_HostServ, u->nick, "Core Modules: %s", my_hostcoremodules);
						}
						notice(s_HostServ, u->nick, " - ");
						notice(s_HostServ, u->nick, "End of Configuration");

				free(my_hostcoremodules);
				free(my_hostsetters);

					return MOD_CONT;

}

int do_botconf (User *u)
{

						char *my_botcoremodules = NULL;
						char *my_bsminusers = NULL;
						char *my_bsbadwordsmax = NULL;
						char *my_bskeepdata = NULL;
						int my_bssmartjoin = 0;
						char *my_bsgentlebwreason = NULL;
						char *my_bscasesensitive = NULL;
						char *my_bsfantasycharacter = NULL;

				Directive confvalues[] = {
					{ "BotCoreModules", { { PARAM_STRING, PARAM_RELOAD, &my_botcoremodules } } }
				};

				Directive confvalues1[] = {
					{ "BSMinUsers", { { PARAM_STRING, PARAM_RELOAD, &my_bsminusers } } }
				};

				Directive confvalues2[] = {
					{ "BSBadWordsMax", { { PARAM_STRING, PARAM_RELOAD, &my_bsbadwordsmax } } }
				};

				Directive confvalues3[] = {
					{ "BSKeepData", { { PARAM_STRING, PARAM_RELOAD, &my_bskeepdata } } }
				};

				Directive confvalues4[] = {
					{ "BSSmartJoin", { { PARAM_SET, PARAM_RELOAD, &my_bssmartjoin } } }
				};

				Directive confvalues5[] = {
					{ "BSGentleBWReason", { { PARAM_STRING, PARAM_RELOAD, &my_bsgentlebwreason } } }
				};

				Directive confvalues6[] = {
					{ "BSCaseSensitive", { { PARAM_SET, PARAM_RELOAD, &my_bscasesensitive } } }
				};

				Directive confvalues7[] = {
					{ "BSFantasyCharacter", { { PARAM_STRING, PARAM_RELOAD, &my_bsfantasycharacter } } }
				};

				moduleGetConfigDirective(confvalues);
				moduleGetConfigDirective(confvalues1);
				moduleGetConfigDirective(confvalues2);
				moduleGetConfigDirective(confvalues3);
				moduleGetConfigDirective(confvalues4);
				moduleGetConfigDirective(confvalues5);
				moduleGetConfigDirective(confvalues6);
				moduleGetConfigDirective(confvalues7);

				notice(s_BotServ, u->nick, " - ");
				notice(s_BotServ, u->nick, "%s Configuration - Modes:%s",s_BotServ, ircd->botservmode);
				notice(s_BotServ, u->nick, "Minimum users on channel: %s          Max badwords: %s               Keep data: %s", my_bsminusers, my_bsbadwordsmax, my_bskeepdata);
				notice(s_BotServ, u->nick, "Smart join: %s Gentle reason: %s Case sensitive: %s", (my_bssmartjoin), (my_bsgentlebwreason ? my_bsgentlebwreason : "Disabled"), (my_bscasesensitive ? my_bscasesensitive : "Disabled"));
				notice(s_BotServ, u->nick, "Fantasy character: %s", my_bsfantasycharacter);

				if (is_oper(u)) {
					notice(s_BotServ, u->nick, "Core Modules: %s", my_botcoremodules);
				}
				notice(s_BotServ, u->nick, " - ");
				notice(s_BotServ, u->nick, "End of Configuration");

				free(my_botcoremodules);
				free(my_bsminusers);
				free(my_bsbadwordsmax);
				free(my_bskeepdata);
				free(my_bsgentlebwreason);
				free(my_bscasesensitive);
				free(my_bsfantasycharacter);

					return MOD_CONT;
}


int do_nickconf (User *u)
{
	int my_nsgetpass = 0;
	char *my_nsexpire = NULL;
	int my_allowgroupchange = 0;
	char *my_nicklen = NULL;
	char *my_nickregdely = NULL;
	char *my_nsregmax = NULL;
	int my_restrictopernicks = 0;
	char *my_nickcoremodules = NULL;
	char *my_nsresenddelay = NULL;
	char *my_nsrexpire = NULL;
	char *my_nsmasaliases = NULL;
	char *my_nsaccessmax = NULL;
	char *my_nsguestnickprefix = NULL;
	char *my_nsreleasetimeout = NULL;
	int my_nsallowimmed = 0;
	int my_nslistopersonly = 0;
	char *my_nslistmax = NULL;
	char *my_enforceruser = NULL;
	int *my_usesvshold = 0;

	Directive confvalues[] = {
		{ "NSRestrictGetPass", { { PARAM_SET, PARAM_RELOAD, &my_nsgetpass } } }
	};

	Directive confvalues1[] = {
		{ "NSExpire", { { PARAM_STRING, PARAM_RELOAD, &my_nsexpire } } }
	};

	Directive confvalues2[] = {
		{ "NSNoGroupChange", { { PARAM_SET, PARAM_RELOAD, &my_allowgroupchange } } }      
	};

	Directive confvalues3[] = {
		{ "NickLen", { { PARAM_STRING, PARAM_RELOAD, &my_nicklen } } }
	};

	Directive confvalues4[] = {
		{ "RestrictOperNicks", { { PARAM_SET, PARAM_RELOAD, &my_restrictopernicks } } }
	};

	Directive confvalues5[] = {
		{ "NickRegDelay", { { PARAM_STRING, PARAM_RELOAD, &my_nickregdely } } }
	};

	Directive confvalues6[] = {
		{ "NSEmailMax", { { PARAM_STRING, PARAM_RELOAD, &my_nsregmax } } }
	};

	Directive confvalues7[] = {
		{ "NickCoreModules", { { PARAM_STRING, PARAM_RELOAD, &my_nickcoremodules } } }
	};

	Directive confvalues8[] = {
		{ "NSResendDelay", { { PARAM_STRING, PARAM_RELOAD, &my_nsresenddelay } } }
	};

	Directive confvalues9[] = {
		{ "NSRExpire", { { PARAM_STRING, PARAM_RELOAD, &my_nsrexpire } } }
	};

	Directive confvalues10[] = {
		{ "NSMaxAliases", { { PARAM_STRING, PARAM_RELOAD, &my_nsmasaliases } } }
	};

	Directive confvalues11[] = {
		{ "NSAccessMax", { { PARAM_STRING, PARAM_RELOAD, &my_nsaccessmax } } }
	};

	Directive confvalues12[] = {
		{ "NSGuestNickPrefix", { { PARAM_STRING, PARAM_RELOAD, &my_nsguestnickprefix } } }
	};

	Directive confvalues13[] = {
		{ "NSReleaseTimeout", { { PARAM_STRING, PARAM_RELOAD, &my_nsreleasetimeout } } }
	};

	Directive confvalues14[] = {
		{ "NSAllowKillImmed", { { PARAM_SET, PARAM_RELOAD, &my_nsallowimmed } } }
	};

	Directive confvalues15[] = {
		{ "NSListOpersOnly", { { PARAM_SET, PARAM_RELOAD, &my_nslistopersonly } } }
	};
  
	Directive confvalues16[] = {
		{ "NSListMax", { { PARAM_STRING, PARAM_RELOAD, &my_nslistmax } } }
	};

	Directive confvalues17[] = {
		{ "NickCoreModules", { { PARAM_STRING, PARAM_RELOAD, &my_enforceruser } } }
	};

	Directive confvalues18[] = {
		{ "UseSVSHOLD", { { PARAM_SET, PARAM_RELOAD, &my_usesvshold } } }
	};

	moduleGetConfigDirective(confvalues);
	moduleGetConfigDirective(confvalues1);
	moduleGetConfigDirective(confvalues2);
	moduleGetConfigDirective(confvalues3);
	moduleGetConfigDirective(confvalues4);
	moduleGetConfigDirective(confvalues5);
	moduleGetConfigDirective(confvalues6);
	moduleGetConfigDirective(confvalues7);
	moduleGetConfigDirective(confvalues8);
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

	notice(s_NickServ, u->nick, " - ");
	notice(s_NickServ, u->nick, "%s Configuration - Modes:%s",s_NickServ, ircd->nickservmode);
	notice(s_NickServ, u->nick, "Allow Group change: %s    Nick Expires: %s           Max Nick Length: %s", (my_allowgroupchange ? "Disabled" : "Enabled"), my_nsexpire, my_nicklen);
	notice(s_NickServ, u->nick, "Nick Registration Delay: %s         Max Nick Reg per email address: %s                  Restrict Oper Nicks: %s", my_nickregdely, my_nsregmax, (my_restrictopernicks ? "Enabled" : "Disabled"));
	notice(s_NickServ, u->nick, "Nick Resend Delay: %s     Stage 1 expires: %s      Max number of grouped nicks: %s", my_nsresenddelay, (my_nsrexpire ? my_nsrexpire : "Disabled" ), my_nsmasaliases);
	notice(s_NickServ, u->nick, "Access list max:%s            Guest prefix: %s                release Timeout: %s", my_nsaccessmax, my_nsguestnickprefix, my_nsreleasetimeout);
	notice(s_NickServ, u->nick, "Allow kill immediately: %s      List restricted to opers: %s      Max per list:%s", (my_nsallowimmed ? "Enabled": "Disabled" ), (my_nslistopersonly ? "Disabled" : "Enabled" ), my_nslistmax);

	if (is_oper(u)) {
		if (UseSVSHOLD) {
			notice(s_NickServ, u->nick, "Enforcer type: SVSHOLD");
		} else {
			notice(s_NickServ, u->nick, "Enforcer type: %s", my_enforceruser);
		}
		notice(s_NickServ, u->nick, "Core Modules: %s", my_nickcoremodules);
	}

	if (findModule("ns_getpass")) {	
		notice(s_NickServ, u->nick, "Nick GETPASS: %s", (my_nsgetpass ? "Root Only" : "Admin"));
	} else {
		notice(s_NickServ, u->nick, "Nick GETPASS: Disabled");
	}
	notice(s_NickServ, u->nick, " - ");
	notice(s_NickServ, u->nick, "End of Configuration");

	free(my_nsexpire);
	free(my_nicklen);
	free(my_nickregdely);
	free(my_nsregmax);
	free(my_nickcoremodules);
	free(my_nsresenddelay);
	free(my_nsrexpire);
	free(my_nsmasaliases);
	free(my_nsaccessmax);
	free(my_nsguestnickprefix);
	free(my_nsreleasetimeout);
	free(my_nslistmax);
	free(my_enforceruser);
	
	return MOD_CONT;
}

/* EOF */
