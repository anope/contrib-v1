/**
 * -----------------------------------------------------------------------------
 * Name    : chanrecount
 * Author  : Viper <Viper@Anope.org>
 * Date    : 19/08/2011 (Last update: 19/08/2011)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Requires    : Anope-1.8.6
 * Tested      : Anope 1.8.6 GIT + InspIRCd 1.2.0
 * -----------------------------------------------------------------------------
 * This module is designed to address a desync issue in Anope 1.8 databases.
 *
 * Due to bugs in the database merge tool and some anope versions it is possible
 * that the counter tracking the number of channels a user has registered under
 * his account is no longer correct.
 * This module will reset the counter and update it based on the current
 * chanserv data.
 *
 * Use: When users encounter the following error
 *     -ChanServ- Sorry, you have already exceeded your limit of 40 channels.
 * without any obvious reason, load this module to reset the counters of all 
 * users and update them based on the current chanserv data.
 *
 * Note: This module must be loaded manually or through ModuleDelayedAutoload;
 *       NOT in ModuleAutoload or one of the CoreModules directives!!!
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *   1.0  -  Initial Release.
 *
 * -----------------------------------------------------------------------------
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.0"


/**
 * Fix the database and unload..
 * @param argc Argument count
 * @param argv Argument list
 * @return Always returns MOD_STOP as this module only needs to run once..
 **/
int AnopeInit(int argc, char **argv) {
	NickCore *nc = NULL;
	ChannelInfo *ci = NULL;
	int i, c;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002chanrecount\002] Loading module...");

	if (!moduleMinVersion(1,8,6,3071)) {
		alog("[\002chanrecount\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	/* Go over all nickcores and reset counter.. */
	c = 0;
	for (i = 0; i < 1024; i++) {
		for (nc = nclists[i]; nc; nc = nc->next) {
			nc->channelcount = 0;
			c++;
		}
	}
	alog("[chanrecount] Reset counter of %d nickgroups.", c);

	/* Go over all registered channels and update the founders counter.. */
	c = 0;
	for (i = 0; i < 256; i++) {
		for (ci = chanlists[i]; ci; ci = ci->next) {
			if ((ci->flags & CI_VERBOTEN) || !ci->founder)
				continue;

			ci->founder->channelcount++;
			c++;
		}
	}
	alog("[chanrecount] Counted %d channels..", c);

	alog("[\002chanrecount\002] Module updated database successfully....");
	wallops(s_OperServ, "Channelcount updated successfully. Auto-unloading module..");
	return MOD_STOP;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002chanrecount\002] Unloading module...");
}

/* EOF */
