/**
 * Modules Main Functions for loading and matching commands. - Headers
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
 * Last Updated   : 21/06/2007
 *
 **/

#include "config.h"

#define AUTHOR "Viper"
#define VERSION "1.1.12"

/* Functions */
int do_fantasy(int ac, char **av);
int do_fantasy_denied(int ac, char **av);

/* Global Variables */
int excempt_nr, EnOperCmds, SAdminOverride, OverrideCoreCmds;
char **ListExempts;

/* EOF */
