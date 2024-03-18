/**
 * Provides ircd specific logic - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 04/04/2007
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
 * Last Updated   : 25/11/2011
 *
 **/

#include "ircd.h"

int has_umode_H(User *u) {
	if (!u)
		return 0;

	if (!stricmp(IRCDModule, "unreal32"))
		return u->mode & UMODE_H_unreal32;

	if (!stricmp(IRCDModule, "inspircd12"))
		return u->mode & UMODE_H_insp12;

	if (!stricmp(IRCDModule, "inspircd20"))
		return u->mode & UMODE_H_insp20;

	if (!stricmp(IRCDModule, "ptlink"))
		return u->mode & UMODE_H_ptlink;

	if (!stricmp(IRCDModule, "solidircd"))
		return u->mode & UMODE_H_solidircd;

	return 0;
}

int has_umode_B(User *u) {
	if (!u)
		return 0;

	if (!stricmp(IRCDModule, "unreal32"))
		return u->mode & UMODE_B_unreal32;

	if (!stricmp(IRCDModule, "inspircd12"))
		return u->mode & UMODE_B_insp12;

	if (!stricmp(IRCDModule, "inspircd20"))
		return u->mode & UMODE_B_insp20;

	if (!stricmp(IRCDModule, "ptlink"))
		return u->mode & UMODE_B_ptlink;

	return 0;
}

/* EOF */
