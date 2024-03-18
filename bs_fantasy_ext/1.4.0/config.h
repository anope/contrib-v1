/**
 * Module's Compile time Configuration Options
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
 * Last Updated   : 20/11/2011
 *
 **/


/**
 * SUPPORTED
 * Undefining this will make you lose all support for bs_fantasy_ext!!!
 *
 * As of version 1.1.13 (RC2) bs_fantasy_ext checks whether unsupported modules are enabled/loaded in anope.
 * If this is the case, bs_fantasy_ext will attempt to unload them or unload itself if it s a config directive.
 * These measures have been added to protect users against themselves and prevent the author of this module
 * from being confronted with situations that would not occor during normal use (for example when using RAW).
 *
 * If you undef this directive, these checks will be disabled, however you WILL LOSE ALL SUPPORT!!!
 *
 * To undefine this replace "#define" by "#undef" or simply comment the line out.
 **/
#define SUPPORTED


/**
 * When defined bs_fantasy_ext will automatically unload modules made redundant.
 * However, due to restrictions on the Anope core, this may not work properly on all OSs.
 * If you experience problems or crashes when bs_fantasy_ext tries to unload other modules
 * undefine this directive or simply comment the line out.
 *
 * Note that this directive does not have any impact when compiling on windows since
 * it never supports auto-unloading and is therefore always disabled.
 **/
#define AUTO_UNLOAD

/**
 * This determines the delimeter that will be used to seperate the existing topic and the part
 * added to it by !appendtopic.
 **/
#define AppendToTopicDel		"||"


/**
 * To disable a fantasy trigger Undefine (#undef) these or simply comment the line out.
 * Define it to enable the trigger. This was added on request and considering the anope core provides
 * the ability to anable/disable certain commands by not loading some core modules it would
 * be stupid if those commands were still accessible through the fantasy commands.
 * Note that not all commands can be disabled here !
 *
 * This needs to be set at compile time and changing these will require you to recompile the module.
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

/* Enable the !set trigger that gives access to both the chanserv and botserv set options*/
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

/* Enable the !suspend trigger */
#define ENABLE_SUSPEND

/* Enable the !unban command (Also requires OverrideCoreCmds to be set) */
#define ENABLE_UNBAN

/* Enable the !sync trigger (Also requires cs_sync to be loaded.) */
#define ENABLE_SYNC

/* Enable the !shun trigger (Also requires os_shun to be loaded) */
#define ENABLE_SHUN

/* Enable the !tshun trigger (Also requires os_tshun to be loaded) */
#define ENABLE_TSHUN

/* Enable !kick trigger implementation by this module to override the one in the core */
#define ENABLE_KICK

/* EOF */
