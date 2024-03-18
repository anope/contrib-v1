/*
 * ns_quote.c
 *
 *  Created on: 11 Feb 2013
 *      Author: denver
 *
 *  ns_quote derived from ns_fortune_cookie by SGR.
 *  As this is an *old* module, I have only tested it for Unreal3.2.10
 */


#include "module.h"
#define AUTHOR "SGR"
#define VERSION "1.00"

  /* ------------------------------------------------------------
   * Name  : ns_fortunecookie
   * Author: SGR <Alex_SGR@ntlworld.com>
   * Date  : 30/01/2004
   * ------------------------------------------------------------
   * Functions: m_ns_fortune_cookie, m_cookie_add, m_ns_do_cookie
   *            m_ns_identify_cookie.
   * Limitations: None Known.
   * Tested: Unreal(3.2), Viagra
   * ------------------------------------------------------------
   * This version has been tested on UnrealIRCd and Viagra.
   *
   * This module allows networks to have a "fortune cookie"
   * read to users as they identify or as they use NickServ's
   * FORTUNE command (added if the module if configured with it).
   *
   * An example 'Cookie File' can be obtained from SGR. However,
   * the format is quite simple: One line per cookie, max legnth
   * of a line is 420 characters. There should be no blank lines.
   *
   *  Thanks to dengel, Rob and Certus for all there support.
   * ------------------------------------------------------------
   */

/* ---------------------------------------------------------------------- */
/* START OF CONFIGURATION BLOCK - please read the comments :)             */
/* ---------------------------------------------------------------------- */

/* --> COOKIE_PREFIX [MANDATORY]
 * The below will be prefixed onto whatever random line is
 * pulled out of the cookie file. Please ensure this is
 * sensible and is not numerical only.
 */
#define COOKIE_PREFIX "Cookie:"

/* --> COOKIE_FILE [MANDATORY]
 * * This is the file from which to get our fortune cookies. If
 * ONLY a filename is given [i.e. without and /'s or \'s, I'll
 * look for this in the services DATA directory [Same place as
 * the services databases]. Otherwise, a _FULL_ path should be
 * specified.
 *
 * Its recommended you leave it as it is, and just create a
 * 'cookies.conf' file in the same dir as your services databases.
 * Then if you define ADD_COOKIE_COMMAND you can add new cookies
 * to this file from IRC.
 *
 * This file should be both READ and WRITEable by the user
 * services are run under.
 *
 * Please CREATE this file BEFORE loading this module, and ensure there
 * are at least 3 entries in this file. The max line legnth is 420
 * characters. No lines should be blank.
 */
#define COOKIE_FILE   "cookies.conf"

/* ---> ADD_COOKIE_COMMAND [Recommended]
 * Comment this out [add // to the beginning on the line] to disable
 * the 'cookie' command. This will cause NickServ to give users a
 * fortune cookie ONLY when they identify. If it _IS_ defined users
 * can request a cookie at anytime using /NickServ FORTUNE. [Recommended]
 */

//#define ADD_COOKIE_COMMAND

/* ---> COOKIE_ON_IDENTIFY [Recommended]
 * Comment this out [add // to the beginning on the line] to disable
 * the 'cookie' on identify. This will cause NickServ to give users a
 * fortune cookie ONLY when use the FORTUNE command. If it _IS_ defined
 * users will automatically get a cookie upon sucesfully identifying.
 */

#define COOKIE_ON_IDENTIFY

/* ---------------------------------------------------------------------- */
/* DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING         */
/* ---------------------------------------------------------------------- */

int m_ns_fortune_cookie(User *u);
int m_ns_identify_cookie(User *u);
int m_ns_do_cookie(User *u);
int m_cookie_add(User *u, char *cookie);
int SGR_Module_Help_NICKSERV_FORTUNE_FULL(User *u);
void SGR_Module_Help_NICKSERV_FORTUNE(User *u);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    FILE *cookiefile;
    #ifdef COOKIE_ON_IDENTIFY
    c=createCommand("ID",m_ns_identify_cookie,NULL,-1,-1,-1,-1,-1);
    moduleAddCommand(NICKSERV,c,MOD_TAIL);
    c=createCommand("IDENTIFY",m_ns_identify_cookie,NULL,-1,-1,-1,-1,-1);
    moduleAddCommand(NICKSERV,c,MOD_TAIL);
    #endif
    #ifdef ADD_COOKIE_COMMAND
    c=createCommand("FORTUNE",m_ns_fortune_cookie,NULL,-1,-1,-1,-1,-1);
    moduleAddCommand(NICKSERV,c,MOD_UNIQUE);
    moduleAddHelp(c,SGR_Module_Help_NICKSERV_FORTUNE_FULL);
    moduleSetNickHelp(SGR_Module_Help_NICKSERV_FORTUNE);
    #endif
    #if !defined(COOKIE_ON_IDENTIFY) && !defined(ADD_COOKIE_COMMAND)
    alog("[ns_fortunecookie] ERROR: Neither COOKIE_ON_IDENTIFY or ADD_COOKIE_COMMAND");
    alog("[ns_fortunecookie]        has been defined. This module is useless without");
    alog("[ns_fortunecookie]        at least one of those two settings.");
    alog("[ns_fortunecookie] Module Auto-Unloading.");
    return MOD_STOP;
    #endif
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    cookiefile = fopen(COOKIE_FILE, "r");
    if (!cookiefile) {
        alog("[ns_fortunecookie] ERROR: There doesn't seem to be a %s file", COOKIE_FILE);
        alog("[ns_fortunecookie]        (or there is and I can't access it).");
        alog("[ns_fortunecookie] Please create one and configure the module properly.");
        alog("[ns_fortunecookie] Module Auto-Unloading.");
        return MOD_STOP;
    }
    return MOD_CONT;
}

#ifdef COOKIE_ON_IDENTIFY
int m_ns_identify_cookie(User *u)
{
   if (nick_identified(u)) {
       m_ns_do_cookie(u);
   }
   return MOD_CONT;
}
#endif

#ifdef ADD_COOKIE_COMMAND
void SGR_Module_Help_NICKSERV_FORTUNE(User *u)
{
   notice(s_NickServ,u->nick, "    FORTUNE     See a random 'Fortune Cookie'.");
   return;
}

int SGR_Module_Help_NICKSERV_FORTUNE_FULL(User *u)
{
  if (is_services_admin(u)) {
      notice(s_NickServ, u->nick, "-----------------------------------------------------------------------");
      notice(s_NickServ, u->nick, "Syntax: FORTUNE [ADD|LIST|COUNT] [fortune] ");
      notice(s_NickServ, u->nick, "   ");
      notice(s_NickServ, u->nick, "The FORTUNE command (without any parameters) will show you a random fortune cookie.");
      notice(s_NickServ, u->nick, "fortune cookie%s",
      #ifdef COOKIE_ON_IDENTIFY
      ", just like the one you see when you identified");
      notice(s_NickServ, u->nick, "with %s", s_NickServ);
      #else
      ".");
      #endif
      notice(s_NickServ, u->nick, " ");
      notice(s_NickServ, u->nick, "If the COUNT parameter is provided, a report on the number of cookies");
      notice(s_NickServ, u->nick, "in the cookie file will be returned. If ADD is used, the fortune");
      notice(s_NickServ, u->nick, "provided will be added to the fortune database.");
      notice(s_NickServ, u->nick, "Finally, if LIST is provided, all fortunes in the fortune database will");
      notice(s_NickServ, u->nick, "be reported back.");
      notice(s_NickServ, u->nick, "-----------------------------------------------------------------------");
      return MOD_CONT;
  }
  notice(s_NickServ, u->nick, "-----------------------------------------------------------------------");
  notice(s_NickServ, u->nick, "Syntax: FORTUNE ");
  notice(s_NickServ, u->nick, "   ");
  notice(s_NickServ, u->nick, "The FORTUNE command will show you a random fortune cookie.");
  #ifdef COOKIE_ON_IDENTIFY
  notice(s_NickServ, u->nick, "Just like the one you see when you Identify with %s.", s_NickServ);
  #endif
  notice(s_NickServ, u->nick, "-----------------------------------------------------------------------");
  return MOD_CONT;
}

int m_ns_fortune_cookie(User *u)
{
   char *arg1 = strtok(NULL, " ");
   char *arg2 = strtok(NULL, "");
   if (!arg1) {
       m_ns_do_cookie(u);
       return MOD_CONT;
   }
   else {
         if ((!is_services_admin(u))) {
              notice(s_NickServ, u->nick, "Syntax: FORTUNE (no paramters)");
              return MOD_CONT;
         }
         else if (stricmp(arg1,"ADD")==0) {
                  if (!arg2) {
                      notice(s_NickServ, u->nick, "Syntax: FORTUNE ADD [some fortune here]");
                      return MOD_CONT;
                  }
	          else {
		        if ( ((strlen(arg2) > 9)) && ((strlen(arg2) < 421))) {
	                     m_cookie_add(u, arg2);
		        }
			else {
                              notice(s_NickServ, u->nick, "ERROR: Fortunes need to be between 10 and 420 characters long.");
			}
                        return MOD_CONT;
                   }
		   return MOD_CONT;
         }
         else if (stricmp(arg1,"LIST")==0) {
                  FILE *cookiefile = fopen(COOKIE_FILE,"r");
                  int  linenum =0;
		  int  len     =0;
                  char line[420];

                  if (!cookiefile) {
                      notice(s_NickServ, u->nick, "There doesn't seem to be a %s file.", COOKIE_FILE);
                      notice(s_NickServ, u->nick, "Please create one and configure the module with the correct path.");
		      return MOD_CONT;
                  }
		  else {
		        notice(s_NickServ, u->nick, "----------------------------------------------------------");
		        notice(s_NickServ, u->nick, "                   Fortune cookie list.                   ");
		        notice(s_NickServ, u->nick, "----------------------------------------------------------");
                        while (fgets(line, (sizeof(line) - 1), cookiefile) != NULL) {
                               len = strlen(line);
                               line[len-1] = '\0';
                               linenum++;
			       notice(s_NickServ, u->nick, "Fortune %d: %s", linenum, line);
                        }
		        notice(s_NickServ, u->nick, "----------------------------------------------------------");
	           }
		   fclose(cookiefile);
	           return MOD_CONT;
         }
	 else if (stricmp(arg1,"COUNT")==0) {
	          FILE *cookiefile = fopen(COOKIE_FILE,"r");
                  int  linenum =0;
		  char line[420];

		  if (!cookiefile) {
                      notice(s_NickServ, u->nick, "There doesn't seem to be a %s file.", COOKIE_FILE);
                      notice(s_NickServ, u->nick, "Please create one and configure the module with the correct path.");
		      return MOD_CONT;
                  }
	          while (fgets(line, (sizeof(line) - 1), cookiefile) != NULL) {
                         linenum++;
                  }
                  notice(s_NickServ, u->nick, "There are %d fortunes in the cookie file.", linenum);
		  fclose(cookiefile);
	          return MOD_CONT;
         }
	 else {
               notice(s_NickServ, u->nick, "Syntax: FORTUNE [ADD|LIST|COUNT]");
         }
         return MOD_CONT;
   }
   return MOD_CONT;
}

int m_cookie_add(User *u, char *cookie)
{
  FILE *cookiefile;

  if ((cookiefile = fopen(COOKIE_FILE,"a")) == NULL)  {
       notice(s_NickServ, u->nick, "There doesn't seem to be a %s file.", COOKIE_FILE);
       notice(s_NickServ, u->nick, "Please create one and configure the module with the correct path.");
       return MOD_CONT;
  }
  fprintf(cookiefile, "%s\n", cookie);
  notice(s_NickServ, u->nick, "\002Cookie Added:\002 %s", cookie);
  fclose (cookiefile);
  return MOD_CONT;
}
#endif

int m_ns_do_cookie(User *u)
{
  FILE *cookiefile;
  int  linenum =0;
  int  random  =0;
  int  len     =0;
  char line[420];

  srand (time (NULL) + getpid () + rand () % 9999);
  cookiefile = fopen(COOKIE_FILE, "r");
  if (!cookiefile) {
      return MOD_CONT;
  }
  while (fgets(line, (sizeof(line) - 1), cookiefile) != NULL) {
           len = strlen(line);
           line[len-1] = '\0';
           linenum++;
  }
  if (linenum == 0) {
      return MOD_CONT;
  }
  random = (rand () % linenum) - 1;
  fclose(cookiefile);
  cookiefile = fopen(COOKIE_FILE, "r");
  linenum = 0;
  while (fgets(line, (sizeof(line) - 1), cookiefile) != NULL) {
         len = strlen(line);
         line[len-1] = '\0';
         linenum++;
         if (linenum == random) {
             break;
         }
  }
  if (line != NULL) {
      notice(s_NickServ, u->nick, "\002%s\002 %s", COOKIE_PREFIX, line);
  }
  fclose(cookiefile);
  return MOD_CONT;
}
