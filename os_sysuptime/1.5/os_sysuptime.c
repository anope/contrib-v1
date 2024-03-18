/***************************************************************************************/
/* Anope Module : os_sysuptime.c : v1.4                                                */  
/* Scott Seufert (katsklaw@ircmojo.net)                                                */
/*                                                                                     */
/* Proxy module for Trystan, DO NOT CONTACT TRYSTAN FOR SUPPORT, YOU WILL BE IGNORED!  */
/*                                                                                     */
/* Anope (c) 2000-2006 Anope.org                                                       */
/*                                                                                     */
/* v1.0 - Rob's module                                                                 */
/* v1.1 - Joris patches to work on OpenBSD                                             */
/* v1.2 - Rob made the output string a define for easier translation                   */
/* v1.3 - Trystan does....                                                             */
/*      - Makes it work on FreeBSD systems without linux binary support                */
/*      - Adds Windows support (note GetTickCount - is screwed on MS side)             */
/*      - Adds thrid party flag                                                        */
/*      - Adds AnopeFini so it will compile under windows                              */
/*      - Adds translation system                                                      */
/*      - Adds help to the module                                                      */
/*      - Changed variable in my_do_sysuptime() to prevent compiler complaining        */
/*      - Should now work on NetBSD and DragonFlyBSD as well                           */
/* v1.4 - Fixed compiler error on FreeBSD reported by Fudge                            */
/*      - Module_Help_OPERSERV_SYSUPTIME_FULL() now uses language table                */
/*      - Module_Help_OPERSERV_SYSUPTIME_FULL() sends from OperServ not HelpServ       */
/*      - Add check of command add status to make sure we can move forward             */
/*      - Declared AnopeFini() and AnopeInit() to avoid compiler warnings              */
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

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#include "module.h"

#define AUTHOR "katsklaw"
#define VERSION "1.5"
#define MYNAME "os_sysuptime"

#define LANG_NUM_STRINGS    	    7
#define SYSUPTIME_HOST 				0
#define SYSUPTIME_NO_UPTIME         1
#define SYSUPTIME_SHORT_HELP        2
#define SYSUPTIME_SYNTAX            3
#define SYSUPTIME_HELP              4
#define SYSUPTIME_FORMAT            5
#define SYSUPTIME_HELP_FULL         6

int my_do_sysuptime(User *u);
char *my_sysuptime(User *u);
void mAddLanguages(void);
int Module_Help_OPERSERV_SYSUPTIME_FULL(User *u);
void Module_Help_OPERSERV_SYSUPTIME(User *u);
int AnopeInit(int argc, char **argv);
void AnopeFini(void);

/*************************************************************************/

int AnopeInit(int argc, char **argv) 
{
	Command *c;
    int status = 0;       /* status from adding stuff */

	c=createCommand("sysuptime",my_do_sysuptime,is_services_admin,-1,-1,-1,-1,-1);
	status = moduleAddCommand(OPERSERV,c,MOD_HEAD);
    if (status != MOD_ERR_OK) {
        /* something went wrong say something about */
        alog("[%s%s] unable to bind to command SYSUPTIME error [%d]", MYNAME, MODULE_EXT, status);
        return MOD_STOP;
    } else {
    	alog("[%s%s]: New Command /msg operserv sysuptime - Status: %d", MYNAME, MODULE_EXT, status);
    }

    moduleAddHelp(c, Module_Help_OPERSERV_SYSUPTIME_FULL);
    moduleSetHelpHelp(Module_Help_OPERSERV_SYSUPTIME);

	mAddLanguages();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
    moduleSetType(THIRD);          /* Third party module         */

	return MOD_CONT;
}

/*************************************************************************/

void AnopeFini(void)
{
    alog("[%s%s] Unloaded successfully", MYNAME, MODULE_EXT);
}

/*************************************************************************/

void Module_Help_OPERSERV_SYSUPTIME(User *u)
{
   moduleNoticeLang(s_OperServ, u, SYSUPTIME_SHORT_HELP);
   return;
}

/*************************************************************************/

int Module_Help_OPERSERV_SYSUPTIME_FULL(User *u)
{
        moduleNoticeLang(s_OperServ, u, SYSUPTIME_HELP_FULL);
        return MOD_CONT;
}

/*************************************************************************/

int my_do_sysuptime(User *u) 
{
	char *mytime = NULL;

	if(u) {
		mytime = my_sysuptime(u);
		if(mytime) {
			moduleNoticeLang(s_OperServ, u, SYSUPTIME_HOST, mytime);
		} else {
			moduleNoticeLang(s_OperServ, u, SYSUPTIME_NO_UPTIME);
		}
	}
	return MOD_CONT;
}

/*************************************************************************/

char *my_sysuptime(User *u) 
{
#ifndef _WIN32
	int seconds;
#else
	DWORD seconds;
#endif
	int days = 0, hours = 0, minutes = 0;
	static char tmp[255];
	char *timeformat;

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
	int mib[2];
	size_t size;
	time_t now;
	struct timeval boottime;
#if defined(__FreeBSD__)
	FILE *fp;
#endif
#else
	FILE *fp;
#endif

#ifndef _WIN32
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
#if defined(__FreeBSD__)
    /* FreeBSD can have this just need the linux binary so this gives a fall back 
       to using the other code 
    */
	fp = fopen("/proc/uptime","r");
	if(fp != NULL ) {
		fscanf(fp,"%d",&seconds);
		fclose(fp);
	} else {
#endif
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);
	if (sysctl(mib, 2, &boottime, &size, NULL, 0) < 0)
		return (NULL);
	time(&now);
	seconds = now - boottime.tv_sec;
	seconds += 30;
#if defined(__FreeBSD__)
	}
#endif

#else	
	fp = fopen("/proc/uptime","r");
	if( fp == NULL ) {
		return NULL;
	}
	fscanf(fp,"%d",&seconds);
	fclose(fp);
#endif


#else
	seconds = GetTickCount();
#endif


	days = seconds / 86400;
	seconds -= (days * 86400);
	hours = seconds / 3600;
	seconds -= (hours * 3600);
	minutes = seconds / 60;

	timeformat = moduleGetLangString(u, SYSUPTIME_FORMAT);
	snprintf(tmp, sizeof(tmp), timeformat, days, hours, minutes);
	return tmp;
}

/*************************************************************************/

void mAddLanguages(void)
{
    char *langtable_en_us[] = {
        /* SYSUPTIME_HOST */
        "Host OS uptime: [%s]",
		/* SYSUPTIME_NO_UPTIME */
		"Module unable to get system uptime",
		/* SYSUPTIME_SHORT_HELP */
		"    SYSUPTIME    Display system uptime.",
		/* SYSUPTIME_SYNTAX */
		" Syntax: SYSUPTIME",
		/* SYSUPTIME_HELP */
		" Displays the systems uptime",
		/* SYSUPTIME_FORMAT */
		"\2%d\2 days \2%d\2 hours \2%d\2 minutes",
        /* SYSUPTIME_HELP_FULL */
        " Syntax: SYSUPTIME\n"
        "   \n"
        " Displays the systems uptime",
    };

    /* insert the english lang */
    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/*************************************************************************/



