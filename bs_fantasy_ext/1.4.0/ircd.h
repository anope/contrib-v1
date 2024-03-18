/**
 * Provides ircd specific logic - Headers
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

/* This is currently the same in all protocol modules so does not
 * not need ircd checking... */
#define CMODE_m 0x00000002

/* H - Hide IRCop Status (IRCop Only) - IRCd specific.. */
#define UMODE_H_unreal32  0x00080000
#define UMODE_H_solidircd 0x20000000
#define UMODE_H_ptlink    0x00000100
#define UMODE_H_insp12    0x00004000
#define UMODE_H_insp20    0x00004000

/* B - User is a Bot - IRCd specific.. */
#define UMODE_B_unreal32  0x00020000
#define UMODE_B_ptlink    0x00000080
#define UMODE_B_insp12    0x00001000
#define UMODE_B_insp20    0x00001000


/* Function */
int has_umode_H(User *u);
int has_umode_B(User *u);

/* EOF */
