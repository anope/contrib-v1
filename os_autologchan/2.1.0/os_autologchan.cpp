/*********************************************************************************************
 * Turns LogChan on when anope starts
 * licensed under GNU GPL
 *
 * MrStonedOne @ ScL-IRC kyleshome@gmail.com
 * 2.1.0 update by Syloq @ Nightstar
 *
 *
 *                 *****Changed from 2.0.0 to 2.1.0:*****
 *
 * Not a lot has changed but this version will work for 1.9.0_p1 at the very least.
 *
 * Most of this is a direct pull of information from the actual source of anope.
 *
 */
/**********************************************************************************************/


#include "module.h"
#define AUTHOR "Syloq"
#define VERSION "2.1.0"

class OSAutoLogChan : public Module
{
public:
        OSAutoLogChan(const std::string &modname, const std::string &creator) : Module(modname, creator)
        {
                Channel *c;
                this->SetAuthor(AUTHOR);
                this->SetVersion(VERSION);

                if (LogChannel) {
                        if (ircd->join2msg) {
                                c = findchan(LogChannel);
                                ircdproto->SendJoin(findbot(s_GlobalNoticer), LogChannel, c ? c->creation_time : time(NULL));
                        }
                        logchan = 1;
                        alog("Now sending log messages to %s", LogChannel);
                } else {
                        alog("os_autologchan: no log channel defined");
                }

        }
};

MODULE_INIT("os_autologchan",OSAutoLogChan)

/* EOF */

