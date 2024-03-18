/**
 * Module's Compile time Configuration Options
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
 * Last Updated   : 26/02/2007
 *
 **/


/**
 *	Undefine this if you disable a fantasy trigger or just comment a line out. Define it to enable
 *	a trigger. This was added on request and considering the anope core provides
 *	the ability to anable/disable certain commands by not loading some core modules it would
 *	be stupid if those commands were still accessible through the fantasy commands.
 *	Note that not all commands can be disabled here !
 *
 * This needs to be set at compile time and cannot be changed unless by recompiling the module.
 **/

/* Enable the !cmdlist trigger */
#define ENABLE_CMDLIST

/* Enable the !help trigger */
#define ENABLE_HELP

/* Enable the !clear trigger */
#define ENABLE_CLEAR

/* Enable the !xop trigger */
#define ENABLE_XOP

/* Enable the !access & !levels trigger */
#define ENABLE_ACCESS

/* Enable the !akick trigger */
#define ENABLE_AKICK

/* Enable the !badwords trigger */
#define ENABLE_BADWORDS

/* Enable the !set trigger  that gives access to both the chanserv and botserv set options*/
#define ENABLE_SET

/* Enable the !topic trigger */
#define ENABLE_TOPIC

/* Enable the !appendtopic trigger */
#define ENABLE_APPENDTOPIC

/* Enable the !invite trigger */
#define ENABLE_INVITE

/* Enable the !staff (!ircops) and !admin trigger */
#define ENABLE_STAFF

/* Enable the !up and !down triggers */
#define ENABLE_UPDOWN

/* Enable the !ban trigger */
#define ENABLE_BAN

/* Enable the !mute & !unmute trigger */
#define ENABLE_MUTEUNMUTE

/* Enable the !kb trigger implementation by this module to override the one in the core */
#define ENABLE_KICKBAN

/* Enable the !info trigger */
#define ENABLE_INFO

/* Enable the !kill trigger */
#define ENABLE_KILL

/* Enable the !mode trigger */
#define ENABLE_MODE

/* Enable the !akill trigger */
#define ENABLE_AKILL

/* Enable the !ignore trigger */
#define ENABLE_IGNORE

/* Enable the !bkick trigger to change botserv kick settings */
#define ENABLE_BKICK

/* Enable the !tb(an) trigger */
#define ENABLE_TBAN

/* Enable the !tkb trigger */
#define ENABLE_TKICKBAN

/* EOF */
