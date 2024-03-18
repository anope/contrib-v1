/**
 * Modules Main Functions for loading and matching commands. - Headers
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
 * Last Updated   : 31/05/2013
 *
 **/

#include "config.h"

#define AUTHOR "Viper"
#define VERSION "1.4.1"

/* Functions */
int do_fantasy(int ac, char **av);
int do_fantasy_denied(int ac, char **av);

/* Global Variables */
int enabled, SHUNExpiry, IgnoreBots;
int excempt_nr, EnOperCmds, SAdminOverride, OverrideCoreCmds, RestrictKB;
char **ListExempts;
int en_sync, en_shun, en_f_dt, en_f_karma, en_f_vhost, en_why;

/* Constants */
#ifdef SUPPORTED
int supported = 1;
#else
int supported = 0;
#endif
char *PrevUnbanIP = "PrevUnbanIP";
char *NrUnbanIP = "NrUnbanIP";
int DefSHUNExpiry = 172800;			/* 48hrs (60.60.48) */

/* EOF */
