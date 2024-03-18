/*
	Module name: os_logjpk (log join part kick)
	Version: 1.0
	Purpose: log join, part, kick, and topic change to a specified channel.
	Author: RoBerT or GhosT
	Date: 24 Aug 2006
	Tested with: Unreal3.2.4, Anope1.7.14
	License: Under the terms of GNU General Public License as published by the Free Software Foundation
	
	Credit: os_logjoinpart.c : v1.0 by Trystan Scott Lee
*/

/* NOTE: This is my first module ;) Any comment or make it better is welcome ;)*/

#include "module.h"
#define AUTHOR "Rb"
#define VERSION "1.0"
#define MYLOGCHAN "#EC"

int mEventJoin(int argc, char **argv);
int mEventPart(int argc, char **argv);
int mEventKick(int argc, char **argv);
int mEventTopic(int argc, char **argv);

void my_alog(const char *fmt, ...);

int AnopeInit(int argc, char **argv) 
{
    int status = 0;
    EvtHook *hook = NULL;

    hook = createEventHook(EVENT_JOIN_CHANNEL, mEventJoin);
    status = moduleAddEventHook(hook);
    
    hook = createEventHook(EVENT_PART_CHANNEL, mEventPart);
    status = moduleAddEventHook(hook);
    
    hook = createEventHook(EVENT_CHAN_KICK, mEventKick);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_TOPIC_UPDATED, mEventTopic);
    status = moduleAddEventHook(hook);
    
    alog("Loading os_jpk.so");
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void AnopeFini(void) {
  alog("Unloading os_logjpk.so");
}

/* JOIN */
int mEventJoin(int argc, char **argv) {
    if (argc != 3)
        return MOD_CONT;
    if (strcmp(argv[0], EVENT_STOP) == 0) {
		my_alog("%s joins %s", argv[1], argv[2]);
    }
	return MOD_CONT;
}
/* PART */
int mEventPart(int argc, char **argv) {
    if (argc != 3)
        return MOD_CONT;
    if (strcmp(argv[0], EVENT_STOP) == 0) {
		my_alog("%s parts %s", argv[1], argv[2]);
    }
	return MOD_CONT;
}

/* KICK */
int mEventKick(int argc, char **argv) {
	my_alog("%s was kicked from %s", argv[0], argv[1]);
	return MOD_CONT;
}

/* Topic */
int mEventTopic(int argc, char **argv) {
	my_alog("Topic %s was changed", argv[0], argv[1]);
	return MOD_CONT;
}

void my_alog(const char *fmt, ...)
{
    char str[BUFSIZE];

    va_list args;
    va_start(args, fmt);

    vsnprintf(str, sizeof(str), fmt, args);
    privmsg(s_GlobalNoticer, MYLOGCHAN, str);
}