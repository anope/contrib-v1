/**
 * Misc channel commands the module adds - Headers
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Absurd-IRC.net>
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
 * Last Updated   : 12/10/2006
 *
 **/

/* Functions */
#ifdef ENABLE_TOPIC
int set_topic(User * u, Channel *c, char *topic);
#endif

#ifdef ENABLE_APPENDTOPIC
int append_to_topic(User * u, Channel *c, char *newtopic);
#endif

#ifdef ENABLE_INVITE
int do_invite(User * u, Channel *c, char *nick);
#endif

#ifdef ENABLE_BAN
int do_ban(User * u, Channel *c, char *params);
#endif

#ifdef ENABLE_MUTEUNMUTE
int do_mute(User * u, Channel *c, char *params);
int do_unmute(User * u, Channel *c, char *params);
#endif

#ifdef ENABLE_KICKBAN
int do_core_kickban(User * u, Channel *c, char *target, char *reason);
#endif

/* EOF */
