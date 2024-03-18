/**
 * Misc channel commands the module adds - Headers
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 21/07/2006
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated   : 03/09/2012
 *
 **/

/* Functions */
#ifdef ENABLE_TOPIC
int set_topic(User * u, Channel *c, char *topic);
#endif

#ifdef ENABLE_APPENDTOPIC
int append_to_topic(User * u, Channel *c, char *newtopic);
int topic_replace_append(User * u, Channel *c, char *newtopic);
#endif

#ifdef ENABLE_PREPENDTOPIC
int prepend_to_topic(User * u, Channel *c, char *newtopic);
int topic_replace_prepend(User * u, Channel *c, char *newtopic);
#endif

#ifdef ENABLE_INVITE
int do_invite(User * u, Channel *c, char *nick);
#endif

#ifdef ENABLE_KICKBAN
int do_core_kickban(User * u, Channel *c, char *target, char *reason);
#endif

#ifdef ENABLE_KICK
int do_core_kick(User * u, Channel *c, char *target, char *reason);
#endif

#ifdef ENABLE_SYNC
int do_sync(User *u, Channel *c);
#endif

/* EOF */
