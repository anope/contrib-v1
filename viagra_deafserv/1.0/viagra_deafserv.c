#include "module.h"
#define AUTHOR "OvErRiTe"
#define VERSION "1.0"



  /* -----------------------------------------------------------
   * Name  : viagra_deafserv
   * Author: OvErRite
   * Date  : 19/01/2004
   * -----------------------------------------------------------
   *
   * Description: This module will check the version of connecting
   * clients and apply umode D to clients that match the criteria.
   * Umode D (Deaf) is used to keep bandwidth down by preventing
   * channel messages from reaching bots that dont need them.
   *
   * Functions: my_privmsgA, my_nickA, ctcp_check, apply_mode,
   *            timeout_version_user, CheckVersionReply
   *
   * Limitations: Anope1.5.6-r30 and later only. + Viagra Beta 7 and later only
   * Tested: Viagra
   * -----------------------------------------------------------
   * This version has been tested on Viagra
   *
   *  This modules has configurable options
   *  Please read the comments by each one.
   *
   *  Version Changes:
   *
   *  1: DEAFServ created using SGR's ctcpserv.
   */

/* ---------------------------------------------------------------------- */
/* START OF CONFIGURATION BLOCK - please read the comments :)             */
/* ---------------------------------------------------------------------- */

 /****************************************
       FOR THE BELOW SETTINGS **ONLY**
      * CHANGE WHAT IS **INSIDE** THE
      * "Speach marks".
  ****************************************/
/* Set these to be the Nick, Host and Realname of the CTCPServ Psudo
 * client, as per your whims.
 */
char *s_DEAFServ = "DEAFServ";  /* Nickname - NICKLEN character MAXIMUM */
char *s_DEAFServHOST = "DEAFServ.Network.Service";  /* HostMask: 64 character MAXIMUM */
char *s_DEAFServREALNAME = "VERSION Checker";         /* Realname 30 character MAXIMUM */

/* Time to wait before CTCPing users who have joined the network - MANDATORY
 * Note that an integer followed by a letter == the time.
 * e.g: 3s = 3 seconds;  3m = 3 mins; 3h = 3 hours; 3d = 3 days.
 * set to a sensible time, e.g 3s to 30s [NOT less than 1s] */
char *TIME_TO_WAIT_BEFORE_CTCP = "2s";

/* Below are the blocks that are detectable for version matches.
These blocks can be added in the check routine with minimal coding
knowledge if 5 blocks is not enough.*/
/* If you dont want to use a block then leave it as ""; */
char *versionmatch1 = "iroffer";
char *versionmatch2 = "";
char *versionmatch3 = "";
char *versionmatch4 = "";
char *versionmatch5 = "";

 /******************************************
       FOR THE BELOW SETTINGS **ONLY**
       CHANGE THE INTEGER BETWEEN THE
       EQUALS SIGN (=) AND SEMI-COLON (;).
  ******************************************/

/* On load status - should I automatically start checking clients?
 * SET CHECK [ALL | ON | OFF ]
 *             2     1    0
 * Diff between ON and ALL - when ON only clients who don't ID to
 * NickServ or /Oper up in TIME_TO_WAIT_BEFORE_CTCP will be checked
 *
 * This setting MUST be 0, 1 or 2
 */
int cDEFAULTMODE = 2;

/* on load logging status
 * SET LOGGING [OFF|BASIC|USEFUL|VERBOSE|FULL]"
 *               0    1     2      3      4
 * 0 - Off
 * 1 - Every Action
 * 2 - Every Match + Every Action + Every FAIL
 * 3 - Every New User Versioned + Every Match + Every Action
 * 4 - Detected User + Every New User Versioned + Every Match + Every Action
 *
 * This setting MUST be 0, 1, 2, 3 or 4
 */
int cDEFAULTLOGLEVEL = 0;

/* ---------------------------------------------------------------------- */
/* DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING         */
/* ---------------------------------------------------------------------- */







int CTCPid;
int my_privmsgA(char *source, int ac, char **av);
int my_nickA(char *source, int ac, char **av);
int ctcp_check(User * u, char *buf);
int timeout_version_user(int argc, char **argv);
int apply_mode(User *u);
int CheckVersionReply(char *to_be_matched);
int donotchecktrue = 0;
int CTCPServLOGLEV = 0;
int AnopeInit(int argc, char **argv)
{
    Message *msg = NULL;
    int status;
    msg = createMessage("PRIVMSG", my_privmsgA);
    status = moduleAddMessage(msg, MOD_HEAD);
    msg = createMessage("NOTICE", my_privmsgA);
    status = moduleAddMessage(msg, MOD_HEAD);
    #if defined(IRC_ULTIMATE3)
    msg = createMessage("CLIENT", my_nickA);
    #else
    msg = createMessage("NICK", my_nickA);
    #endif
    status = moduleAddMessage(msg, MOD_TAIL);
    if (status == MOD_ERR_OK) {
        NEWNICK(s_DEAFServ, s_DEAFServ, s_DEAFServHOST, s_DEAFServREALNAME, "+ioS", 1);
    }
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    donotchecktrue = cDEFAULTMODE;
    CTCPServLOGLEV = cDEFAULTLOGLEVEL;
    alog("[ircd_ctcpserv] This module has loaded and is now active.");
    alog("[ircd_ctcpserv] For information see /msg %s HELP", s_DEAFServ);
    if (!cDEFAULTMODE) {
         alog("[ircd_ctcpserv] %s Protection DISABLED. To activate protection now, use: /msg %s SET CHECK ON", s_DEAFServ, s_DEAFServ);
    }
    if (cDEFAULTMODE) {
        if (cDEFAULTMODE > 1) {
         alog("[ircd_ctcpserv] %s - Protection activated for ALL clients.", s_DEAFServ);
        }
        if (cDEFAULTMODE == 1) {
         alog("[ircd_ctcpserv] %s - Protection activated.", s_DEAFServ);
        }
    }
    if (!cDEFAULTLOGLEVEL) {
         alog("[ircd_ctcpserv] %s - Logging set to OFF. Use: /msg %s SET LOGGING ON - to enable.", s_DEAFServ, s_DEAFServ);
    }
    if (cDEFAULTLOGLEVEL == 1) {
         alog("[ircd_ctcpserv] %s - Logging set to BASIC.", s_DEAFServ);
    }
    if (cDEFAULTLOGLEVEL == 2) {
         alog("[ircd_ctcpserv] %s - Logging set to USEFUL.", s_DEAFServ);
    }
    if (cDEFAULTLOGLEVEL == 3) {
         alog("[ircd_ctcpserv] %s - Logging set to VERBOSE.", s_DEAFServ);
    }
    if (cDEFAULTLOGLEVEL == 4) {
         alog("[ircd_ctcpserv] %s - Logging set to FULL.", s_DEAFServ);
    }
    return MOD_CONT;
}

void AnopeFini(void)
{
    send_cmd(s_DEAFServ, "QUIT :Module Unloaded!");
}

int my_privmsgA(char *source, int ac, char **msg)
{

/* an uglier version of Rob's CatServ module code */
    User *u;
    char *s;
    /* First, some basic checks */
    if (ac != 2) { /* bleh */
        return MOD_CONT;
    }
    if (!(u = finduser(source))) {
        return MOD_CONT;
    }                           /* non-user source */
    if (*msg[0] == '#') {
        return MOD_CONT;
    }
    /* Channel message */
    s = strchr(msg[0], '@');
    if (s) {
        *s++ = 0;
        if (stricmp(s, ServerName) != 0) {
            return MOD_CONT;
        }
    }
    if ((stricmp(msg[0], s_DEAFServ)) == 0) {     /* its for US! */
        ctcp_check(u, msg[1]);
        return MOD_STOP;
    } else {                    /* ok it isnt us, let the old code have it */
        return MOD_CONT;
    }
}

/*****************************************************************************/

int ctcp_check(User * u, char *buf)
{
    char *isversion = strtok(buf, " ");
    char *s;
    char *t;

  if (!isversion) {
     return MOD_STOP;
  }
  if (stricmp(isversion, "\1PING") == 0) {
     if (!(s = strtok(NULL, ""))) {
         s = "\1";
     }
     notice(s_DEAFServ, u->nick, "\1PING %s", s);
     return MOD_STOP;
  }
  if (skeleton) {
      notice_lang(s_DEAFServ, u, SERVICE_OFFLINE, s_DEAFServ);
      return MOD_STOP;
  }
  if (CTCPServLOGLEV > 3) {
         alog("[ircd_deafserv] - Got a message - starts: %s", isversion);
  }
  if (stricmp(isversion, "\1VERSION") == 0) {
      if (CTCPServLOGLEV > 3) {
          alog("[ircd_deafserv] - Its a VERSION!");
      }
      if (donotchecktrue) {
          if (!(s = strtok(NULL, ""))) {
                s = "\1";
          }

		  if (CTCPServLOGLEV > 3) {
				  alog("[ircd_deafserv] - Checking | %s | against def lists.", s);
			  }
		  if (CheckVersionReply(s) == 1) {
			  apply_mode(u);
			  return MOD_STOP;
		  }
  	  }

  }
  if (stricmp(isversion, "HELP") == 0) {
      s = strtok(NULL, " ");
      if (!s) {
          notice(s_DEAFServ, u->nick, "-----------------------------------------------------------------------");
          notice(s_DEAFServ, u->nick, "%s checks users CTCP versions as they join the network.", s_DEAFServ);
          notice(s_DEAFServ, u->nick, "This is done so we can quickly remove mass-spam or clone bots.");
          notice(s_DEAFServ, u->nick, "If you match a forbidden client you may be forcefully removed");
          notice(s_DEAFServ, u->nick, "from the network. For more information join the official help");
          notice(s_DEAFServ, u->nick, "room.");
          if (is_oper(u)) {
              notice(s_DEAFServ, u->nick, "   ");
              notice(s_DEAFServ, u->nick, "The module version is %s", VERSION);
              notice(s_DEAFServ, u->nick, "   ");
              notice(s_DEAFServ, u->nick, "New defintions should be added to custom.dat");
              notice(s_DEAFServ, u->nick, "Please remember to NOT modify the official.dat file - ");
              notice(s_DEAFServ, u->nick, "and to only add your own 'definitions' to the custom.dat");
              notice(s_DEAFServ, u->nick, "file; offical.dat may get updated, check the anope forums.");
              notice(s_DEAFServ, u->nick, "This file [custom.dat] can be updated \"on the fly\" so no");
              notice(s_DEAFServ, u->nick, "'reshashing' is needed. Please ensure there are NO blank lines.");
              notice(s_DEAFServ, u->nick, "in this file.");
              notice(s_DEAFServ, u->nick, "   ");
              notice(s_DEAFServ, u->nick, "For help on setting %s options see /msg %s HELP SET", s_DEAFServ,
	                                  s_DEAFServ);
          }
          notice(s_DEAFServ, u->nick, "-----------------------------------------------------------------------");
          return MOD_CONT;
	  }
          if (stricmp(s, "SET") == 0) {
              notice(s_DEAFServ, u->nick, "-----------------------------------------------------------------------");
              notice(s_DEAFServ, u->nick, "Syntax: SET CHECK [ALL|ON|OFF]");
              notice(s_DEAFServ, u->nick, "Syntax: SET LOGGING [OFF|BASIC|USEFUL|VERBOSE|FULL]");
              notice(s_DEAFServ, u->nick, "   ");
              notice(s_DEAFServ, u->nick, "SET CHECK:   Set if the CTCP VERSION on connect is actually used. This");
              notice(s_DEAFServ, u->nick, "             command is designed to stop the checks for a short time");
              notice(s_DEAFServ, u->nick, "             without a SRA having to unload the module. ALL means that");
              notice(s_DEAFServ, u->nick, "             ALL clients will be checked, ON means only non-ID'd and");
              notice(s_DEAFServ, u->nick, "             non-opered client will be checked (after timeout) and OFF");
              notice(s_DEAFServ, u->nick, "             means no clients will be checked at all.");
              notice(s_DEAFServ, u->nick, "   ");
              notice(s_DEAFServ, u->nick, "SET LOGGING: This command sets how verbose %s's logging is. When set to", s_DEAFServ);
              notice(s_DEAFServ, u->nick, "             OFF, there is no logging what-so-ever. When set to BASIC, logs");
              notice(s_DEAFServ, u->nick, "             of clients who ilicit ACTION are made to the logchan.");
              notice(s_DEAFServ, u->nick, "             When set to USEFUL, CTCPServ will send logchan messages of");
              notice(s_DEAFServ, u->nick, "             the definition matched and the action ensuing it. VERBOSE");
              notice(s_DEAFServ, u->nick, "             puts all relevant info into the logchan, and finally FULL");                  		  notice(s_DEAFServ, u->nick, "             is used for debugging purposes. ");
              notice(s_DEAFServ, u->nick, "   ");
              notice(s_DEAFServ, u->nick, "NOTE: If this module is unloaded these settings are lost.");
              notice(s_DEAFServ, u->nick, "-----------------------------------------------------------------------");
              return MOD_CONT;
          }
      }
  if (stricmp(isversion, "SET") == 0) {
      if (!is_services_admin(u)) {
          return MOD_CONT;
      }
      s = strtok(NULL, " ");
      t = strtok(NULL, " ");
      if (!s || !t) {
          notice(s_DEAFServ, u->nick, "See \2/msg %s HELP SET\2 for more information.", s_DEAFServ);
          return MOD_CONT;
      }

      /* ------------
       * Checking INT's
       * ------------
       *  OFF = 0
       *  ON = 1
       *  ALL = 2
       * */
      if (stricmp(s, "CHECK") == 0) {
          if (stricmp(t, "OFF") == 0) {
	      donotchecktrue = 0;
              notice(s_DEAFServ, u->nick, "You sucessfully disabled CTCP VERSION checking.");
              wallops(s_DEAFServ,  "%s set %s on connect CTCP VERSION checking OFF.", u->nick, s_DEAFServ);
              return MOD_CONT;
          }
	  if (stricmp(t, "ON") == 0) {
	      donotchecktrue = 1;
              notice(s_DEAFServ, u->nick, "You sucessfully enabled CTCP VERSION checking.");
              wallops(s_DEAFServ,  "%s set %s on connect CTCP VERSION checking ON.", u->nick, s_DEAFServ);
              return MOD_CONT;
	  }
	  if (stricmp(t, "ALL") == 0) {
	      donotchecktrue = 2;
              notice(s_DEAFServ, u->nick, "You sucessfully enabled CTCP VERSION checking [ALL clients].");
              wallops(s_DEAFServ,  "%s set %s on connect CTCP VERSION checking ON [ALL clients].", u->nick, s_DEAFServ);
              return MOD_CONT;
	  }
	  else {
                notice(s_DEAFServ, u->nick, "See \2/msg %s HELP SET\2 for more information.", s_DEAFServ);
	  }
                  return MOD_CONT;
      }
	/* ----------
 	* Logging INT's
 	*-----------
 	* 0 Off
	* 1 Every Action
 	* 2 Every Match + Every Action + Every FAIL
 	* 3 Every New User Versioned + Every Match + Every Action
 	* 4 Detected User + Every New User Versioned + Every Match + Every Action
 	*/
      if (stricmp(s, "LOGGING") == 0) {
          if (stricmp(t, "OFF") == 0) {
	      CTCPServLOGLEV = 0;
              notice(s_DEAFServ, u->nick, "You sucessfully set %s logging to OFF.", s_DEAFServ);
              wallops(s_DEAFServ,  "%s set sucessfully set %s logging to OFF.", u->nick, s_DEAFServ);
              return MOD_CONT;
          }
	  if (stricmp(t, "BASIC") == 0) {
	      CTCPServLOGLEV = 1;
              notice(s_DEAFServ, u->nick, "You sucessfully set %s logging to BASIC.", s_DEAFServ);
              wallops(s_DEAFServ,  "%s set sucessfully set %s logging to BASIC.", u->nick, s_DEAFServ);
              return MOD_CONT;
	  }
	  if (stricmp(t, "USEFUL") == 0) {
	      CTCPServLOGLEV = 2;
              notice(s_DEAFServ, u->nick, "You sucessfully set %s logging to USEFUL.", s_DEAFServ);
              wallops(s_DEAFServ,  "%s set sucessfully set %s logging to USEFUL.", u->nick, s_DEAFServ);
              return MOD_CONT;
	  }
	  if (stricmp(t, "VERBOSE") == 0) {
	      CTCPServLOGLEV = 3;
              notice(s_DEAFServ, u->nick, "You sucessfully set %s logging to VERBOSE.", s_DEAFServ);
              wallops(s_DEAFServ,  "%s set sucessfully set %s logging to VERBOSE.", u->nick, s_DEAFServ);
              return MOD_CONT;
	  }
	  if (stricmp(t, "FULL") == 0) {
	      CTCPServLOGLEV = 4;
              notice(s_DEAFServ, u->nick, "You sucessfully set %s logging to FULL.", s_DEAFServ);
              wallops(s_DEAFServ,  "%s set sucessfully set %s logging to FULL.", u->nick, s_DEAFServ);
              return MOD_CONT;
	  }

     }
  }
  return MOD_CONT;
}

int my_nickA(char *source, int ac, char **av)
{
     char CTCPidx[16];
     char *argv[1];
     if (CTCPServLOGLEV > 3) {
         alog("[ircd_deafserv] - Hooked NICK/CLIENT Message");
     }
     if (ac < 2) {
        if (CTCPServLOGLEV > 2) {
            alog("CTCPServ [ircd_deafserv] ERROR - NICK/CLIENT MSG LACKS NECESSARY ARGUMENTS");
	}
        return MOD_CONT;
     }
     if (CTCPServLOGLEV > 3) {
         alog("[ircd_deafserv] - Found a new user :D");
     }
     #ifndef IRC_ULTIMATE3
     if (*source) {
         return MOD_CONT;
     }
     #endif
     if (CTCPServLOGLEV > 3) {
         alog("[ircd_deafserv] - av0 %s av1 %s av2 %s", av[0], av[1], av[2] );
     }
     argv[0] = sstrdup(av[0]);
     CTCPid = CTCPid + 1;
     snprintf(CTCPidx, sizeof(CTCPidx), "CTCP-%d", CTCPid);
     if (moduleAddCallback(CTCPidx, time(NULL)+dotime(TIME_TO_WAIT_BEFORE_CTCP), timeout_version_user,1,argv) != MOD_ERR_OK) {
         alog("CTCPServ [ircd_deafserv] --------- ERROR REPORT -------");
         alog("CTCPServ [ircd_deafserv] moduleAddCallback Failed. (Timeout setting for function");
         alog("CTCPServ [ircd_deafserv] int timeout_version_user; args: %s)", av[0]);
         alog("CTCPServ [ircd_deafserv] MOD_ERR_OK not returned. Please report this to the module developer");
         alog("CTCPServ [ircd_deafserv] ------- ERROR REPORT END------");
	 free(argv[0]);
         return MOD_CONT;
     }
     if (CTCPid > 2147483600) {
         CTCPid = 1;
     }
     free(argv[0]);
     return MOD_CONT;
}

int timeout_version_user(int argc, char **argv)
{
  User *u2;
  char *peep = argv[0];
  if (!argc) {
      return MOD_STOP;
  }
  if (finduser(peep)) {
      u2 = finduser(peep);
      if (donotchecktrue < 2 ) {
          if ((is_oper(u2)) || (nick_identified(u2))) {
              if (CTCPServLOGLEV > 3) {
	           alog("[ircd_deafserv] %s - Not being checked (is an Oper or NickServ Identifed)", u2->nick);
	      }
              return MOD_CONT;
          }
     }
     if (donotchecktrue) {
         if (CTCPServLOGLEV > 2) {
	     alog("[ircd_deafserv] Version Checking %s", u2->nick);
	 }
         send_cmd(s_DEAFServ, "PRIVMSG %s :\1VERSION\1", u2->nick);
     }
  }
  else {
         if (CTCPServLOGLEV > 1) {
             alog("[ircd_deafserv] ERROR - could not find %s [Perhaps they Quit]", peep);
	 }
  }
return MOD_CONT;
}

/*-----------------------------------------------*/

int CheckVersionReply(char *to_be_matched)
{
  if(versionmatch1 != ""){
	  if (stristr(to_be_matched, versionmatch1)){
		  if (CTCPServLOGLEV > 1) {
			alog("DEAF VERSION MATCH: %s +[MATCHED WITH]+ %s", to_be_matched, versionmatch1);
		  }
		  return 1;
	  }
  }

  if(versionmatch2 != ""){
	  if (stristr(to_be_matched, versionmatch2)){
		  if (CTCPServLOGLEV > 1) {
			alog("DEAF VERSION MATCH: %s +[MATCHED WITH]+ %s", to_be_matched, versionmatch2);
		  }
		  return 1;
	  }
  }
    if(versionmatch3 != ""){
  	  if (stristr(to_be_matched, versionmatch3)){
  		  if (CTCPServLOGLEV > 1) {
  			alog("DEAF VERSION MATCH: %s +[MATCHED WITH]+ %s", to_be_matched, versionmatch3);
  		  }
  		  return 1;
  	  }
  }
    if(versionmatch4 != ""){
  	  if (stristr(to_be_matched, versionmatch4)){
  		  if (CTCPServLOGLEV > 1) {
  			alog("DEAF VERSION MATCH: %s +[MATCHED WITH]+ %s", to_be_matched, versionmatch4);
  		  }
  		  return 1;
  	  }
  }
    if(versionmatch5 != ""){
  	  if (stristr(to_be_matched, versionmatch5)){
  		  if (CTCPServLOGLEV > 1) {
  			alog("DEAF VERSION MATCH: %s +[MATCHED WITH]+ %s", to_be_matched, versionmatch5);
  		  }
  		  return 1;
  	  }
  }


  return 0;
}

int apply_mode(User *u)
{
   char akillmask[BUFSIZE];

   if (CTCPServLOGLEV > 0) {
      alog("Deaf Client Match Detected: %s (%s)", u->nick, u->host);
   }

   send_cmd(NULL, "SVSMODE %s +D", u->nick);

   notice(s_DEAFServ, u->nick, "You client has been assigned Usermode D (Deaf).");
   notice(s_DEAFServ, u->nick, "If this was done in error please privmsg an operator.");

   return MOD_STOP;
}
