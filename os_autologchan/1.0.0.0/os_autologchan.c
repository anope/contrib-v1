/*Turns LogChan on when anope starts
 *licensed under GNU GPL
 *
 * MrStonedOne @ irc.syntheticgeeks.com
 *
 */
/*************************************************************************/


#include "module.h"
#define AUTHOR "MrStonedOne"
#define VERSION "1.0.0"

int AnopeInit(int argc, char **argv)
{
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	alog("Now sending log messages to %s", LogChannel);
	logchan = 1;
	return MOD_CONT;
}

