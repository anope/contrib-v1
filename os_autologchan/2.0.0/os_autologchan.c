/*********************************************************************************************
 * Turns LogChan on when anope starts
 * licensed under GNU GPL
 *
 * MrStonedOne @ ScL-IRC kyleshome@gmail.com
 *
 *
 *                 *****Changed from 1.0.0 to 2.0.0:*****
 *
 * Made it windows friendly. *happy now?*
 * 
 * Checks join2msg for support on ircd's where Global has to be in the channel to msg.
 *
 * Checks if logchannel is defined and if not alogs a error msg.
 *
 * Hopefully will be QA ready
 */
/**********************************************************************************************/


#include "module.h"
#define AUTHOR "MrStonedOne"
#define VERSION "2.0.0"

int AnopeInit(int argc, char **argv)
{
	Channel *c;
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	/*do what /msg operserv set logchan on does.*/
	if (LogChannel) {
            if (ircd->join2msg) {
                c = findchan(LogChannel);
                anope_cmd_join(s_GlobalNoticer, LogChannel, c ? c->creation_time : time(NULL));
            }
		/*i couldn't figure out how to set logchan on without using a internal anope variable, so thats what i ended up doing.*/
		logchan = 1;
            alog("Now sending log messages to %s", LogChannel);
        } else {
            alog("OS_AutoLogChan: no log channel defined");
        }

	return MOD_CONT;
}
void AnopeFini(void) {
    /* nothing to do */
}
