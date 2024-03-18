/* ----------------------------------------------------------------------------
 * Name    : diceserv.c
 * Author  : Naram Qashat (CyberBotX)
 * Version : 2.0.2
 * Date    : (Last modified) September 24th, 2011
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 1, or (at your option) any later
 * version.
 * ----------------------------------------------------------------------------
 * Requires: Anope-1.8.x (May work with later versions of 1.7.x, untested)
 * ----------------------------------------------------------------------------
 * Description:
 *
 * A dice rolling pseudo-client, mainly useful for playing pen & paper RPGs,
 * such as Dungeons and Dragons, over IRC.
 * ----------------------------------------------------------------------------
 * Usage:
 *
 * This module has 3 configurable options.  Make sure to add diceserv to your
 * ModuleDelayedAutoload directive.
 *
 * ----- Put these lines in your services.conf --------------------------------

# DiceServDBName [OPTIONAL]
# Specifies the filename of DiceServ's database.  If not given, defaults
#   to "diceserv.db".

#DiceServDBName "diceserv.db"

# DiceServName [OPTIONAL]
# Specifies the nickname of the DiceServ pseudo-client.  If not given, defaults
#   to "DiceServ".

#DiceServName "DiceServ"

# DiceServChanOpCanIgnore [OPTIONAL]
# Network-wide setting to allow channel operators to also have the ability to
#   have DiceServ ignore a channel instead of just the channel founder.

#DiceServChanOpCanIgnore

 * ----- End of Configuration -------------------------------------------------
 * Changelog:
 *
 * 2.0.2 - Fix a crash bug for the Anope 1.8.x versions and a lack of an error
 *           message for the Anope 1.9.x versions caused if there is nothing
 *           before or after the tilde.
 * 2.0.1 - Minor fix of permission in DiceServ's SET command.
 *       - Minor edit of GECOS of DiceServ to use "Service" instead of
 *           "Server".
 *       - Added version for Anope 1.9.5.
 *   2.0 - Replaced Coda's expression parser with a parser of my own that
 *           converts the expression from infix to postfix using the
 *           shunting-yard algorithm and then evaluates it.
 *       - Added support for DnD3e character creation rolls.
 *       - Removed Exalted dice rolls from the service, I'm not 100% sure they
 *           were ever being done right.
 *       - Used enums for the dice roll types and errors for easier usage.
 *       - Fixed overflow checks.
 *       - Allow for d% anywhere in an expression, suggested by Namegduf.
 *       - Split apart parsing the expression and evaluating it, suggested by
 *           Namegduf.
 *       - Better buffer overflow checking, and removed check for the
 *           expression being too long, the current parser can handle long
 *           expressions unlike the last parser (no offense to Coda).
 *       - Added support for the pen & paper RPG Earthdawn, suggested by
 *           DukePyrolator.
 *       - Added support for math functions. (such as sqrt, trunc, cos, etc.)
 *       - Added support for the math constants of e and pi.
 *       - Added a CALC command which is like ROLL but without rounding.
 *       - Added configuration options for DiceServ's pseudo-client name as
 *           well as giving a network-wide ability for channel operators to
 *           set DiceServ channel ignores.
 *       - Added functionality for a shorter extended output if showing the
 *           results of each individual roll would be too long to display.
 *       - Better way of determining when to display the extended output
 *           buffer.
 *       - Utilize the Mersenne Twister RNG (random number generator) instead
 *           of Anope's RNG, it's faster but uses up a bit more memory. As a
 *           result, Coda's algorithm used in the dice roller is not longer
 *           used.
 *       - Added support for dual-argument functions of max(), min, and rand().
 *   1.0 - Initial version (was only part of an Epona 1.4.14 edit).
 *       - Contained expression parser and dice roller algorithms from Adam
 *           Higerd (Coda Highland).
 *
 * ----------------------------------------------------------------------------
 */

#ifdef _WIN32
# include <float.h>
#endif
#include <math.h>
#include "module.h"

static const char *const AUTHOR = "Naram Qashat (CyberBotX)";
static const char *const VERSION = "v2.0.2";

static const unsigned DICE_MAX_TIMES = 25;
static const unsigned DICE_MAX_DICE = 99999;
static const unsigned DICE_MAX_SIDES = 99999;
static const unsigned DICE_MAX_EX_SIZE = 510;
enum DICE_ERRS /* Error codes */
{
	DICE_ERR_NONE,
	DICE_ERR_PARSE,
	DICE_ERR_DIV0,
	DICE_ERR_UNDEFINED,
	DICE_ERR_UNACCEPT_DICE,
	DICE_ERR_UNACCEPT_SIDES,
	DICE_ERR_OVERUNDERFLOW,
	DICE_ERR_STACK
};
enum DICE_TYPES /* Roll types */
{
	DICE_TYPE_ROLL,
	DICE_TYPE_CALC,
	DICE_TYPE_EXROLL,
	DICE_TYPE_EXCALC,
	DICE_TYPE_DND3E,
	DICE_TYPE_EARTHDAWN
};
enum DICE_LNG /* Language strings */
{
	DICE_ROLL_SYNTAX,
	DICE_CALC_SYNTAX,
	DICE_EXROLL_SYNTAX,
	DICE_EXCALC_SYNTAX,
	DICE_DND3ECHAR_SYNTAX,
	DICE_EARTHDAWN_SYNTAX,
	DICE_ROLL_USER,
	DICE_CALC_USER,
	DICE_EXROLL_USER,
	DICE_EXCALC_USER,
	DICE_DND3ECHAR_USER,
	DICE_EARTHDAWN_USER,
	DICE_ERROR_NOPARENAFTERFUNCTION,
	DICE_ERROR_BEFORECOMMA,
	DICE_ERROR_AFTERCOMMA,
	DICE_ERROR_BEFOREOPERATOR,
	DICE_ERROR_AFTEROPERATOR,
	DICE_ERROR_BEFOREOPENPAREN,
	DICE_ERROR_AFTEROPENPAREN,
	DICE_ERROR_BEFORECLOSEPAREN,
	DICE_ERROR_AFTERCLOSEPAREN,
	DICE_ERROR_INVALIDCHAR,
	DICE_ERROR_NONUMBERBEFOREOPERATOR,
	DICE_ERROR_NOTENOUGHOPENPARENS,
	DICE_ERROR_INVALIDCOMMA,
	DICE_ERROR_TOOMANYOPENPARENS,
	DICE_ERROR_EMPTYTOKEN,
	DICE_ERROR_NOTENOUGHFUNCTIONARGS,
	DICE_ERROR_NOTENOUGHOPERATORARGS,
	DICE_ERROR_EMPTYNUMBER,
	DICE_ERROR_TOOMANYNUMBERS,
	DICE_PARSE_ERROR,
	DICE_ERROR_DIVBYZERO,
	DICE_ERROR_UNDEFINED,
	DICE_DICEOVERLIMIT,
	DICE_SIDESOVERLIMIT,
	DICE_TIMESOVERLIMIT,
	DICE_DICEUNDERLIMIT,
	DICE_SIDESUNDERLIMIT,
	DICE_TIMESUNDERLIMIT,
	DICE_STACK_ERR,
	DICE_BUFFER_OVERFLOW,
	DICE_EX_OVERFLOW,
	DICE_OVERUNDERFLOW,
	DICE_ERRSTR,
	DICE_ERROR_EARTHDAWN_INPUT,
	DICE_ERROR_EARTHDAWN_RANGE,
	DICE_ERROR_DND3E_MODTOTALZERO,
	DICE_ERROR_DND3E_MAXSCORETHIRTEEN,
	DICE_SET_DISABLED,
	DICE_SET_SYNTAX,
	DICE_SET_UNKNOWN_OPTION,
	DICE_SET_IGNORE_SYNTAX,
	DICE_SERVADMIN_SET_IGNORE_SYNTAX,
	DICE_SET_IGNORE_CHAN_ON,
	DICE_SET_IGNORE_CHAN_OFF,
	DICE_SET_IGNORE_NICK_ON,
	DICE_SET_IGNORE_NICK_OFF,
	DICE_STATUS_SYNTAX,
	DICE_STATUS_CHAN_REGGED,
	DICE_STATUS_CHAN,
	DICE_STATUS_NICK_ONLINE,
	DICE_STATUS_NICK_REGGED,
	DICE_STATUS_NICK,
	DICE_STATUS_IGNORE,
	DICE_STATUS_ALLOW,
	DICE_LIST_SYNTAX,
	DICE_LIST_HEADER,
	DICE_LIST_HEADER_IGNORED,
	DICE_LIST_HEADER_ALLOWED,
	DICE_LIST_HEADER_ALL,
	DICE_LIST_HEADER_CHANNELS,
	DICE_LIST_HEADER_NICKS,
	DICE_LIST_HEADER_REGONLY,
	DICE_LIST_HEADER_UNREGONLY,
	DICE_LIST_UNREG,
	DICE_LIST_REG,
	DICE_LIST_END,
	DICE_INFO_IGNORE,
	DICE_INVALID_NICK,
	DICE_IGNORED,
	DICE_ALLOWED,
	DICE_HELP_CMD_ROLL,
	DICE_HELP_CMD_CALC,
	DICE_HELP_CMD_EXROLL,
	DICE_HELP_CMD_EXCALC,
	DICE_HELP_CMD_DND3ECHAR,
	DICE_HELP_CMD_EARTHDAWN,
	DICE_HELP_CMD_SET,
	DICE_HELP_CMD_STATUS,
	DICE_HELP_CMD_LIST,
	DICE_HELP,
	DICE_HELP_FOOTER,
	DICE_HELP_ROLL,
	DICE_HELP_ROLL_EXPRESSIONS,
	DICE_HELP_FUNCTIONS,
	DICE_HELP_CALC,
	DICE_HELP_EXROLL,
	DICE_HELP_EXCALC,
	DICE_HELP_DND3ECHAR,
	DICE_HELP_EARTHDAWN,
	DICE_HELP_SET,
	DICE_HELP_SET_IGNORE,
	DICE_SERVADMIN_HELP_SET_IGNORE,
	DICE_SERVADMIN_HELP_STATUS,
	DICE_SERVADMIN_HELP_LIST,
	DICE_NUM_STRINGS
};

/* Step table for the pen & paper RPG Earthdawn
 * Retrieved from http://arkanabar.tripod.com/steps.html
step	dice		step	dice				step	dice				step	dice
1		d4-2		26		d20d10d8d6			51		2d20d12+2d10+2d8	76		3d20+4d10+3d8d6
2		d4-1		27		d20d10+2d8			52		2d20+2d10+2d8+2d6	77		3d20+4d10+4d8
3		d4			28		d20+2d10d8			53		2d20+2d10+3d8d6		78		3d20+5d10+3d8
4		d6			29		d20d12d10d8			54		2d20+3d10+2d8d6		79		3d20d12+4d10+3d8
5		d8			30		d20d10d8+2d6		55		2d20+3d10+3d8		80		4d20+3d10+3d8d4
6		d10			31		d20d10+2d8d6		56		2d20+4d10+2d8		81		4d20+3d10+3d8d6
7		d12			32		d20+2d10d8d6		57		2d20d12+3d10+2d8	82		4d20+3d10+4d8
8		2d6			33		d20+2d10+2d8		58		3d20+2d10+2d8d4		83		4d20+4d10+3d8
9		d8d6		34		d20+3d10d8			59		3d20+2d10+2d8d6		84		4d20d12+3d10+3d8
10		d10d6		35		d20d12+2d10d8		60		3d20+2d10+3d8		85		4d20+3d10+3d8+2d6
11		d10d8		36		2d20d10d8d4			61		3d20+3d10+2d8		86		4d20+3d10+4d8d6
12		2d10		37		2d20d10d8d6			62		3d20d12+2d10+2d8	87		4d20+4d10+3d8d6
13		d12d10		38		2d20d10+2d8			63		3d20+2d10+2d8+2d6	88		4d20+4d10+4d8
14		d20d4		39		2d20+2d10d8			64		3d20+2d10+3d8d6		89		4d20+5d10+3d8
15		d20d6		40		2d20d12d10d8		65		3d20+3d10+2d8d6		90		4d20d12+4d10+3d8
16		d20d8		41		2d20d10d8+2d6		66		3d20+3d10+3d8		91		4d20+4d10+4d8d4
17		d20d10		42		2d20d10+2d8d6		67		3d20+4d10+2d8		92		4d20+4d10+4d8d6
18		d20d12		43		2d20+2d10d8d6		68		3d20d12+3d10+2d8	93		4d20+4d10+5d8
19		d20+2d6		44		2d20+2d10+2d8		69		3d20+3d10+3d8d4		94		4d20+5d10+4d8
20		d20d8d6		45		2d20+3d10d8			70		3d20+3d10+3d8d6		95		4d20d12+4d10+4d8
21		d20d10d6	46		2d20d12+2d10d8		71		3d20+3d10+4d8		96		4d20+4d10+4d8+2d6
22		d20d10d8	47		2d20+2d10+2d8d4		72		3d20+4d10+3d8		97		4d20+4d10+5d8d6
23		d20+2d10	48		2d20+2d10+2d8d6		73		3d20d12+3d10+3d8	98		4d20+5d10+4d8d6
24		d20d12d10	49		2d20+2d10+3d8		74		3d20+3d10+3d8+2d6	99		4d20+5d10+5d8
25		d20d10d8d4	50		2d20+3d10+2d8		75		3d20+3d10+4d8d6		100		4d20+6d10+4d8
 */
static const char *const EarthdawnStepTable[] =
{
	"", /* 0, not used */
	"1d4-2", "1d4-1", "1d4", "1d6", "1d8", /* 1-5 */
	"1d10", "1d12", "2d6", "1d8+1d6", "1d10+1d6", /* 6-10 */
	"1d10+1d8", "2d10", "1d12+1d10", "1d20+1d4", "1d20+1d6", /* 11-15 */
	"1d20+1d8", "1d20+1d10", "1d20+1d12", "1d20+2d6", "1d20+1d8+1d6", /* 16-20 */
	"1d20+1d10+1d6", "1d20+1d10+1d8", "1d20+2d10", "1d20+1d12+1d10", "1d20+1d10+1d8+1d4", /* 21-25 */
	"1d20+1d10+1d8+1d6", "1d20+1d10+2d8", "1d20+2d10+1d8", "1d20+1d12+1d10+1d8", "1d20+1d10+1d8+2d6", /* 26-30 */
	"1d20+1d10+2d8+1d6", "1d20+2d10+1d8+1d6", "1d20+2d10+2d8", "1d20+3d10+1d8", "1d20+1d12+2d10+1d8", /* 31-35 */
	"2d20+1d10+1d8+1d4", "2d20+1d10+1d8+1d6", "2d20+1d10+2d8", "2d20+2d10+1d8", "2d20+1d12+1d10+1d8", /* 36-40 */
	"2d20+1d10+1d8+2d6", "2d20+1d10+2d8+1d6", "2d20+2d10+1d8+1d6", "2d20+2d10+2d8", "2d20+3d10+1d8", /* 41-45 */
	"2d20+1d12+2d10+1d8", "2d20+2d10+2d8+1d4", "2d20+2d10+2d8+1d6", "2d20+2d10+3d8", "2d20+3d10+2d8", /* 46-50 */
	"2d20+1d12+2d10+2d8", "2d20+2d10+2d8+2d6", "2d20+2d10+3d8+1d6", "2d20+3d10+2d8+1d6", "2d20+3d10+3d8", /* 51-55 */
	"2d20+4d10+2d8", "2d20+1d12+3d10+2d8", "3d20+2d10+2d8+1d4", "3d20+2d10+2d8+1d6", "3d20+2d10+3d8", /* 56-60 */
	"3d20+3d10+2d8", "3d20+1d12+2d10+2d8", "3d20+2d10+2d8+2d6", "3d20+2d10+3d8+1d6", "3d20+3d10+2d8+1d6", /* 61-65 */
	"3d20+3d10+3d8", "3d20+4d10+2d8", "3d20+1d12+3d10+2d8", "3d20+3d10+3d8+1d4", "3d20+3d10+3d8+1d6", /* 66-70 */
	"3d20+3d10+4d8", "3d20+4d10+3d8", "3d20+1d12+3d10+3d8", "3d20+3d10+3d8+2d6", "3d20+3d10+4d8+1d6", /* 71-75 */
	"3d20+4d10+3d8+1d6", "3d20+4d10+4d8", "3d20+5d10+3d8", "3d20+1d12+4d10+3d8", "4d20+3d10+3d8+1d4", /* 76-80 */
	"4d20+3d10+3d8+1d6", "4d20+3d10+4d8", "4d20+4d10+3d8", "4d20+1d12+3d10+3d8", "4d20+3d10+3d8+2d6", /* 81-85 */
	"4d20+3d10+4d8+1d6", "4d20+4d10+3d8+1d6", "4d20+4d10+4d8", "4d20+5d10+3d8", "4d20+1d12+4d10+3d8", /* 86-90 */
	"4d20+4d10+4d8+1d4", "4d20+4d10+4d8+1d6", "4d20+4d10+5d8", "4d20+5d10+4d8", "4d20+1d12+4d10+4d8", /* 91-95 */
	"4d20+4d10+4d8+2d6", "4d20+4d10+5d8+1d6", "4d20+5d10+4d8+1d6", "4d20+5d10+5d8", "4d20+6d10+4d8"/* 96-100 */
};

static char	*s_DiceServ = NULL, /* DiceServ client name, I don't like that I had to make this a char* to stop warnings :/ */
			*DiceServDBName = NULL, /* Database file name */
			*DiceExOutput, /* Extended output string */
			*DiceShortExOutput, /* Shorter extended output string */
			*RollSource, /* Source of the roll when SourceIsBot is set */
			DiceErrStr[81]; /* Error string */
static CommandHash *DiceServ_cmdTable[MAX_CMD_HASH]; /* Commands for DiceServ */
static enum DICE_ERRS DiceErrCode; /* Error code, see DICE_ERRS above */
static int	DiceErrNum, /* Value that caused last error, if needed */
			SourceIsBot, /* 1 when DiceServ is invoked through a BotServ fantasy command */
			DoDnD3eChar, /* 1 when a DnD3e character roll is requested */
			DoEarthdawn, /* 1 when an Earthdawn roll is requested */
			DnDmods[6][2], /* Array storing all 6 sets of DnD3e character rolls, index 0 storing the actual values (modified), index 1 storing the mod value */
			DiceServChanOpCanIgnore; /* 1 when channel operators should be allowed to have DiceServ ignore a channel, otherwise only channel owners can set that */
static unsigned	DiceErrPos, /* Position of last error, if needed */
				DiceEx, /* 1 if any sort of extended roll is requsted, 2 if we were doing an extended roll but the extended output string overflowed as we are going to use the shorter extended output string,
						 * 3 if we are doing an extended roll but both extended output strings overflowed, 0 for no extended roll */
				DnDresults[4], /* Array storing the 4 rolls in a single set for a DnD3e character roll */
				DnDPos = 0; /* Index within above array */
static User *CurrUser = NULL; /* Current user that is using DiceServ */

/* Ignored channel handling */
#ifdef LIBOL
static libol_array_t ignored_channels;

static bool ignored_channels_compare(void *data1, void *data2)
{
	return !stricmp(data1, data2);
}
#else
static char **ignored_channels = NULL; /* Have to do this since the Channel struct doesn't have ModuleData like NickCore, ChannelInfo, and User do */
static unsigned num_ignored_channels = 0;
#endif

/** Check the ignored channels array for the given channel name.
 * @param chan Name of the channel to search for
 * @return -1 if the channel is not found in the array, otherwise the index within the array
 *
 * NOTE: -1 will also be returned when the array is empty.
 */
static int find_chan_ignore(const char *chan)
{
#ifdef LIBOL
	return libol_array_find(&ignored_channels, chan);
#else
	if (ignored_channels && num_ignored_channels)
	{
		unsigned i;
		for (i = 0; i < num_ignored_channels; ++i)
			if (!stricmp(chan, ignored_channels[i]))
				return i;
	}

	return -1;
#endif
}

/** Deletes the given channel name from the ignored channels array.
 * @param chan Name of the channel to delete
 *
 * If the channel is not found, nothing will be done.
 */
static void del_chan_ignore(const char *chan)
{
#ifdef LIBOL
	libol_array_del(&ignored_channels, chan);
#else
	int index = find_chan_ignore(chan);

	if (index != -1)
	{
		char **new_ignored_channels = (char **)smalloc(sizeof(char *) * (num_ignored_channels - 1));
		unsigned i, j;
		for (i = 0, j = 0; i < num_ignored_channels; ++i)
		{
			if (i == index)
			{
				free(ignored_channels[i]);
				continue;
			}
			new_ignored_channels[j++] = ignored_channels[i];
		}
		free(ignored_channels);
		ignored_channels = new_ignored_channels;
		--num_ignored_channels;
	}
#endif
}

/** Adds the given channel name to the ignored channels array.
 * @param chan Name of the channel to add
 *
 * This will also delete an existing entry before trying to add the channel.
 */
static void add_chan_ignore(const char *chan)
{
	del_chan_ignore(chan);

#ifdef LIBOL
	libol_array_add(&ignored_channels, sstrdup(chan));
#else
	ignored_channels = (char **)srealloc(ignored_channels, sizeof(char *) * (num_ignored_channels + 1));
	ignored_channels[num_ignored_channels++] = sstrdup(chan);
#endif
}

/** Clears the entire ignored channels array from memory.
 */
static void clear_chan_ignores(void)
{
#ifdef LIBOL
	libol_array_clear(&ignored_channels);
#else
	unsigned i;
	for (i = 0; i < num_ignored_channels; ++i)
		free(ignored_channels[i]);

	free(ignored_channels);
	ignored_channels = NULL;
	num_ignored_channels = 0;
#endif
}

/** Calculates the length of a string.
 * @param str String to calculate length of
 * @return Length of string
 *
 * This will return 0 on a NULL string.
 */
static size_t my_strlen(const char *str)
{
	return str ? strlen(str) : 0;
}
/** Copy a string to a string
 * @param str Pointer to the string
 * @param str2 String to copy
 *
 * This might be a bit much when I could use a statically sized array for the string, but this gives me something similar to C++'s string class without using C++.
 */
static void my_strcpy(char **str, const char *str2)
{
	if (*str)
		free(*str);
	if (str2)
	{
		*str = (char *)smalloc(strlen(str2) + 1);
		strcpy(*str, str2);
	}
	else
		*str = NULL;
}
/** Concatenate a single character to a string.
 * @param str Pointer to the string
 * @param chr Character to concatenate
 *
 * This might be a bit much when I could use a statically sized array for the string, but this gives me something similar to C++'s string class without using C++.
 */
static void my_strccat(char **str, char chr)
{
	char insert[2] = {chr, 0};
	if (*str)
		*str = (char *)srealloc(*str, strlen(*str) + 2);
	else
	{
		*str = (char *)smalloc(2);
		(*str)[0] = 0;
	}
	strcat(*str, insert);
}
/** Concatenate a string to a string.
 * @param str Pointer to the string
 * @param str2 String to concatenate
 *
 * This might be a bit much when I could use a statically sized array for the string, but this gives me something similar to C++'s string class without using C++.
 */
static void my_strcat(char **str, const char *str2)
{
	if (str2)
	{
		size_t len = my_strlen(*str) + strlen(str2) + 1;
		if (*str)
			*str = (char *)srealloc(*str, len);
		else
		{
			*str = (char *)smalloc(len);
			(*str)[0] = 0;
		}
		strcat(*str, str2);
	}
}

/** Determine if the double-precision floating point value is infinite or not.
 * @param num The double-precision floating point value to check
 * @return 1 if the value is infinite, 0 otherwise
 */
static int is_infinite(double num)
{
#ifdef _WIN32
	int fpc = _fpclass(num);
	return fpc == _FPCLASS_NINF || fpc == _FPCLASS_PINF;
#else
	return isinf(num);
#endif
}

/** Determine if the double-precision floating point value is NaN (Not a Number) or not.
 * @param num The double-precision floating point value to check
 * @return 1 if the value is NaN, 0 otherwise
 */
static int is_notanumber(double num)
{
#ifdef _WIN32
	int fpc = _fpclass(num);
	return fpc == _FPCLASS_SNAN || fpc == _FPCLASS_QNAN;
#else
	return isnan(num);
#endif
}

#ifdef _WIN32
/** Calculate inverse hyperbolic cosine for Windows.
 * @param num The double-precision floating point value to calculate
 * @return The inverse hyperbolic cosine of the value
 */
static double acosh(double num)
{
	return log(num + sqrt(num - 1) * sqrt(num + 1));
}

/** Calculate inverse hyperbolic sine for Windows.
 * @param num The double-precision floating point value to calculate
 * @return The inverse hyperbolic sine of the value
 */
static double asinh(double num)
{
	return log(num + sqrt(num * num + 1));
}

/** Calculate inverse hyperbolic tangent for Windows.
 * @param num The double-precision floating point value to calculate
 * @return The inverse hyperbolic tangent of the value
 */
static double atanh(double num)
{
	return 0.5 * log((1 + num) / (1 - num));
}

/** Calculate cube root for Windows.
 * @param num The double-percision floating point value to calculate
 * @return The cube root of the value
 */
static double cbrt(double num)
{
	return pow(num, 1.0 / 3.0);
}

/** Truncate a double-precision floating point value for Windows.
 * @param num The double-precision floating point value to truncate
 * @return The truncated value
 */
static double trunc(double num)
{
	double intPart;
	modf(num, &intPart);
	return intPart;
}

/*-
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/** Case-insensitive substring search.
 * @param s Original string to search in
 * @param find Substring fo search for
 * @return If the substring is empty, the original string is returned. If the substring is not in the original string, NULL is returned,
 *           otherwise, a pointer is returned that corresponds to the position in the original string where the substring occurs
 *
 * This code comes from the BSD libc implemenation.
 */
static char *strcasestr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

	if ((c = *find++))
	{
		c = tolower((unsigned char)c);
		len = strlen(find);
		do
		{
			do
			{
				if (!(sc = *s++))
					return NULL;
			} while ((char)tolower((unsigned char)sc) != c);
		} while (_strnicmp(s, find, len));
		--s;
	}
	return (char *)s;
}
#endif

#define MERS_N 624
static const int MERS_M = 397;
static const int MERS_R = 31;
static const int MERS_U = 11;
static const int MERS_S = 7;
static const int MERS_T = 15;
static const int MERS_L = 18;
#define MERS_A 0x9908B0DF
static const uint32 MERS_B = 0x9D2C5680;
static const uint32 MERS_C = 0xEFC60000;

static uint32 mt[MERS_N]; /* State vector */
static int mti; /* Index into mt */

/** Generate 32 random bits using the Mersenne Twister algorithm.
 * @return The random bits generated
 *
 * This code was copied from Agner Fog's C++ class library of uniform
 * random number generators, obtained from http://www.agner.org/random/
 */
static uint32 Mersenne_BRandom()
{
	/* Generate 32 random bits */
	uint32 y;

	if (mti >= MERS_N)
	{
		/* Generate MERS_N words at one time */
		const uint32 LOWER_MASK = (1LU << MERS_R) - 1; /* Lower MERS_R bits */
		const uint32 UPPER_MASK = 0xFFFFFFFF << MERS_R; /* Upper (32 - MERS_R) bits */
		static const uint32 mag01[2] = {0, MERS_A};

		int kk;
		for (kk = 0; kk < MERS_N - MERS_M; ++kk)
		{
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + MERS_M] ^ (y >> 1) ^ mag01[y & 1];
		}

		for (; kk < MERS_N - 1; ++kk)
		{
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + (MERS_M - MERS_N)] ^ (y >> 1) ^ mag01[y & 1];
		}

		y = (mt[MERS_N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
		mt[MERS_N - 1] = mt[MERS_M - 1] ^ (y >> 1) ^ mag01[y & 1];
		mti = 0;
	}
	y = mt[mti++];

	/* Tempering (May be omitted): */
	y ^=  y >> MERS_U;
	y ^= (y << MERS_S) & MERS_B;
	y ^= (y << MERS_T) & MERS_C;
	y ^=  y >> MERS_L;

	return y;
}

/** Internal initialization of the Mersenne Twister RNG.
 * @param seed The value used to seed the generator.
 *
 * This function is not meant to be called directly, but rather should
 * be called through Mersenne_RandomInit() instead.
 *
 * This code was copied from Agner Fog's C++ class library of uniform
 * random number generators, obtained from http://www.agner.org/random/
 */
static void Mersenne_Init0(int seed)
{
	/* Seed generator */
	const uint32 factor = 1812433253UL;
	mt[0] = seed;
	for (mti = 1; mti < MERS_N; ++mti)
		mt[mti] = (factor * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
}

/** Initialze the Mersenene Twister RNG.
 * @param seed The value used to seed the generator.
 *
 * This code was copied from Agner Fog's C++ class library of uniform
 * random number generators, obtained from http://www.agner.org/random/
 */
static void Mersenne_RandomInit(int seed)
{
	int i;
	/* Initialize and seed */
	Mersenne_Init0(seed);

	/* Randomize some more */
	for (i = 0; i < 37; ++i)
		Mersenne_BRandom();
}

/** Generate a random floating point value.
 * @return A floating point value in the interval 0 <= x < 1
 *
 * This function is called from Mersenne_IRandom() and should probably
 * not be called directly.
 *
 * This code was copied from Agner Fog's C++ class library of uniform
 * random number generators, obtained from http://www.agner.org/random/
 */
static double Mersenne_Random()
{
	/* Output random float number in the interval 0 <= x < 1
	 * Multiply by 2^(-32) */
	return (double)Mersenne_BRandom() * (1.0 / (65536.0 * 65536.0));
}

/** Generate a random integer within the given range.
 * @param min The minimum value of the range
 * @param max The maximum value of the range
 * @return An integer in the interval min <= x <= max
 *
 * This code was copied from Agner Fog's C++ class library of uniform
 * random number generators, obtained from http://www.agner.org/random/
 */
static int Mersenne_IRandom(int min, int max)
{
	int r;
	/* Output random integer in the interval min <= x <= max
	 * Relative error on frequencies < 2^-32 */
	if (max <= min)
	{
		if (max == min)
			return min;
		else
			return 0x80000000;
	}
	/* Multiply interval with random and truncate */
	r = (int)((double)((uint32)(max - min + 1)) * Mersenne_Random() + min);
	if (r > max)
		r = max;
	return r;
}

/** Determine if the given character is a number.
 * @param chr Character to check
 * @return 1 if the character is a number, 0 otherwise
 */
static int is_number(char chr) { return (chr >= '0' && chr <= '9') || chr == '.'; }
/** Determine if the given character is a multiplication or division operator.
 * @param chr Character to check
 * @return 1 if the character is a multiplication or division operator, 0 otherwise
 */
static int is_muldiv(char chr) { return chr == '%' || chr == '/' || chr == '*'; }
/** Determine if the given character is an addition or subtraction operator.
 * @param chr Character to check
 * @return 1 if the character is an addition or subtraction operator, 0 otherwise
 */
static int is_plusmin(char chr) { return chr == '+' || chr == '-'; }
/** Determine if the given character is an operator of any sort, except for parenthesis.
 * @param chr Character to check
 * @return 1 if the character is a non-parenthesis operator, 0 otherwise
 */
static int is_op_noparen(char chr) { return is_plusmin(chr) || is_muldiv(chr) || chr == '^' || chr == 'd'; }
/** Determine if the given character is an operator of any sort.
 * @param chr Character to check
 * @return 1 if the character is an operator, 0 otherwise
 */
static int is_operator(char chr) { return chr == '(' || chr == ')' || is_op_noparen(chr); }
/** Determine if the substring portion of the given string is a function.
 * @param func String to check
 * @return 0 if the string isn't a function, or a number corresponding to the length of the function name
 */
static unsigned is_function(const char *func)
{
	/* acosh, asinh, atanh, floor, log10, round, trunc */
	if (!strnicmp(func, "acosh", 5) || !strnicmp(func, "asinh", 5) || !strnicmp(func, "atanh", 5) || !strnicmp(func, "floor", 5) || !strnicmp(func, "log10", 5) || !strnicmp(func, "round", 5) || !strnicmp(func, "trunc", 5))
		return 5;
	/* acos, asin, atan, cbrt, ceil, cosh, rand, sinh, sqrt, tanh */
	if (!strnicmp(func, "acos", 4) || !strnicmp(func, "asin", 4) || !strnicmp(func, "atan", 4) || !strnicmp(func, "cbrt", 4) || !strnicmp(func, "ceil", 4) || !strnicmp(func, "cosh", 4) || !strnicmp(func, "rand", 4) ||
		!strnicmp(func, "sinh", 4) || !strnicmp(func, "sqrt", 4) || !strnicmp(func, "tanh", 4))
		return 4;
	/* abs, cos, deg, exp, fac, log, max, min, rad, sin, tan */
	if (!strnicmp(func, "abs", 3) || !strnicmp(func, "cos", 3) || !strnicmp(func, "deg", 3) || !strnicmp(func, "exp", 3) || !strnicmp(func, "fac", 3) || !strnicmp(func, "log", 3) || !strnicmp(func, "max", 3) ||
		!strnicmp(func, "min", 3) || !strnicmp(func, "rad", 3) || !strnicmp(func, "sin", 3) || !strnicmp(func, "tan", 3))
		return 3;
	/* None of the above */
	return 0;
}
/** Determine the number of arguments that the given function needs.
 * @param func Function string to check
 * @return Returns 1 except for the min, max, and rand functions which return 2.
 */
static unsigned function_argument_count(const char *func)
{
	if (!strnicmp(func, "max", 3) || !strnicmp(func, "min", 3) || !strnicmp(func, "rand", 4))
		return 2;
	return 1;
}
/** Determine if the substring portion of the given string is a constant (currently only e and pi).
 * @param constant String to check
 * @return 0 if the string isn't a constant, or a number corresponding to the length of the constant's name
 */
static unsigned is_constant(const char *constant)
{
	/* pi */
	if (!strnicmp(constant, "pi", 2))
		return 2;
	/* e */
	if (!strnicmp(constant, "e", 1))
		return 1;
	/* None of the above */
	return 0;
}
/** Determine if the given operator has a higher precendence than the operator on the top of the stack during infix to postfix conversion.
 * @param adding The operator we are adding to the stack, or an empty string if nothing is being added and we just want to remove
 * @param topstack The operator that was at the top of the operator stack
 * @return 0 if the given operator has the same or lower precendence (and won't cause a pop), 1 if the operator has higher precendence (and will cause a pop)
 *
 * In addition to the above in regards to the return value, there are other situations. If the top of the stack is an open parenthesis,
 * or is empty, a 0 will be returned to stop the stack from popping anything else. If nothing is being added to the stack and the previous
 * sitation hasn't occurred, a 1 will be returned to signify to continue popping the stack until the previous sitation occurs. If the operator
 * being added is a function, we return 0 to signify we aren't popping. If the top of the stack is a function, we return 1 to signify we are
 * popping. A -1 is only returned if an invalid operator is given, hopefully that will never happen.
 */
static int would_pop(const char *adding, const char *topstack)
{
	if (!adding)
		return !topstack || topstack[0] == '(' ? 0 : 1;
	if (is_function(adding))
		return 0;
	if (!topstack || topstack[0] == '(')
		return 0;
	if (is_function(topstack))
		return 1;
	if (!stricmp(topstack, adding) && adding[0] != '^')
		return 1;
	switch (adding[0])
	{
		case 'd':
			return 0;
		case '^':
			return topstack[0] == '^' || topstack[0] == 'd' ? 1 : 0;
		case '%':
		case '/':
		case '*':
			return topstack[0] == '^' || topstack[0] == 'd' || is_muldiv(topstack[0]) ? 1 : 0;
		case '+':
		case '-':
			return is_op_noparen(topstack[0]) ? 1 : 0;
	}
	return -1;
}

/** Calculate a die roll for the given number of sides for a set number of times.
 * @param num Number of times to throw the die
 * @param sides Number of sides on the die
 * @return The sum of the results of the die after every throw
 *
 * This also handles showing each roll separately if an extended roll command has been used. If a DnD3e character roll was requested,
 * that will also be stored.
 */
static unsigned Dice(int num, unsigned sides)
{
	unsigned sum = 0;
	int i = 0, bonus = 0;
	char buf[20] = "";
	/* Add <x>d<y>=( to the extended output string */
	if (DiceEx == 1)
	{
		snprintf(buf, 20, "%ud%u=(", num, sides);
		if (my_strlen(DiceShortExOutput) <= DICE_MAX_EX_SIZE)
			my_strcat(&DiceShortExOutput, buf);
		if (my_strlen(DiceExOutput) <= DICE_MAX_EX_SIZE)
			my_strcat(&DiceExOutput, buf);
	}
	for (; i < num; ++i)
	{
		/* Get a random number between 1 and the number of sides */
		unsigned number = Mersenne_IRandom(1, sides);
		/* Add the result (plus a space before it for every roll after the first) to the extended output string */
		if (DiceEx == 1)
		{
			snprintf(buf, 20, "%s%u", i ? " " : "", number);
			/* If the user requested an Earthdawn roll and we are currently processing a bonus roll, we have to edit the buffer */
			if (DoEarthdawn && bonus)
			{
				char tmpbuf[20] = "";
				/* If the last character of the extended output string is a [, then this is the first bonus, we remove the space (if any) from the buffer */
				if (DiceExOutput && DiceExOutput[strlen(DiceExOutput) - 1] == '[')
				{
					if (*buf == ' ')
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", buf + 1);
					else
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", buf);
				}
				/* Otherwise, if it's the first die in the set, we force add a space */
				else if (!i)
					snprintf(tmpbuf, sizeof(tmpbuf), " %s", buf);
				my_strcat(&DiceExOutput, tmpbuf);
			}
			/* All other dice roll types, we just add the buffer to the extended output string */
			else if (my_strlen(DiceExOutput) <= DICE_MAX_EX_SIZE)
				my_strcat(&DiceExOutput, buf);
		}
		/* When doing a DnD3e character roll, add the current roll to the results array */
		if (DoDnD3eChar)
			DnDresults[DnDPos++] = number;
		/* When doing an Earthdawn roll, we have to check for the possiblity of bonus rolls when the roll maxes out */
		if (DoEarthdawn && DiceEx == 1)
		{
			/* When the number rolled is equal to the sides, the roll has maxed out */
			if (number == sides)
			{
				/* If we aren't yet in a bonus roll, we add the wording to the extended output string */
				if (!bonus)
					my_strcat(&DiceExOutput, " Bonus[");
				/* We set that we are in a bonus roll, and reduce i by one to "repeat" the current roll again */
				bonus = 1;
				--i;
			}
			/* Otherwise, the roll wasn't maxed, we close the bonus part of the buffer if it was open and set that we are no longer doing bonus rolls */
			else
			{
				if (bonus)
					my_strccat(&DiceExOutput, ']');
				bonus = 0;
			}
		}
		sum += number;
	}
	if (DiceEx == 1)
	{
		/* Add the sum to the shorter extended output string */
		snprintf(buf, 20, "%u", sum);
		if (my_strlen(DiceShortExOutput) <= DICE_MAX_EX_SIZE)
			my_strcat(&DiceShortExOutput, buf);

		/* Close the extended output string with a ) */
		if (my_strlen(DiceShortExOutput) <= DICE_MAX_EX_SIZE)
			my_strcat(&DiceShortExOutput, ") ");
		if (my_strlen(DiceExOutput) <= DICE_MAX_EX_SIZE)
			my_strcat(&DiceExOutput, ") ");
	}
	return sum;
}

/** Convert a double-precision floating point value to a string.
 * @param num The value to convert
 * @param size The field width of the value when converted to a string (will be reducded by 6 for the case of really small or really large numbers)
 * @param buffer The string to place the converted value in
 *
 * NOTE: This does not have a way to check the size of the buffer, it assumes it's the same size as temp.
 */
static void dtoa(double num, int size, char *buffer)
{
	char temp[33];
	snprintf(temp, sizeof(temp), "%.*g", size - 6, num);
	snprintf(buffer, sizeof(temp), "%s", temp);
}

/** Round a value to the given number of decimals, originally needed for Windows but also used for other OSes as well due to undefined references.
 * @param val The value to round
 * @param decimals The number of digits after the decimal point, defaults to 0
 * @return The rounded value
 *
 * NOTE: Function is a slightly modified form of the code from this page:
 * http://social.msdn.microsoft.com/forums/en-US/vclanguage/thread/a7d4bf31-6c32-4b25-bc76-21b29f5287a1/
 */
static double my_round(double val, unsigned decimals)
{
	double sign, tempval, decimalpart;
	unsigned tempint;
	if (!val) /* val must be different from zero to avoid division by zero! */
		return 0;
	sign = fabs(val) / val; /* we obtain the sign to calculate positive always */
	tempval = fabs(val * pow(10.0, (double)decimals)); /* shift decimal places */
	tempint = (unsigned)tempval;
	decimalpart = tempval - tempint; /* obtain just the decimal part */
	if (decimalpart >= 0.5)
		tempval = ceil(tempval); /* next integer number if greater or equal to 0.5 */
	else
		tempval = floor(tempval); /* otherwise stay in the current interger part */
	return (tempval * pow(10.0, (double)(-(int)decimals))) * sign; /* shift again to the normal decimal places */
}

/** Structure to store the infix notation string as well as the positions each character is compared to the original input */
struct Infix
{
	char *str;
	unsigned *positions, positions_len;
};

/** Creates a new Infix structure.
 * @return A new infix structure
 */
static struct Infix *create_infix()
{
	struct Infix *infix = (struct Infix *)smalloc(sizeof(struct Infix));
	infix->str = NULL;
	infix->positions = NULL;
	infix->positions_len = 0;
	return infix;
}
/** Free up the memory used by the Infix structure and free itself.
 * @param infix The Infix structure to cleanup.
 */
static void cleanup_infix(struct Infix *infix)
{
	free(infix->str);
	free(infix->positions);
	free(infix);
}
/** Add a position to the Infix structure.
 * @param infix The Infix structure to add to
 * @param position The position to add to the positions array
 */
static void add_to_infix(struct Infix *infix, unsigned position)
{
	if (!infix->positions_len)
		infix->positions = (unsigned *)smalloc(sizeof(unsigned));
	else
		infix->positions = (unsigned *)srealloc(infix->positions, sizeof(unsigned) * (infix->positions_len + 1));
	infix->positions[infix->positions_len++] = position;
}
/** Add a set of positions to the Infix structure.
 * @param infix The Infix structure to add to
 * @param positions The positions to add to the positions array
 * @param num The number of positions to add
 */
static void add_set_to_infix(struct Infix *infix, unsigned positions[], unsigned num)
{
	unsigned x = 0;
	if (!infix->positions_len)
		infix->positions = (unsigned *)smalloc(sizeof(unsigned) * num);
	else
		infix->positions = (unsigned *)srealloc(infix->positions, sizeof(unsigned) * (infix->positions_len + num));
	for (; x < num; ++x)
		infix->positions[infix->positions_len++] = positions[x];
}

/** Fix an infix notation equation.
 * @param infix The original infix notation equation
 * @return A fixed infix notation equation
 *
 * This will convert a single % to 1d100, place a 1 in front of any d's that have no numbers before them, change all %'s after a d into 100,
 * add *'s for implicit multiplication, and convert unary -'s to _ for easier parsing later.
 */
static struct Infix *fix_infix(const char *infix)
{
	struct Infix *newinfix = create_infix();
	int prev_was_func = 0, prev_was_const = 0;
	unsigned x = 0, len = my_strlen(infix);
	if (!strcmp(infix, "%"))
	{
		unsigned tmp[] = {0, 0, 0, 0, 0};
		my_strcpy(&newinfix->str, "1d100");
		add_set_to_infix(newinfix, tmp, 5);
		return newinfix;
	}
	for (x = 0; x < len; ++x)
	{
		unsigned func = is_function(&infix[x]), constant = is_constant(&infix[x]);
		char curr = tolower(infix[x]);
		/* Check for a function, and skip it if it exists */
		if (func)
		{
			unsigned y = x;
			if (x && is_number(infix[x - 1]))
			{
				my_strccat(&newinfix->str, '*');
				add_to_infix(newinfix, x);
			}
			for (; y < x + func; ++y)
			{
				my_strccat(&newinfix->str, infix[y]);
				add_to_infix(newinfix, y);
			}
			x += func - 1;
			prev_was_func = 1;
			continue;
		}
		/* Check for a constant, and skip it if it exists */
		if (constant)
		{
			unsigned y = x;
			if (x && is_number(infix[x - 1]))
			{
				my_strccat(&newinfix->str, '*');
				add_to_infix(newinfix, x);
			}
			for (; y < x + constant; ++y)
			{
				my_strccat(&newinfix->str, infix[y]);
				add_to_infix(newinfix, y);
			}
			if (x + constant < len && (is_number(infix[x + constant]) || is_constant(&infix[x + constant]) || is_function(&infix[x + constant])))
			{
				my_strccat(&newinfix->str, '*');
				add_to_infix(newinfix, x + constant);
			}
			x += constant - 1;
			prev_was_const = 1;
			continue;
		}
		if (curr == 'd')
		{
			add_to_infix(newinfix, x);
			if (!x)
			{
				my_strcat(&newinfix->str, "1d");
				add_to_infix(newinfix, x);
			}
			else
			{
				if (!is_number(infix[x - 1]) && infix[x - 1] != ')' && !prev_was_const)
				{
					my_strccat(&newinfix->str, '1');
					add_to_infix(newinfix, x);
				}
				my_strccat(&newinfix->str, 'd');
			}
			if (x != len - 1 && infix[x + 1] == '%')
			{
				my_strcat(&newinfix->str, "100");
				++x;
				add_to_infix(newinfix, x);
				add_to_infix(newinfix, x);
			}
		}
		else if (curr == '(')
		{
			if (x && !prev_was_func && (is_number(infix[x - 1]) || prev_was_const))
			{
				my_strccat(&newinfix->str, '*');
				add_to_infix(newinfix, x);
			}
			my_strccat(&newinfix->str, '(');
			add_to_infix(newinfix, x);
		}
		else if (curr == ')')
		{
			my_strccat(&newinfix->str, ')');
			add_to_infix(newinfix, x);
			if (x != len - 1 && (is_number(infix[x + 1]) || infix[x + 1] == '(' || is_constant(&infix[x + 1])))
			{
				my_strccat(&newinfix->str, '*');
				add_to_infix(newinfix, x);
			}
		}
		else if (curr == '-')
		{
			add_to_infix(newinfix, x);
			if (x != len - 1 && (!x ? 1 : is_op_noparen(tolower(infix[x - 1])) || infix[x - 1] == '(' || infix[x - 1] == ','))
			{
				if (infix[x + 1] == '(' || is_function(&infix[x + 1]))
				{
					my_strcat(&newinfix->str, "0-");
					add_to_infix(newinfix, x);
				}
				else if (is_number(infix[x + 1]) || is_constant(&infix[x + 1]))
					my_strccat(&newinfix->str, '_');
				else
					my_strccat(&newinfix->str, '-');
			}
			else
				my_strccat(&newinfix->str, '-');
		}
		else
		{
			my_strccat(&newinfix->str, curr);
			add_to_infix(newinfix, x);
		}
		prev_was_func = prev_was_const = 0;
	}
	add_to_infix(newinfix, len);
	return newinfix;
}

/** Validate an infix notation equation.
 * @param infix The infix notation equation to validate
 * @return 0 for an invalid equation, 1 for a valid one
 *
 * The validation is as follows:
 * - All functions must have an open parenthesis after them.
 * - A comma must be prefixed by a number or close parenthesis and must be suffixed by a number, open parenthesis, _ for unary minus, constant, or function.
 * - All non-parentheis operators must be prefixed by a number or close parenthesis and suffixed by a number, open parenthesis, _ for unary minus, constant, or function.
 * - All open parentheses must be prefixed by an operator, open parenthesis, or comma and suffixed by a number, an open parenthesis, _ for unary minus, constant, or function.
 * - All close parentheses must be prefixed by a number or close parenthesis and suffixed by an operator, close parenthesis, or comma.
 */
static int check_infix(const struct Infix *infix)
{
	int prev_was_func = 0, prev_was_const = 0;
	unsigned x = 0, len = my_strlen(infix->str);
	for (; x < len; ++x)
	{
		unsigned position = infix->positions[x], func = is_function(&infix->str[x]), constant = is_constant(&infix->str[x]);
		/* Check for a function and skip it if it exists */
		if (func)
		{
			if ((x + func < len && infix->str[x + func] != '(') || x + func >= len)
			{
				DiceErrPos = infix->positions[x + func >= len ? len : x + func];
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_NOPARENAFTERFUNCTION));
				return 0;
			}
			x += func - 1;
			prev_was_func = 1;
			continue;
		}
		/* Check for a constant, and skip it if it exists */
		if (constant)
		{
			x += constant - 1;
			prev_was_const = 1;
			continue;
		}
		if (infix->str[x] == ',')
		{
			if (!x ? 1 : !is_number(infix->str[x - 1]) && infix->str[x - 1] != ')' && !prev_was_const)
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_BEFORECOMMA));
				return 0;
			}
			if (x == len - 1 ? 1 : !is_number(infix->str[x + 1]) && infix->str[x + 1] != '(' && infix->str[x + 1] != '_' && !is_constant(&infix->str[x + 1]) && !is_function(&infix->str[x + 1]))
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_AFTERCOMMA));
				return 0;
			}
		}
		else if (is_op_noparen(infix->str[x]))
		{
			if (!x ? 1 : !is_number(infix->str[x - 1]) && infix->str[x - 1] != ')' && !prev_was_const)
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_BEFOREOPERATOR));
				return 0;
			}
			if (x == len - 1 ? 1 : !is_number(infix->str[x + 1]) && infix->str[x + 1] != '(' && infix->str[x + 1] != '_' && !is_constant(&infix->str[x + 1]) && !is_function(&infix->str[x + 1]))
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_AFTEROPERATOR));
				return 0;
			}
		}
		else if (infix->str[x] == '(')
		{
			if (x && !is_op_noparen(infix->str[x - 1]) && infix->str[x - 1] != '(' && infix->str[x - 1] != ',' && !prev_was_func)
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_BEFOREOPENPAREN));
				return 0;
			}
			if (x != len - 1 && !is_number(infix->str[x + 1]) && infix->str[x + 1] != '(' && infix->str[x + 1] != '_' && !is_constant(&infix->str[x + 1]) && !is_function(&infix->str[x + 1]))
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_AFTEROPENPAREN));
				return 0;
			}
		}
		else if (infix->str[x] == ')')
		{
			if (x && !is_number(infix->str[x - 1]) && infix->str[x - 1] != ')' && !prev_was_const)
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_BEFORECLOSEPAREN));
				return 0;
			}
			if (x != len - 1 && !is_op_noparen(infix->str[x + 1]) && infix->str[x + 1] != ')' && infix->str[x + 1] != ',')
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_AFTERCLOSEPAREN));
				return 0;
			}
		}
		else if (!is_number(infix->str[x]) && !is_muldiv(infix->str[x]) && !is_plusmin(infix->str[x]) && !is_operator(infix->str[x]) && infix->str[x] != '_')
		{
			DiceErrPos = position;
			DiceErrCode = DICE_ERR_PARSE;
			snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_INVALIDCHAR));
			return 0;
		}
		prev_was_func = prev_was_const = 0;
	}
	return 1;
}

/** Tokenize an infix notation equation by adding spaces between operators.
 * @param infix The original infix notation equation to tokenize
 * @return A new infix notation equation with spaces between operators
 */
static struct Infix *tokenize_infix(const struct Infix *infix)
{
	struct Infix *newinfix = create_infix();
	unsigned x = 0, len = my_strlen(infix->str);
	for (; x < len; ++x)
	{
		unsigned position = infix->positions[x], func = is_function(&infix->str[x]), constant = is_constant(&infix->str[x]);
		char curr = infix->str[x];
		if (func)
		{
			unsigned y = x;
			if (x && newinfix->str && *newinfix->str && newinfix->str[strlen(newinfix->str) - 1] != ' ')
			{
				my_strccat(&newinfix->str, ' ');
				add_to_infix(newinfix, position);
			}
			for (; y < x + func; ++y)
			{
				my_strccat(&newinfix->str, infix->str[y]);
				add_to_infix(newinfix, infix->positions[y]);
			}
			if (x != len - 1)
			{
				my_strccat(&newinfix->str, ' ');
				add_to_infix(newinfix, infix->positions[x + func]);
			}
			x += func - 1;
		}
		else if (constant)
		{
			unsigned y = x;
			if (x && newinfix->str && *newinfix->str && newinfix->str[strlen(newinfix->str) - 1] != ' ' && newinfix->str[strlen(newinfix->str) - 1] != '_')
			{
				my_strccat(&newinfix->str, ' ');
				add_to_infix(newinfix, position);
			}
			for (; y < x + constant; ++y)
			{
				my_strccat(&newinfix->str, infix->str[y]);
				add_to_infix(newinfix, infix->positions[y]);
			}
			if (x != len - 1)
			{
				my_strccat(&newinfix->str, ' ');
				add_to_infix(newinfix, infix->positions[x + constant]);
			}
			x += constant - 1;
		}
		else if (curr == ',')
		{
			if (x && newinfix->str && *newinfix->str && newinfix->str[strlen(newinfix->str) - 1] != ' ')
			{
				my_strccat(&newinfix->str, ' ');
				add_to_infix(newinfix, position);
			}
			my_strccat(&newinfix->str, ',');
			add_to_infix(newinfix, position);
			if (x != len - 1)
			{
				my_strccat(&newinfix->str, ' ');
				add_to_infix(newinfix, position);
			}
		}
		else if (is_operator(curr))
		{
			if (x && newinfix->str && *newinfix->str && newinfix->str[strlen(newinfix->str) - 1] != ' ')
			{
				my_strccat(&newinfix->str, ' ');
				add_to_infix(newinfix, position);
			}
			my_strccat(&newinfix->str, curr);
			add_to_infix(newinfix, position);
			if (x != len - 1)
			{
				my_strccat(&newinfix->str, ' ');
				add_to_infix(newinfix, position);
			}
		}
		else
		{
			my_strccat(&newinfix->str, curr);
			add_to_infix(newinfix, position);
		}
	}
	return newinfix;
}

/** Enumeration for PostfixValue to determine it's type */
enum PostfixValueType
{
	POSTFIX_VALUE_DOUBLE,
	POSTFIX_VALUE_STRING
};

struct PostfixValue
{
	void *val;
	enum PostfixValueType type;
};

struct Postfix
{
	struct PostfixValue *postfix;
	unsigned postfix_len;
};

/** Clears the memory of the given Postfix structure.
 * @param postfix The Postfix structure to clean up
 */
static void cleanup_postfix(struct Postfix *postfix)
{
	if (postfix->postfix_len)
	{
		unsigned x = 0;
		for (; x < postfix->postfix_len; ++x)
			free(postfix->postfix[x].val);
		free(postfix->postfix);
	}
	postfix->postfix = NULL;
	postfix->postfix_len = 0;
}
/** Adds a double value to the Postfix structure.
 * @param postfix The Postfix structure to add to
 * @param dbl The double value we are adding
 */
static void add_dbl_to_postfix(struct Postfix *postfix, double dbl)
{
	double *dbl_ptr = (double *)smalloc(sizeof(double));
	postfix->postfix = (struct PostfixValue *)srealloc(postfix->postfix, sizeof(struct PostfixValue) * (postfix->postfix_len + 1));
	*dbl_ptr = dbl;
	postfix->postfix[postfix->postfix_len].val = dbl_ptr;
	postfix->postfix[postfix->postfix_len++].type = POSTFIX_VALUE_DOUBLE;
}
/** Adds a string value to the Postfix structure.
 * @param postfix The Postfix structure to add to
 * @param str The string value we are adding
 */
static void add_str_to_postfix(struct Postfix *postfix, const char *str)
{
	char *str_ptr = (char *)smalloc(strlen(str) + 1);
	postfix->postfix = (struct PostfixValue *)srealloc(postfix->postfix, sizeof(struct PostfixValue) * (postfix->postfix_len + 1));
	strcpy(str_ptr, str);
	postfix->postfix[postfix->postfix_len].val = str_ptr;
	postfix->postfix[postfix->postfix_len++].type = POSTFIX_VALUE_STRING;
}

/** A makeshift stack of characters using a string to simluate something like std::stack without using C++ (not exactly like std::stack though). */
static struct my_cstack
{
	char *stack;
	unsigned stacklen;
} cstack = {NULL, 0};

/** Clean up the character stack.
 *
 * This will free the memory of the stack, putting it in an empty and initialized state.
 */
static void cleanup_cstack()
{
	if (cstack.stack)
		free(cstack.stack);
	cstack.stack = NULL;
	cstack.stacklen = 0;
}
/** Add the given string to the character stack.
 * @param str The string to add
 */
static void add_to_cstack(const char *str)
{
	if (cstack.stacklen)
		my_strccat(&cstack.stack, ' ');
	my_strcat(&cstack.stack, str);
	++cstack.stacklen;
}
/** Get the top of the character stack.
 * @return The top of the character stack, or NULL if the stack is empty
 *
 * NOTE: It is up to the caller of this function to free the memory used by the return value.
 */
static char *last_on_cstack()
{
	return cstack.stacklen ? myStrGetToken(cstack.stack, ' ', cstack.stacklen - 1) : NULL;
}
/** Remove the top of the character stack. */
static void remove_from_cstack()
{
	char *newcstack = NULL, *tok;
	unsigned curr = 0;
	if (!cstack.stacklen)
		return;
	while ((tok = myStrGetToken(cstack.stack, ' ', curr)))
	{
		if (curr != cstack.stacklen - 1)
		{
			if (curr)
				my_strccat(&newcstack, ' ');
			my_strcat(&newcstack, tok);
		}
		free(tok);
		++curr;
	}
	my_strcpy(&cstack.stack, newcstack);
	free(newcstack);
	--cstack.stacklen;
}

/** Convert an infix notation equation to a postfix notation equation, using the shunting-yard algorithm.
 * @param infix The infix notation equation to convert
 * @return A postfix notation equation
 *
 * Numbers are always stored in the postfix notation equation immediately, and operators are kept on a stack until they are
 * needed to be added to the postfix notation equation.
 * The conversion process goes as follows:
 * - Firstly, verify that the end of the input is a number, ignoring close parentheses.
 * - Iterate through the infix notation equation, doing the following on each operation:
 *   - When a _ is encountered, add the number following it to the postfix notation equation, but make sure it's negative.
 *   - When a number is encountered, add it to the postfix notation equation.
 *   - When a function is encountered, add it to the operator stack.
 *   - When a constant is encountered, convert it to a number and add it to the postfix notation equation.
 *   - When an operator is encountered:
 *     - Check if we had any numbers prior to the operator, and fail if there were none.
 *     - Always add open parentheses to the operator stack.
 *     - When a close parenthesis is encountered, pop all operators until we get to an open parenthesis or the stack becomes empty, failing on the latter.
 *     - For all other operators, pop the stack if needed then add the operator to the stack.
 *   - When a comma is encountered, do the same as above for when a close parenthesis is encountered, but also check to make sure there was a function prior to the open parenthesis (if there is one).
 *   - If anything else is encountered, fail as it is an invalid value.
 * - If there were operators left on the operator stack, pop all of them, failing if anything is left on the stack (an open parenthesis will cause this).
 */
static struct Postfix *infix_to_postfix(const struct Infix *infix)
{
	unsigned len = my_strlen(infix->str), x = 0, num = 0;
	int prev_was_close = 0, prev_was_number = 0;
	char *token, *lastone = NULL;
	struct Postfix *postfix = (struct Postfix *)scalloc(1, sizeof(struct Postfix));
	cleanup_cstack();
	cleanup_postfix(postfix);
	while ((token = myStrGetToken(infix->str, ' ', num)))
	{
		if (token[0] == '_')
		{
			double number = 0.0;
			if (is_constant(&token[1]))
			{
				if (!stricmp(token + 1, "e"))
					number = -exp(1.0);
				else if (!stricmp(token + 1, "pi"))
					number = -atan(1.0) * 4;
			}
			else
				number = -atof(token + 1);
			if (is_infinite(number) || is_notanumber(number))
			{
				DiceErrCode = is_infinite(number) ? DICE_ERR_OVERUNDERFLOW : DICE_ERR_UNDEFINED;
				free(token);
				if (lastone)
					free(lastone);
				cleanup_postfix(postfix);
				free(postfix);
				cleanup_cstack();
				return NULL;
			}
			add_dbl_to_postfix(postfix, number);
			prev_was_number = 1;
		}
		else if (is_number(token[0]))
		{
			double number = atof(token);
			if (is_infinite(number) || is_notanumber(number))
			{
				DiceErrCode = is_infinite(number) ? DICE_ERR_OVERUNDERFLOW : DICE_ERR_UNDEFINED;
				free(token);
				if (lastone)
					free(lastone);
				cleanup_postfix(postfix);
				free(postfix);
				cleanup_cstack();
				return NULL;
			}
			add_dbl_to_postfix(postfix, number);
			prev_was_number = 1;
		}
		else if (is_function(token))
			add_to_cstack(token);
		else if (is_constant(token))
		{
			double number = 0.0;
			if (!stricmp(token, "e"))
				number = exp(1.0);
			else if (!stricmp(token, "pi"))
				number = atan(1.0) * 4;
			add_dbl_to_postfix(postfix, number);
			prev_was_number = 1;
		}
		else if (is_operator(token[0]))
		{
			lastone = last_on_cstack();
			if (!prev_was_number && token[0] != '(' && token[0] != ')' && !prev_was_close)
			{
				DiceErrPos = infix->positions[x];
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_NONUMBERBEFOREOPERATOR));
				free(token);
				if (lastone)
					free(lastone);
				cleanup_postfix(postfix);
				free(postfix);
				cleanup_cstack();
				return NULL;
			}
			prev_was_number = 0;
			if (token[0] == '(')
			{
				add_to_cstack(token);
				prev_was_close = 0;
			}
			else if (token[0] == ')')
			{
				while (would_pop(token, lastone))
				{
					add_str_to_postfix(postfix, lastone);
					remove_from_cstack();
					if (lastone)
						free(lastone);
					lastone = last_on_cstack();
				}
				if ((lastone && lastone[0] != '(') || !lastone)
				{
					DiceErrPos = infix->positions[x];
					DiceErrCode = DICE_ERR_PARSE;
					snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_NOTENOUGHOPENPARENS));
					free(token);
					if (lastone)
						free(lastone);
					cleanup_postfix(postfix);
					free(postfix);
					cleanup_cstack();
					return NULL;
				}
				else
					remove_from_cstack();
				prev_was_close = 1;
			}
			else
			{
				if (!would_pop(token, lastone))
					add_to_cstack(token);
				else
				{
					while (would_pop(token, lastone))
					{
						add_str_to_postfix(postfix, lastone);
						remove_from_cstack();
						if (lastone)
							free(lastone);
						lastone = last_on_cstack();
					}
					add_to_cstack(token);
				}
				prev_was_close = 0;
			}
		}
		else if (token[0] == ',')
		{
			if (lastone)
				free(lastone);
			lastone = last_on_cstack();
			while (would_pop(token, lastone))
			{
				add_str_to_postfix(postfix, lastone);
				remove_from_cstack();
				if (lastone)
					free(lastone);
				lastone = last_on_cstack();
			}
			if ((lastone && lastone[0] != '(') || !lastone)
			{
				DiceErrPos = infix->positions[x];
				DiceErrCode = DICE_ERR_PARSE;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_INVALIDCOMMA));
				free(token);
				if (lastone)
					free(lastone);
				cleanup_postfix(postfix);
				free(postfix);
				cleanup_cstack();
				return NULL;
			}
			else
			{
				char *paren = lastone;
				remove_from_cstack();
				lastone = last_on_cstack();
				if (!lastone || !is_function(lastone))
				{
					DiceErrPos = infix->positions[x];
					DiceErrCode = DICE_ERR_PARSE;
					snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_INVALIDCOMMA));
					free(token);
					if (lastone)
						free(lastone);
					free(paren);
					cleanup_postfix(postfix);
					free(postfix);
					cleanup_cstack();
					return NULL;
				}
				else
				{
					add_to_cstack(paren);
					free(paren);
				}
				free(lastone);
			}
		}
		else
		{
			DiceErrPos = infix->positions[x];
			DiceErrCode = DICE_ERR_PARSE;
			snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_INVALIDCHAR));
			free(token);
			if (lastone)
				free(lastone);
			cleanup_postfix(postfix);
			free(postfix);
			cleanup_cstack();
			return NULL;
		}
		x += strlen(token) + (x ? 1 : 0);
		free(token);
		++num;
	}
	if (cstack.stacklen)
	{
		if (lastone)
			free(lastone);
		lastone = last_on_cstack();
		while (would_pop(NULL, lastone))
		{
			add_str_to_postfix(postfix, lastone);
			remove_from_cstack();
			if (lastone)
				free(lastone);
			lastone = last_on_cstack();
			if (!cstack.stacklen)
				break;
		}
		if (lastone)
		{
			DiceErrPos = infix->positions[len];
			DiceErrCode = DICE_ERR_PARSE;
			snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_TOOMANYOPENPARENS));
			free(lastone);
			cleanup_postfix(postfix);
			free(postfix);
			cleanup_cstack();
			return NULL;
		}
	}
	cleanup_cstack();
	if (lastone)
		free(lastone);
	return postfix;
}

/** A makeshift stack of double-precision floating points to simluate something like std::stack without using C++ (not exactly like std::stack though). */
static struct my_dstack
{
	double *stack;
	unsigned stacklen;
} dstack = {NULL, 0};

/** Clean up the character stack.
 *
 * This will free the memory of the stack, putting it in an empty and initialized state.
 */
static void cleanup_dstack()
{
	if (dstack.stack)
		free(dstack.stack);
	dstack.stack = NULL;
	dstack.stacklen = 0;
}
/** Add the given value to the character stack.
 * @param val The value to add
 */
static void add_to_dstack(double val)
{
	dstack.stack = (double *)srealloc(dstack.stack, sizeof(double) * (dstack.stacklen + 1));
	dstack.stack[dstack.stacklen++] = val;
}
/** Remove the top of the double stack.
 * @return The top value of the stack
 *
 * std::stack returns nothing on pop, whereas this function does return a value.
 */
static double remove_from_dstack()
{
	double val = 0;
	if (!dstack.stacklen)
		return 0;
	val = dstack.stack[dstack.stacklen - 1];
	dstack.stack = (double *)srealloc(dstack.stack, sizeof(double) * --dstack.stacklen);
	return val;
}

/** Evaluate a postfix notation equation.
 * @param The postfix notation equation to evaluate
 * @return The final result after calcuation of the equation
 *
 * The evaluation pops a single value from the operand stack for a function, and 2 values from the operand stack for an operator. The result
 * of either one is placed back on the operand stack, hopefully leaving a single result at the end.
 */
static double eval_postfix(const struct Postfix *postfix)
{
	double val = 0;
	unsigned x = 0;
	cleanup_dstack();
	for (; x < postfix->postfix_len; ++x)
	{
		if (postfix->postfix[x].type == POSTFIX_VALUE_STRING)
		{
			const char *token = (const char *)postfix->postfix[x].val;
			if (!token || !*token)
			{
				DiceErrCode = DICE_ERR_STACK;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_EMPTYTOKEN));
				cleanup_dstack();
				return 0;
			}
			if (is_function(token))
			{
				unsigned function_arguments = function_argument_count(token);
				double val1;
				if (dstack.stacklen < function_arguments)
				{
					DiceErrCode = DICE_ERR_STACK;
					snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_NOTENOUGHFUNCTIONARGS));
					return 0;
				}
				val1 = remove_from_dstack();
				if (!stricmp(token, "abs"))
					val = fabs(val1);
				else if (!stricmp(token, "acos"))
				{
					if (fabs(val1) > 1)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						cleanup_dstack();
						return 0;
					}
					val = acos(val1);
				}
				else if (!stricmp(token, "acosh"))
				{
					if (val1 < 1)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						cleanup_dstack();
						return 0;
					}
					val = acosh(val1);
				}
				else if (!stricmp(token, "asin"))
				{
					if (fabs(val1) > 1)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						cleanup_dstack();
						return 0;
					}
					val = asin(val1);
				}
				else if (!stricmp(token, "asinh"))
					val = asinh(val1);
				else if (!stricmp(token, "atan"))
					val = atan(val1);
				else if (!stricmp(token, "atanh"))
				{
					if (fabs(val1) >= 1)
					{
						DiceErrCode = fabs(val1) == 1 ? DICE_ERR_DIV0 : DICE_ERR_UNDEFINED;
						cleanup_dstack();
						return 0;
					}
					val = atanh(val1);
				}
				else if (!stricmp(token, "cbrt"))
					val = cbrt(val1);
				else if (!stricmp(token, "ceil"))
					val = ceil(val1);
				else if (!stricmp(token, "cos"))
					val = cos(val1);
				else if (!stricmp(token, "cosh"))
					val = cosh(val1);
				else if (!stricmp(token, "deg"))
					val = val1 * 45.0 / atan(1.0);
				else if (!stricmp(token, "exp"))
					val = exp(val1);
				else if (!stricmp(token, "fac"))
				{
					unsigned n;
					if ((int)val1 < 0)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						cleanup_dstack();
						return 0;
					}
					val = 1;
					for (n = 2; n <= (unsigned)val1; ++n)
						val *= n;
				}
				else if (!stricmp(token, "floor"))
					val = floor(val1);
				else if (!stricmp(token, "log"))
				{
					if (val1 <= 0)
					{
						DiceErrCode = DICE_ERR_DIV0;
						cleanup_dstack();
						return 0;
					}
					val = log(val1);
				}
				else if (!stricmp(token, "log10"))
				{
					if (val1 <= 0)
					{
						DiceErrCode = DICE_ERR_DIV0;
						cleanup_dstack();
						return 0;
					}
					val = log10(val1);
				}
				else if (!stricmp(token, "max"))
				{
					double val2 = val1;
					val1 = remove_from_dstack();
					val = val1 > val2 ? val1 : val2;
				}
				else if (!stricmp(token, "min"))
				{
					double val2 = val1;
					val1 = remove_from_dstack();
					val = val1 < val2 ? val1 : val2;
				}
				else if (!stricmp(token, "rad"))
					val = val1 * atan(1.0) / 45.0;
				else if (!stricmp(token, "rand"))
				{
					char buf[40] = "";
					double val2 = val1;
					val1 = remove_from_dstack();
					if (val1 > val2)
					{
						double tmp = val2;
						val2 = val1;
						val1 = tmp;
					}
					val = Mersenne_IRandom((int)val1, (int)val2);
					snprintf(buf, 40, "rand(%d,%d)=(%d) ", (int)val1, (int)val2, (int)val);
					if (my_strlen(DiceShortExOutput) <= DICE_MAX_EX_SIZE)
						my_strcat(&DiceShortExOutput, buf);
					if (my_strlen(DiceExOutput) <= DICE_MAX_EX_SIZE)
						my_strcat(&DiceExOutput, buf);
				}
				else if (!stricmp(token, "round"))
					val = my_round(val1, 0);
				else if (!stricmp(token, "sin"))
					val = sin(val1);
				else if (!stricmp(token, "sinh"))
					val = sinh(val1);
				else if (!stricmp(token, "sqrt"))
				{
					if (val1 < 0)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						cleanup_dstack();
						return 0;
					}
					val = sqrt(val1);
				}
				else if (!stricmp(token, "tan"))
				{
					if (!fmod(val1 + 2 * atan(1.0), atan(1.0) * 4))
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						cleanup_dstack();
						return 0;
					}
					val = tan(val1);
				}
				else if (!stricmp(token, "tanh"))
					val = tanh(val1);
				else if (!stricmp(token, "trunc"))
					val = trunc(val1);
				if (is_infinite(val) || is_notanumber(val))
				{
					DiceErrCode = is_infinite(val) ? DICE_ERR_OVERUNDERFLOW : DICE_ERR_UNDEFINED;
					cleanup_dstack();
					return 0;
				}
				add_to_dstack(val);
			}
			else if (is_operator(token[0]) && strlen(token) == 1)
			{
				double val1, val2;
				if (dstack.stacklen < 2)
				{
					DiceErrCode = DICE_ERR_STACK;
					snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_NOTENOUGHOPERATORARGS));
					cleanup_dstack();
					return 0;
				}
				val2 = remove_from_dstack();
				val1 = remove_from_dstack();
				switch (token[0])
				{
					case '+':
						val = val1 + val2;
						break;
					case '-':
						val = val1 - val2;
						break;
					case '*':
						val = val1 * val2;
						break;
					case '/':
						if (!val2)
						{
							DiceErrCode = DICE_ERR_DIV0;
							cleanup_dstack();
							return 0;
						}
						val = val1 / val2;
						break;
					case '%':
						if (!val2)
						{
							DiceErrCode = DICE_ERR_DIV0;
							cleanup_dstack();
							return 0;
						}
						val = fmod(val1, val2);
						break;
					case '^':
						if (val1 < 0 && (double)((int)val2) != val2)
						{
							DiceErrCode = DICE_ERR_UNDEFINED;
							cleanup_dstack();
							return 0;
						}
						if (!val1 && !val2)
						{
							DiceErrCode = DICE_ERR_DIV0;
							cleanup_dstack();
							return 0;
						}
						if (!val1 && val2 < 0)
						{
							DiceErrCode = DICE_ERR_OVERUNDERFLOW;
							cleanup_dstack();
							return 0;
						}
						val = pow(val1, val2);
						break;
					case 'd':
						if (val1 < 1 || val1 > DICE_MAX_DICE)
						{
							DiceErrCode = DICE_ERR_UNACCEPT_DICE;
							DiceErrNum = (int)val1;
							cleanup_dstack();
							return 0;
						}
						if (val2 < 1 || val2 > DICE_MAX_SIDES)
						{
							DiceErrCode = DICE_ERR_UNACCEPT_SIDES;
							DiceErrNum = (int)val2;
							cleanup_dstack();
							return 0;
						}
						val = (double)Dice((int)val1, (unsigned)val2);
				}
				if (is_infinite(val) || is_notanumber(val))
				{
					DiceErrCode = is_infinite(val) ? DICE_ERR_OVERUNDERFLOW : DICE_ERR_UNDEFINED;
					cleanup_dstack();
					return 0;
				}
				add_to_dstack(val);
			}
		}
		else
		{
			double *val_ptr = (double *)postfix->postfix[x].val;
			if (!val_ptr)
			{
				DiceErrCode = DICE_ERR_STACK;
				snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_EMPTYNUMBER));
				cleanup_dstack();
				return 0;
			}
			add_to_dstack(*val_ptr);
		}
	}
	val = remove_from_dstack();
	if (dstack.stacklen)
	{
		DiceErrCode = DICE_ERR_STACK;
		snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_TOOMANYNUMBERS));
		val = 0;
	}
	cleanup_dstack();
	return val;
}

/** Parse an infix notation expression and convert the expression to postfix notation.
 * @param infix The original expression, in infix notation, to convert to postfix notation
 * @return A postfix notation expression equivilent to the infix notation expression given, or NULL if the infix notation expression could not be parsed or converted
 *
 * NOTE: It is up to the caller of this function to free the memory used by the return value.
 */
static struct Postfix *DoParse(const char *infix)
{
	struct Infix *infixcpy = fix_infix(infix), *tokenized_infix;
	struct Postfix *postfix;
	if (!infixcpy)
		return NULL;
	if (!check_infix(infixcpy))
	{
		cleanup_infix(infixcpy);
		return NULL;
	}
	tokenized_infix = tokenize_infix(infixcpy);
	if (!tokenized_infix)
	{
		cleanup_infix(infixcpy);
		return NULL;
	}
	postfix = infix_to_postfix(tokenized_infix);
	cleanup_infix(infixcpy);
	cleanup_infix(tokenized_infix);
	return postfix;
}

/** Evaluate a postfix notation expression.
 * @param postfix The postfix notation expression to evaluate
 * @return The final result after evaluation
 */
static double DoEvaluate(const struct Postfix *postfix)
{
	double ret;
	DiceErrCode = DICE_ERR_NONE;
	DiceErrPos = 0;
	if (DiceExOutput)
		free(DiceExOutput);
	if (DiceShortExOutput);
		free(DiceShortExOutput);
	DiceExOutput = DiceShortExOutput = NULL;
	ret = eval_postfix(postfix);
	if (ret > INT_MAX || ret < INT_MIN)
		DiceErrCode = DICE_ERR_OVERUNDERFLOW;
	return ret;
}

/** DiceServ's error handler, if any step along the way fails, this will display the cause of the error to the user.
 * @param u The user that invoked DiceServ
 * @param dice The dice expression that was used to invoke DiceServ
 * @param ErrorPos The position of the error (usually the same DiceErrPos, but not always)
 * @return 1 in almost every case to signify there was an error, 0 if an unknown error code was passed in or no error code was set
 */
static int ErrorHandler(User *u, const char *dice, unsigned ErrorPos)
{
	int WasError = 1;
	switch (DiceErrCode)
	{
		case DICE_ERR_PARSE:
		{
			char spaces[460] = "";
			moduleNoticeLang(s_DiceServ, u, DICE_PARSE_ERROR);
			notice_user(s_DiceServ, u, " %s", dice);
			memset(spaces, ' ', ErrorPos > strlen(dice) ? strlen(dice) : ErrorPos);
			notice_user(s_DiceServ, u, "(%s^)", spaces);
			moduleNoticeLang(s_DiceServ, u, DICE_ERRSTR);
			notice_user(s_DiceServ, u, "%s", DiceErrStr);
			break;
		}
		case DICE_ERR_DIV0:
			moduleNoticeLang(s_DiceServ, u, DICE_ERROR_DIVBYZERO);
			notice_user(s_DiceServ, u, " %s", dice);
			break;
		case DICE_ERR_UNDEFINED:
			moduleNoticeLang(s_DiceServ, u, DICE_ERROR_UNDEFINED);
			notice_user(s_DiceServ, u, " %s", dice);
			break;
		case DICE_ERR_UNACCEPT_DICE:
			if (DiceErrNum <= 0)
				moduleNoticeLang(s_DiceServ, u, DICE_DICEUNDERLIMIT, DiceErrNum, DICE_MAX_DICE);
			else
				moduleNoticeLang(s_DiceServ, u, DICE_DICEOVERLIMIT, DiceErrNum, DICE_MAX_DICE);
			break;
		case DICE_ERR_UNACCEPT_SIDES:
			if (DiceErrNum <= 0)
				moduleNoticeLang(s_DiceServ, u, DICE_SIDESUNDERLIMIT, DiceErrNum, DICE_MAX_SIDES);
			else
				moduleNoticeLang(s_DiceServ, u, DICE_SIDESOVERLIMIT, DiceErrNum, DICE_MAX_SIDES);
			break;
		case DICE_ERR_OVERUNDERFLOW:
			moduleNoticeLang(s_DiceServ, u, DICE_OVERUNDERFLOW);
			notice_user(s_DiceServ, u, " %s", dice);
			break;
		case DICE_ERR_STACK:
			moduleNoticeLang(s_DiceServ, u, DICE_STACK_ERR);
			notice_user(s_DiceServ, u, " %s", dice);
			moduleNoticeLang(s_DiceServ, u, DICE_ERRSTR);
			notice_user(s_DiceServ, u, "%s", DiceErrStr);
			break;
		default:
			WasError = 0;
	}
	return WasError;
}

/** Find the lowest result out of the 4 6-sided dice thrown for a DnD3e character.
 * @param themin Pointer to an integer storing the lowest result
 * @return The index in DnDresult of the lowest result
 */
static unsigned GetMinDnD(unsigned *themin)
{
	unsigned x = 1, minpos = 0, min = DnDresults[0];
	for (; x < 4; ++x)
		if (DnDresults[x] < min)
		{
			minpos = x;
			min = DnDresults[x];
		}
	*themin = min;
	return minpos;
}

/** Surround the lowest value with reverse control codes in the extended roll string for DnD3e character creation, and subtract the lowest value from the total score.
 * @param v Pointer to the original score
 */
static void DnDRollCorrect(double *v)
{
	unsigned min, minpos = GetMinDnD(&min), x = 0, y = 0, len = my_strlen(DiceExOutput);
	int inex = 0;
	char *tmpstr = NULL;
	for (; x < len; ++x)
	{
		char curr = DiceExOutput[x];
		if (x && DiceExOutput[x - 1] == '(')
			inex = 1;
		if (inex && y == minpos && curr != ' ' && curr != ')')
			my_strccat(&tmpstr, 22);
		my_strccat(&tmpstr, curr);
		if (inex && y == minpos && curr != ' ' && curr != ')')
			my_strccat(&tmpstr, 22);
		if (inex && curr == ' ')
			++y;
		if (curr == ')')
			inex = 0;
	}
	*v -= min;
	my_strcpy(&DiceExOutput, tmpstr);
	free(tmpstr);
}

/** Calculate the sum of all the modifier values of all the rolls for DnD3e character creation.
 * @return The sum of the modifiers
 */
static int DnDmodadd()
{
	int mods = 0, x = 0;
	for (; x < 6; ++x)
		mods += DnDmods[x][1];
	return mods;
}

/** Determine the highest roll of all the rolls for DnD3e character creation.
 * @return The highest value
 */
static int DnDmaxatt()
{
	int x = 1, maxatt = DnDmods[0][0];
	for (; x < 6; ++x)
		if (maxatt < DnDmods[x][0])
			maxatt = DnDmods[x][0];
	return maxatt;
}

/** Get the syntax string for the given dice type.
 * @param type The dice type
 * @return The syntax string for the type
 */
static int syntax_string(enum DICE_TYPES type)
{
	switch (type)
	{
		case DICE_TYPE_CALC:
			return DICE_CALC_SYNTAX;
		case DICE_TYPE_EXROLL:
			return DICE_EXROLL_SYNTAX;
		case DICE_TYPE_EXCALC:
			return DICE_EXCALC_SYNTAX;
		case DICE_TYPE_DND3E:
			return DICE_DND3ECHAR_SYNTAX;
		case DICE_TYPE_EARTHDAWN:
			return DICE_EARTHDAWN_SYNTAX;
		default:
			return DICE_ROLL_SYNTAX;
	}
}

/** DiceServ's core routine, parses the results and then sends them to their destination.
 * @param u The user that invoked DiceServ
 * @param origdice The dice roll expression as given by the user
 * @param chan The channel, if any, that the results should be anounced to
 * @param comment The comment, if any, that should accompany the results
 * @param type The type of roll being done, see DICE_TYPES enumeration above
 * @param round_result Determines if the final results should be rounded or not
 *
 * This is where the given arguments are validated, and where multiple roll sets are handled (if a ~ was in the dice roll expression). This also
 * checks for DiceServ ignore values on either users/nicks or channels.
 */
static void diceserv_roller(User *u, const char *origdice, const char *chan, const char *comment, enum DICE_TYPES type, int round_result)
{
	char	*buf = NULL, /* Buffer to store output */
			commentstr[511] = "", /* Full comment, constructed from given arguments */
			dice[511] = "", /* Stores a copy of the dice to use */
			tmp[33] = "", /* Temporary stroage for conversion from double to string */
			*extmp = NULL, /* Temporary storage for extended result buffer */
			*shortextmp = NULL, /* Temporary storage for shorter extended result buffer */
			*origdicecpy = NULL, /* Copy of the original dice string */
			chancpy[511] = "", /* Copy of the channel string */
			commentcpy[511] = "", /* Copy of the comment string */
			sep[2], /* Separator between extended output sets */
			*ignore; /* Stores the diceserv_ignore key from the module data */
	struct Postfix *dice_postfix; /* The dice value from above in postfix notation */
	int	inchan = 1, /* 1 if the result is to go to a channel, 0 otherwise */
		chaniscomm = 0, /* 1 if the channel should actually be part of the comment, 0 if it should be treated as a channel name */
		error = 0, /* 1 when there really is an error, 0 if there is no error */
		n, /* Temporary counter for number of sets to roll */
		l = -1, /* Length of string up to ~, if any */
		DnDmodpos = 0, /* Current position within the DnDmods array */
		DnDcontinue, /* 1 when the dice need to be rerolled for a DnD3e character, if the sum of their modifiers was 0 or less, or if their highest score was a 13 or less */
		oldn = 0, /* Stores the original value of the counter for number of sets to roll */
		MaxMessageLength = 510; /* Stores the maximum length of the dynamic parts of the buffer, calculated below */
	unsigned step = 0; /* Used with Earthdawn, the step that the user requested the roll for */
	double v; /* Stores the result of the dice expression */
	/* Reset variables, and set DiceEx depending on if we are requesting an extended dice roll or not */
	DiceEx = type == DICE_TYPE_ROLL || type == DICE_TYPE_CALC ? 0 : 1;
	DiceErrCode = DICE_ERR_NONE;
	DiceErrPos = DnDPos = 0;
	DiceErrNum = DoDnD3eChar = DoEarthdawn = 0;
	if (DiceExOutput)
		free(DiceExOutput);
	if (DiceShortExOutput)
		free(DiceShortExOutput);
	DiceExOutput = DiceShortExOutput = NULL;
	CurrUser = u;
	/* Check for a ignore on the user or their registered nick, if any, and deny them access if they are ignored */
	ignore = moduleGetData(&u->moduleData, "diceserv_ignore");
	if (ignore)
	{
		free(ignore);
		return;
	}
	if (u->na)
	{
		ignore = moduleGetData(&u->na->nc->moduleData, "diceserv_ignore");
		if (ignore)
		{
			free(ignore);
			return;
		}
	}
	/* Check if there was even a dice argument given for all rolls except for the DnD3e character creation roll */
	if (type != DICE_TYPE_DND3E && !origdice)
	{
		if (!SourceIsBot)
			moduleNoticeLang(s_DiceServ, u, syntax_string(type));
		return;
	}
	/* Make copies of the passed arguments */
	if (origdice)
		my_strcpy(&origdicecpy, origdice);
	if (chan)
		snprintf(chancpy, sizeof(chancpy), "%s", chan);
	if (comment)
		snprintf(commentcpy, sizeof(commentcpy), "%s", comment);
	/* Always yse 6~4d6 (6 sets of 4d6) for a DnD3e character creation roll */
	if (type == DICE_TYPE_DND3E)
	{
		my_strcpy(&origdicecpy, "6~4d6");
		DoDnD3eChar = 1;
	}
	/* Set if an Earthdawn roll is requested */
	else if (type == DICE_TYPE_EARTHDAWN)
		DoEarthdawn = 1;
	/* If the channel doesn't start with #, we'll treat it as if it was part of the comment */
	if (chancpy[0] && *chancpy != '#')
	{
		chaniscomm = 1;
		inchan = 0;
	}
	else if (!chancpy[0])
		inchan = 0;
	/* If a channel was given, ignore the roll if the user isn't in the channel.
	 * Also, check if the channel has ignored rolls to it or if it's been moderated (+m) and the user has no status to the channel. */
	if (inchan)
	{
		Channel *c = findchan(chancpy);
		if (c)
		{
			int found_c_ignore;
			char *modes, *CMODE_m;
			ChannelInfo *ci;
			if (!is_on_chan(c, u))
			{
				if (!SourceIsBot)
				{
					notice_lang(s_DiceServ, u, CHAN_X_INVALID, chancpy);
					moduleNoticeLang(s_DiceServ, u, syntax_string(type));
				}
				return;
			}
			found_c_ignore = find_chan_ignore(c->name);
			ci = cs_findchan(chancpy);
			if (ci)
				ignore = moduleGetData(&ci->moduleData, "diceserv_ignore");
			else
				ignore = NULL;
			modes = chan_get_modes(c, 0, 1);
			CMODE_m = strchr(modes, 'm');
			if (found_c_ignore != -1 || ignore || (CMODE_m && !chan_get_user_status(c, u)))
				inchan = *chancpy = 0;
			if (ignore)
				free(ignore);
			if (!inchan && SourceIsBot)
				return;
		}
		else
			inchan = *chancpy = 0;
	}
	/* Generate the comment string */
	snprintf(commentstr, sizeof(commentstr), "%s%s%s", chancpy[0] && !inchan ? chancpy : "", chancpy[0] && !inchan ? " " : "", commentcpy[0] ? commentcpy : "");
	/* Calculate max message length */
	if (inchan && SourceIsBot)
	{
		BotInfo *bi = findbot(RollSource);
		MaxMessageLength -= strlen(bi->nick);
		MaxMessageLength -= strlen(bi->user);
		MaxMessageLength -= strlen(bi->host);
	}
	else
	{
		MaxMessageLength -= strlen(s_DiceServ);
		MaxMessageLength -= strlen(ServiceUser);
		MaxMessageLength -= strlen(ServiceHost);
	}
	MaxMessageLength -= 7; /* For the :, !, and @, plus the space after that and after the PRIVMSG/NOTICE and after the target and the : for the message */
	MaxMessageLength -= inchan && SourceIsBot ? 7 : 6; /* inchan with BotServ bot == PRIVMSG, otherwise NOTICE */
	MaxMessageLength -= inchan ? strlen(chancpy) : strlen(u->nick); /* inchan uses channel's name, otherwise uses user's nick */
	MaxMessageLength -= 7; /* For the < > [ ] :, and the space before the [ and after the : */
	MaxMessageLength -= strlen(origdicecpy); /* The dice expression itself */
	if (type == DICE_TYPE_ROLL || type == DICE_TYPE_CALC)
		MaxMessageLength -= 4; /* Roll or Calc */
	else if (type == DICE_TYPE_EXROLL || type == DICE_TYPE_EXCALC)
		MaxMessageLength -= 6; /* Exroll or Excalc */
	else if (type == DICE_TYPE_DND3E)
		MaxMessageLength -= 20; /* DnD3e Character roll */
	else if (type == DICE_TYPE_EARTHDAWN)
		MaxMessageLength -= 14; /* Earthdawn roll */
	if (inchan)
	{
		MaxMessageLength -= 4; /* "for " */
		MaxMessageLength -= strlen(u->nick); /* nick of user who used the command */
	}
	if (commentstr[0])
		MaxMessageLength -= strlen(commentstr) + 1; /* Comment plus space */
	/* Check for overflow prior to adding in dice results */
	if (MaxMessageLength <= 0)
	{
		moduleNoticeLang(s_DiceServ, u, DICE_BUFFER_OVERFLOW);
		return;
	}
	/* End calculating max message length */
	snprintf(sep, sizeof(sep), "%s", " "); /* Might change in the future if we need a different seperator */
	/* When a ~ is in the dice expression, the expression before the ~ will be parsed to determine the number of sets to throw, the actual set will be after the ~ */
	if (strchr(origdicecpy, '~'))
	{
		char *times = NULL;
		struct Postfix *times_postfix;
		/* Earthdawn rolls do not support using ~ syntax */
		if (type == DICE_TYPE_EARTHDAWN)
		{
			moduleNoticeLang(s_DiceServ, u, DICE_ERROR_EARTHDAWN_INPUT);
			return;
		}
		/* Extract the expression before the ~ into times, and place the expression after the ~ into dice, then parse the expression in times */
		my_strcpy(&times, origdicecpy);
		snprintf(dice, sizeof(dice), "%s", strchr(times, '~') + 1);
		l = strchr(times, '~') - times;
		times[l] = 0;
		if (!my_strlen(times) || !my_strlen(dice))
		{
			DiceErrCode = DICE_ERR_PARSE;
			snprintf(DiceErrStr, sizeof(DiceErrStr), "%s", moduleGetLangString(CurrUser, DICE_ERROR_EMPTYNUMBER));
			ErrorHandler(u, origdicecpy, l);
			free(times);
			return;
		}
		times_postfix = DoParse(times);
		free(times);
		/* If the parsing failed, display the error and leave */
		if (!times_postfix)
		{
			ErrorHandler(u, origdicecpy, DiceErrPos);
			return;
		}
		/* Evaulate the expression */
		v = DoEvaluate(times_postfix);
		cleanup_postfix(times_postfix);
		free(times_postfix);
		/* Check if the evaluated number of times is out of bounds */
		if (DiceErrCode == DICE_ERR_NONE)
		{
			oldn = n = (int)v;
			if (n > DICE_MAX_TIMES)
			{
				moduleNoticeLang(s_DiceServ, u, DICE_TIMESOVERLIMIT, n, DICE_MAX_TIMES);
				return;
			}
			else if (n <= 0)
			{
				moduleNoticeLang(s_DiceServ, u, DICE_TIMESUNDERLIMIT, n, DICE_MAX_TIMES);
				return;
			}
		}
		/* Check for errors */
		error = ErrorHandler(u, origdicecpy, DiceErrPos);
	}
	/* Otherwise, just set the dice, also checking specially for Earthdawn */
	else
	{
		/* For Earthdawn, find the step and set the dice expression */
		if (type == DICE_TYPE_EARTHDAWN)
		{
			char step_str[511], *plus;
			int num;
			snprintf(step_str, sizeof(step_str), "%s", origdicecpy);
			plus = strchr(step_str, '+');
			if (plus)
			{
				snprintf(dice, sizeof(dice), "%s", plus);
				*plus = 0;
			}
			num = atoi(step_str);
			dtoa(num, 32, tmp);
			if (strcmp(step_str, tmp))
			{
				moduleNoticeLang(s_DiceServ, u, DICE_ERROR_EARTHDAWN_INPUT);
				return;
			}
			if (num < 1 || num > 100)
			{
				moduleNoticeLang(s_DiceServ, u, DICE_ERROR_EARTHDAWN_RANGE, num);
				return;
			}
			step = num;
			free(origdicecpy);
			origdicecpy = NULL;
			if (*dice)
				my_strccat(&origdicecpy, '(');
			my_strcat(&origdicecpy, EarthdawnStepTable[step]);
			if (*dice)
			{
				my_strccat(&origdicecpy, ')');
				my_strcat(&origdicecpy, dice);
			}
		}
		snprintf(dice, sizeof(dice), "%s", origdicecpy ? origdicecpy : "");
		oldn = n = 1;
	}
	/* As long as there was no error, either roll the dice and stop, or if doing a DnD3e character, continue rolling dice as
	 * long as we get a set of 6 scores that have a max score of 13 or less, or have the sum of their modifiers be 0 or less */
	if (!error)
	{
		/* Parse the dice */
		dice_postfix = DoParse(dice);
		/* If the parsing failed, display the error and leave */
		if (!dice_postfix)
		{
			ErrorHandler(u, origdicecpy, l + DiceErrPos + 1);
			return;
		}
		do
		{
			/* Reset DnD3e values regardless, and set n to the original number of sets */
			DnDcontinue = DnDmodpos = 0;
			n = oldn;
			/* Roll as many sets as were requested, also resetting the DnD3e position after each roll */
			for (; n > 0; --n, DnDPos = 0)
			{
				/* Evaluate the dice, then check for errors */
				v = DoEvaluate(dice_postfix);
				error = ErrorHandler(u, origdicecpy, l + DiceErrPos + 1);
				/* As long as we didn't have an error, we will continue */
				if (!error)
				{
					/* If this is not the first set, add the seperator to the buffer */
					if (n != oldn)
						my_strcat(&buf, sep);
					/* Process extended output if we are still doing so */
					if (DiceEx == 1)
					{
						/* Make sure that we have some extended output first */
						if (DiceExOutput && *DiceExOutput)
						{
							/* Add a seperator on the extended output */
							if (n == oldn)
							{
								my_strcpy(&shortextmp, "{");
								my_strcpy(&extmp, "{");
							}
							else
							{
								my_strcat(&shortextmp, " | ");
								my_strcat(&extmp, " | ");
							}
							/* Remove the extra space at the end */
							DiceExOutput[strlen(DiceExOutput) - 1] = 0;
							DiceShortExOutput[strlen(DiceShortExOutput) - 1] = 0;
							/* If a DnD3e character roll was requested, correct the result and calculate the modifier */
							if (DiceEx == 1 && type == DICE_TYPE_DND3E)
							{
								DnDRollCorrect(&v);
								DnDmods[DnDmodpos][0] = (int)my_round(v, 0);
								DnDmods[DnDmodpos++][1] = (int)floor((v - 10) / 2);
							}
							/* Add the extended output string to the extended result buffer */
							my_strcat(&extmp, DiceExOutput);
							my_strcat(&shortextmp, DiceShortExOutput);
							/* Add a closing seperator on the last roll */
							if (n == 1)
							{
								my_strcat(&shortextmp, "} ");
								my_strcat(&extmp, "} ");
							}
						}
						/* Without extended output, we treat this as if there was no extended output requested */
						else
							DiceEx = 0;
					}
					/* Round the result and add it the buffer */
					dtoa(round_result ? (int)my_round(v, 0) : v, 32, tmp);
					my_strcat(&buf, tmp);
				}
				/* Leave if there was an error */
				else
					return;
			}
			/* If a DnD3e character roll was requested, and the sum of the modifiers is 0 or less, or the maximum score is 13 or less,
			 * we reset things and redo the rolls */
			if (type == DICE_TYPE_DND3E && (DnDmodadd() <= 0 || DnDmaxatt() <= 13))
			{
				moduleNoticeLang(s_DiceServ, u, DnDmodadd() <= 0 ? DICE_ERROR_DND3E_MODTOTALZERO : DICE_ERROR_DND3E_MAXSCORETHIRTEEN);
				DnDcontinue = 1;
				if (buf)
					free(buf);
				buf = NULL;
				if (DiceExOutput)
					free(DiceExOutput);
				if (DiceShortExOutput)
					free(DiceShortExOutput);
				DiceExOutput = DiceShortExOutput = NULL;
				if (extmp)
					free(extmp);
				if (shortextmp)
					free(shortextmp);
				extmp = shortextmp = NULL;
			}
		} while (DnDcontinue);
		cleanup_postfix(dice_postfix);
		free(dice_postfix);
	}
	/* Determine if we have any sort of overflow with the extended roll buffer or the full buffer */
	if (MaxMessageLength - (int)my_strlen(buf) >= 0)
	{
		if (DiceEx == 1 && MaxMessageLength - (int)my_strlen(buf) - (int)my_strlen(extmp) < 0)
			DiceEx = 2;
		if (DiceEx == 2 && MaxMessageLength - (int)my_strlen(buf) - (int)my_strlen(shortextmp) < 0)
			DiceEx = 3;
		if (DiceEx == 3)
			moduleNoticeLang(s_DiceServ, u, DICE_EX_OVERFLOW, type == DICE_TYPE_EXROLL ? "EXROLL" : (type == DICE_TYPE_EXCALC ? "EXCALC" : "DND3ECHAR"));
	}
	else
	{
		moduleNoticeLang(s_DiceServ, u, DICE_BUFFER_OVERFLOW);
		return;
	}
	/* If we got this far, there should be no errors, but just making sure :P */
	if (DiceErrCode == DICE_ERR_NONE && !error)
	{
		/* Temporarily store which extended buffer, if any, to use */
		const char *exbuffer = DiceEx == 1 ? extmp : (DiceEx == 2 ? shortextmp : "");
		/* If the result is expected to go to a channel, send a notice to the channel unless DiceServ was triggered through
		 * BotServ's fantasy, then send a privmsg instead */
		if (inchan)
		{
			if (type == DICE_TYPE_EARTHDAWN)
			{
				if (SourceIsBot)
					anope_cmd_privmsg(RollSource, chancpy, "<Earthdawn roll for %s [Step %u (%s)]: %s%s>%s%s", u->nick, step, origdicecpy, exbuffer, buf ? buf : "", *commentstr ? " " : "", commentstr);
				else
					anope_cmd_notice(s_DiceServ, chancpy, "<Earthdawn roll for %s [Step %u (%s)]: %s%s>%s%s", u->nick, step, origdicecpy, exbuffer, buf ? buf : "", *commentstr ? " " : "", commentstr);
			}
			else
			{
				if (SourceIsBot)
					anope_cmd_privmsg(RollSource, chancpy, "<%s for %s [%s]: %s%s>%s%s",
						type == DICE_TYPE_ROLL ? "Roll" : (type == DICE_TYPE_CALC ? "Calc" : (type == DICE_TYPE_EXROLL ? "Exroll" : (type == DICE_TYPE_EXCALC ? "Excalc" : "DnD3e Character Roll"))), u->nick, origdicecpy,
						exbuffer, buf ? buf : "", *commentstr ? " " : "", commentstr);
				else
					anope_cmd_notice(s_DiceServ, chancpy, "<%s for %s [%s]: %s%s>%s%s",
						type == DICE_TYPE_ROLL ? "Roll" : (type == DICE_TYPE_CALC ? "Calc" : (type == DICE_TYPE_EXROLL ? "Exroll" : (type == DICE_TYPE_EXCALC ? "Excalc" : "DnD3e Character Roll"))), u->nick, origdicecpy,
						exbuffer, buf ? buf : "", *commentstr ? " " : "", commentstr);
			}
		}
		/* Otherwise, the result is meant to go only to the user requesting the roll */
		else
		{
			if (type == DICE_TYPE_DND3E)
				moduleNoticeLang(s_DiceServ, u, DICE_DND3ECHAR_USER, origdicecpy, exbuffer, buf ? buf : "", *commentstr ? " " : "", commentstr);
			else if (type == DICE_TYPE_EARTHDAWN)
				moduleNoticeLang(s_DiceServ, u, DICE_EARTHDAWN_USER, step, origdicecpy, exbuffer, buf ? buf : "", *commentstr ? " " : "", commentstr);
			else
				moduleNoticeLang(s_DiceServ, u, DiceEx == 1 || DiceEx == 2 ? (type == DICE_TYPE_EXCALC ? DICE_EXCALC_USER : DICE_EXROLL_USER) : (type == DICE_TYPE_CALC ? DICE_CALC_USER : DICE_ROLL_USER), origdice, exbuffer,
					buf ? buf : "", *commentstr ? " " : "", commentstr);
		}
	}
	/* Free all memory to avoid memory leaks */
	if (DiceExOutput)
		free(DiceExOutput);
	if (DiceShortExOutput)
		free(DiceShortExOutput);
	DiceExOutput = DiceShortExOutput = NULL;
	if (buf)
		free(buf);
	if (extmp)
		free(extmp);
	if (shortextmp)
		free(shortextmp);
	if (origdicecpy)
		free(origdicecpy);
}

/** DiceServ message handler, same as the other pseudo-clients
 * @param u User that invoked DiceServ
 * @param buf The buffer to use for all other commands
 */
static void diceserv(User *u, char *buf)
{
	char *cmd = strtok(buf, " ");

	if (!cmd)
		return;
	SourceIsBot = 0;
	if (!stricmp(cmd, "\1PING"))
	{
		char *s = strtok(NULL, " ");
		if (!s)
			s = "\1";
		notice(s_DiceServ, u->nick, "\1PING %s", s);
	}
	else if (skeleton)
		notice_lang(s_DiceServ, u, SERVICE_OFFLINE, s_DiceServ);
	else
		mod_run_cmd(s_DiceServ, u, DiceServ_cmdTable, cmd);
}

/** Handle PRIVMSG sent to DiceServ
 * @param source The source of the message
 * @param ac Number of arguments, must be 2 (target and message)
 * @param av Arguments, should be target and message
 */
static int my_privmsg(char *source, int ac, char **av)
{
	User *u = finduser(source);
	Uid *uid = find_uid(s_DiceServ);
	char *s;

	/* First, some basic checks */
	if (ac != 2)
		return MOD_CONT; /* Not enough parameters */
	if (!u)
	{
		u = find_byuid(source); /* Didn't find user by nick, try by UID */
		if (!u)
			return MOD_CONT; /* Could not find the user */
	}
	s = strchr(av[0], '@');
	if (s)
	{
		*s++ = 0;
		if (stricmp(s, ServerName))
			return MOD_CONT; /* Not meant for our server */
	}
	if (!stricmp(av[0], s_DiceServ) || (uid && !stricmp(av[0], uid->uid)))
	{
		diceserv(u, av[1]);
		return MOD_STOP; /* Was for us, don't allow other modules to override */
	}
	return MOD_CONT; /* Wasn't for us, return it to the core */
}

/** HELP command
 * @param u User who requested the help
 */
static int do_help(User *u)
{
	char *cmd = strtok(NULL, "");
	if (!cmd)
	{
		int i;
		moduleNoticeLang(s_DiceServ, u, DICE_HELP, s_DiceServ, s_DiceServ, s_DiceServ);
		for (i = DICE_HELP_CMD_ROLL; i < DICE_HELP; ++i)
			if (i <= DICE_HELP_CMD_SET || is_services_oper(u))
				moduleNoticeLang(s_DiceServ, u, i, s_DiceServ);
		moduleNoticeLang(s_DiceServ, u, DICE_HELP_FOOTER, s_DiceServ, s_BotServ ? s_BotServ : "BotServ", s_BotServ ? s_BotServ : "BotServ", s_DiceServ, s_DiceServ);
	}
	else
		mod_help_cmd(s_DiceServ, u, DiceServ_cmdTable, cmd);
	return MOD_CONT;
}

/** HELP ROLL command
 * @param u User who requested the help
 */
static int do_help_roll(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_ROLL, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ);
	return MOD_CONT;
}

/** HELP ROLL EXPRESSIONS command
 * @param u User who requested the help
 */
static int do_help_roll_expressions(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_ROLL_EXPRESSIONS, s_DiceServ);
	return MOD_CONT;
}

/** HELP FUNCTIONS command
 * @param u User who requested the help
 */
static int do_help_functions(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_FUNCTIONS, s_DiceServ);
	return MOD_CONT;
}

/** HELP CALC command
 * @param u User who requested the help
 */
static int do_help_calc(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_CALC, s_DiceServ);
	return MOD_CONT;
}

/** HELP EXROLL command
 * @param u User who requested the help
 */
static int do_help_exroll(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_EXROLL, s_DiceServ);
	return MOD_CONT;
}

/** HELP EXCALC command
 * @param u User who requested the help
 */
static int do_help_excalc(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_EXCALC, s_DiceServ);
	return MOD_CONT;
}

/** HELP DND3ECHAR command
 * @param u User who requested the help
 */
static int do_help_dnd3echar(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_DND3ECHAR, s_DiceServ, s_DiceServ);
	return MOD_CONT;
}

/** HELP EARTHDAWN command
 * @param u User who requested the help
 */
static int do_help_earthdawn(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_EARTHDAWN, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ);
	return MOD_CONT;
}

/** HELP SET command
 * @param u User who requested the help
 */
static int do_help_set(User *u)
{
	moduleNoticeLang(s_DiceServ, u, DICE_HELP_SET, s_DiceServ, s_DiceServ);
	return MOD_CONT;
}

/** HELP SET IGNORE command
 * @param u User who requested the help
 */
static int do_help_set_ignore(User *u)
{
	if (is_services_oper(u))
		moduleNoticeLang(s_DiceServ, u, DICE_SERVADMIN_HELP_SET_IGNORE, s_DiceServ, s_DiceServ, s_ChanServ, s_NickServ);
	else
		moduleNoticeLang(s_DiceServ, u, DICE_HELP_SET_IGNORE, s_DiceServ, s_DiceServ, s_ChanServ);
	return MOD_CONT;
}

/** HELP STATUS command
 * @param u User who requested the help
 */
static int do_help_status(User *u)
{
	if (is_services_oper(u))
		moduleNoticeLang(s_DiceServ, u, DICE_SERVADMIN_HELP_STATUS);
	else
		notice_lang(s_DiceServ, u, NO_HELP_AVAILABLE, "STATUS");
	return MOD_CONT;
}

/** HELP LIST command
 * @param u User who requested the help
 */
static int do_help_list(User *u)
{
	if (is_services_oper(u))
		moduleNoticeLang(s_DiceServ, u, DICE_SERVADMIN_HELP_LIST);
	else
		notice_lang(s_DiceServ, u, NO_HELP_AVAILABLE, "LIST");
	return MOD_CONT;
}

/** ROLL command
 * @param u User who requested the roll
 *
 * Handles regular dice rolls.
 */
static int do_roll(User *u)
{
	char *origdice = strtok(NULL, " "), *chan = strtok(NULL, " "), *comment = strtok(NULL, "");
	diceserv_roller(u, origdice, chan, comment, DICE_TYPE_ROLL, 1);
	return MOD_CONT;
}

/** CALC command
 * @param u User who requested the roll
 *
 * Handles regular dice rolls, sans rounding, resulting in more of a calcualtion.
 */
static int do_calc(User *u)
{
	char *origdice = strtok(NULL, " "), *chan = strtok(NULL, " "), *comment = strtok(NULL, "");
	diceserv_roller(u, origdice, chan, comment, DICE_TYPE_CALC, 0);
	return MOD_CONT;
}

/** EXROLL command
 * @param u User who requested the roll
 *
 * Handles dice rolls with extended output.
 */
static int do_exroll(User *u)
{
	char *origdice = strtok(NULL, " "), *chan = strtok(NULL, " "), *comment = strtok(NULL, "");
	diceserv_roller(u, origdice, chan, comment, !origdice || !strcmp(origdice, "%") || strchr(origdice, 'd') || strchr(origdice, 'D') || strcasestr(origdice, "rand(") ? DICE_TYPE_EXROLL : DICE_TYPE_ROLL, 1);
	return MOD_CONT;
}

/** EXCALC command
 * @param u User who requested the roll
 *
 * Handles dice rolls with extended output, sans rounding, resulting in more of a calcuation.
 */
static int do_excalc(User *u)
{
	char *origdice = strtok(NULL, " "), *chan = strtok(NULL, " "), *comment = strtok(NULL, "");
	diceserv_roller(u, origdice, chan, comment, !origdice || !strcmp(origdice, "%") || strchr(origdice, 'd') || strchr(origdice, 'D') || strcasestr(origdice, "rand(") ? DICE_TYPE_EXCALC : DICE_TYPE_CALC, 0);
	return MOD_CONT;
}

/** DND3ECHAR command
 * @param u User who requested the roll
 *
 * Handles the dice rolls that make up character creation in DnD3e.
 */
static int do_dnd3echar(User *u)
{
	char *chan = strtok(NULL, " "), *comment = strtok(NULL, "");
	diceserv_roller(u, NULL, chan, comment, DICE_TYPE_DND3E, 1);
	return MOD_CONT;
}

/** EARTHDAWN command
 * @param u User who requested the roll
 *
 * Handles dice rolls for the pen & paper RPG Earthdawn.
 */
static int do_earthdawn(User *u)
{
	char *origdice = strtok(NULL, " "), *chan = strtok(NULL, " "), *comment = strtok(NULL, "");
	diceserv_roller(u, origdice, chan, comment, DICE_TYPE_EARTHDAWN, 1);
	return MOD_CONT;
}

/** SET command
 * @param u User who requested the command
 *
 * Currently only has a IGNORE sub-command, which can set a channel or nickname/user to be ignored by DiceServ.
 */
static int do_set(User *u)
{
	char *cmd = strtok(NULL, " ");
	/* Don't allow this command to run if the databases are in read-only mode */
	if (readonly)
	{
		moduleNoticeLang(s_DiceServ, u, DICE_SET_DISABLED);
		return MOD_CONT;
	}
	/* No sub-command given, don't continue processing */
	if (!cmd)
	{
		moduleNoticeLang(s_DiceServ, u, DICE_SET_SYNTAX);
		return MOD_CONT;
	}
	/* SET IGNORE sub-command */
	if (!stricmp(cmd, "IGNORE"))
	{
		char *where = strtok(NULL, " "), *param = strtok(NULL, " ");
		enum
		{
			IGNORE_ADD,
			IGNORE_DEL
		} mode;
		/* We require a parameter, otherwise don't continue processing */
		if (!param)
		{
			moduleNoticeLang(s_DiceServ, u, is_services_oper(u) ? DICE_SERVADMIN_SET_IGNORE_SYNTAX : DICE_SET_IGNORE_SYNTAX);
			return MOD_CONT;
		}
		/* The parameter must be ON or OFF, all others should stop processing */
		if (!stricmp(param, "ON"))
			mode = IGNORE_ADD;
		else if (!stricmp(param, "OFF"))
			mode = IGNORE_DEL;
		else
		{
			moduleNoticeLang(s_DiceServ, u, is_services_oper(u) ? DICE_SERVADMIN_SET_IGNORE_SYNTAX : DICE_SET_IGNORE_SYNTAX);
			return MOD_CONT;
		}
		/* If the thing to ignore starts with a #, assume it's a channel */
		if (*where == '#')
		{
			/* Find the Channel record and ChannelInfo record from ChanServ */
			Channel *c = findchan(where);
			ChannelInfo *ci = cs_findchan(where);
			/* If the channel wasn't found and a ChanServ entry wasn't found (or the channel is forbidden or suspended), display an error */
			if (!c && (!ci || (ci && ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)))))
				notice_lang(s_DiceServ, u, CHAN_X_INVALID, where);
			/* If we found a registered channel, we will store the ignore there */
			else if (ci)
			{
				/* The only ones who can set an ignore on a registered channel are Services Admins or the channel's founder (unless DiceServChanOpCanIgnore is set, then channel operators can as well) */
				int isOp = check_access(u, ci, CA_AUTOOP);
				if (is_services_admin(u) || (DiceServChanOpCanIgnore ? isOp : 0) || ((ci->flags & CI_SECUREFOUNDER) ? is_real_founder(u, ci) : is_founder(u, ci)))
				{
					/* Either add or delete the ignore data */
					if (mode == IGNORE_ADD)
					{
						moduleAddData(&ci->moduleData, "diceserv_ignore", "1");
						if (c)
							add_chan_ignore(c->name);
					}
					else
					{
						moduleDelData(&ci->moduleData, "diceserv_ignore");
						if (c)
							del_chan_ignore(c->name);
					}
					moduleNoticeLang(s_DiceServ, u, mode == IGNORE_ADD ? DICE_SET_IGNORE_CHAN_ON : DICE_SET_IGNORE_CHAN_OFF, s_DiceServ, where);
				}
				/* Otherwise, deny access */
				else
					notice_lang(s_DiceServ, u, ACCESS_DENIED);
			}
			/* No registered channel was found, but a channel was, so store temporary data on the channel */
			else
			{
				/* The only ones who can set an ignore on an unregistered channel are Services Admins or channel operators */
				if (is_services_admin(u) || chan_has_user_status(c, u, CUS_OP))
				{
					/* Either add or delete the ignore data */
					if (mode == IGNORE_ADD)
						add_chan_ignore(c->name);
					else
						del_chan_ignore(c->name);
					moduleNoticeLang(s_DiceServ, u, mode == IGNORE_ADD ? DICE_SET_IGNORE_CHAN_ON : DICE_SET_IGNORE_CHAN_OFF, s_DiceServ, where);
				}
				/* Otherwise, deny access */
				else
					notice_lang(s_DiceServ, u, ACCESS_DENIED);
			}
		}
		/* Otherwise, the thing to ignore is going to be assumed to be a nick */
		else
		{
			/* Find the User record and the NickAlias record from NickServ */
			User *nu = finduser(where);
			NickAlias *na = findnick(where);
			/* Only Services Operators can set ignores on nicks, deny access to those who aren't */
			if (!is_services_oper(u))
				notice_lang(s_DiceServ, u, ACCESS_DENIED);
			/* If the nick wasn't found and a NickServ entry wasn't found (or the nick is forbidden or suspended), display an error */
			else if (!nu && (!na || (na && ((na->status & NS_VERBOTEN) || (na->nc && (na->nc->flags & NI_SUSPENDED))))))
				moduleNoticeLang(s_DiceServ, u, DICE_INVALID_NICK, where);
			/* If we found a registered nick, we will store the ignore there */
			else if (na)
			{
				/* If a User record was not found, we will check all the users to find one that has a matching NickCore */
				if (!nu)
					for (nu = firstuser(); nu; nu = nextuser())
						if (nu->na && nu->na->nc == na->nc)
							break;
				/* Either add or delete the ignore data */
				if (mode == IGNORE_ADD)
				{
					moduleAddData(&na->nc->moduleData, "diceserv_ignore", "1");
					if (nu)
						moduleAddData(&nu->moduleData, "diceserv_ignore", "1");
				}
				else
				{
					moduleDelData(&na->nc->moduleData, "diceserv_ignore");
					if (nu)
						moduleDelData(&nu->moduleData, "diceserv_ignore");
				}
				moduleNoticeLang(s_DiceServ, u, mode == IGNORE_ADD ? DICE_SET_IGNORE_NICK_ON : DICE_SET_IGNORE_NICK_OFF, s_DiceServ, where);
			}
			/* No registered nick was found, but a user was, so store temporary data on the user */
			else
			{
				/* Either add or delete the ignore data */
				if (mode == IGNORE_ADD)
					moduleAddData(&nu->moduleData, "diceserv_ignore", "1");
				else
					moduleDelData(&nu->moduleData, "diceserv_ignore");
				moduleNoticeLang(s_DiceServ, u, mode == IGNORE_ADD ? DICE_SET_IGNORE_NICK_ON : DICE_SET_IGNORE_NICK_OFF, s_DiceServ, where);
			}
		}
	}
	/* Unknown SET command given */
	else
	{
		moduleNoticeLang(s_DiceServ, u, DICE_SET_UNKNOWN_OPTION, cmd);
		notice_lang(s_DiceServ, u, MORE_INFO, s_DiceServ, "SET");
	}
	return MOD_CONT;
}

/** STATUS command
 * @param u User who requested the command
 *
 * This will allow Services Operators to view the ignore status of a single channel or a single nickname/user.
 */
static int do_status(User *u)
{
	char *what = strtok(NULL, " "), *ignore;
	/* We require an argument, otherwise don't continue processing */
	if (!what)
	{
		moduleNoticeLang(s_DiceServ, u, DICE_STATUS_SYNTAX);
		return MOD_CONT;
	}
	/* If the argument starts with a #, assume it's a channel */
	if (*what == '#')
	{
		/* Find the Channel record and ChannelInfo record from ChanServ */
		Channel *c = findchan(what);
		ChannelInfo *ci = cs_findchan(what);
		/* If the channel wasn't found and a ChanServ entry wasn't found (or the channel is forbidden or suspended), display an error */
		if (!c && (!ci || (ci && ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)))))
			notice_lang(s_DiceServ, u, CHAN_X_INVALID, what);
		/* If we found a registered channel, show the data for that */
		else if (ci)
		{
			ignore = moduleGetData(&ci->moduleData, "diceserv_ignore");
			moduleNoticeLang(s_DiceServ, u, DICE_STATUS_CHAN_REGGED, what, moduleGetLangString(u, ignore ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW));
			if (ignore)
				free(ignore);
		}
		/* No registered channel was found, but a channel was, so show the data for that */
		else
		{
			int chan_ignore = find_chan_ignore(c->name);
			moduleNoticeLang(s_DiceServ, u, DICE_STATUS_CHAN, what, moduleGetLangString(u, chan_ignore != -1 ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW));
		}
	}
	/* Otherwise, the argument is going to be assumed to be a nick */
	else
	{
		/* Find the User record and the NickAlias record from NickServ */
		User *nu = finduser(what);
		NickAlias *na = findnick(what);
		/* If the nick wasn't found and a NickServ entry wasn't found (or the nick is forbidden or suspended), display an error */
		if (!nu && (!na || (na && ((na->status & NS_VERBOTEN) || (na->nc && (na->nc->flags & NI_SUSPENDED))))))
			moduleNoticeLang(s_DiceServ, u, DICE_INVALID_NICK, what);
		/* If we found a registered nick, show the data for that */
		else if (na)
		{
			ignore = moduleGetData(&na->nc->moduleData, "diceserv_ignore");
			/* If a User record was not found, we will check all the users to find one that has a matching NickCore */
			if (!nu)
				for (nu = firstuser(); nu; nu = nextuser())
					if (nu->na && nu->na->nc == na->nc)
						break;
			/* If we have a User record, then the given nick is online from another nick in their group, show that */
			if (nu && nu->nick != na->nick)
				moduleNoticeLang(s_DiceServ, u, DICE_STATUS_NICK_ONLINE, what, moduleGetLangString(u, ignore ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW), nu->nick);
			/* Otherwise, they are not online, just show that it is registered and it's data */
			else
				moduleNoticeLang(s_DiceServ, u, DICE_STATUS_NICK_REGGED, what, moduleGetLangString(u, ignore ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW));
			if (ignore)
				free(ignore);
		}
		/* No registered nick was found, but a user was, so show the data for that */
		else
		{
			ignore = moduleGetData(&nu->moduleData, "diceserv_ignore");
			moduleNoticeLang(s_DiceServ, u, DICE_STATUS_NICK, what, moduleGetLangString(u, ignore ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW));
			if (ignore)
				free(ignore);
		}
	}
	return MOD_CONT;
}

/** LIST command
 * @param u User who requested the command
 *
 * This will allow Services Operators to list all the nicknames/users or channels (either registered or not) matching a mask that are either ignored, allowed, or both.
 */
static int do_list(User *u)
{
	char *showtype = strtok(NULL, " "), *what = strtok(NULL, " "), *pattern = strtok(NULL, " "), *regtype = strtok(NULL, " "), *ignore;
	int display, shown = 0, i = 0;
	enum
	{
		SHOW_IGNORED,
		SHOW_ALLOWED,
		SHOW_ALL
	} show;
	enum
	{
		REG_SHOW_ALL,
		REG_SHOW_REG,
		REG_SHOW_UNREG
	} reg = REG_SHOW_ALL;
	/* If no pattern was given, stop processing */
	if (!pattern)
	{
		moduleNoticeLang(s_DiceServ, u, DICE_LIST_SYNTAX);
		return MOD_CONT;
	}
	/* Ignore type must be one of the following, otherwise we stop processing */
	if (!stricmp(showtype, "IGNORE"))
		show = SHOW_IGNORED;
	else if (!stricmp(showtype, "ALLOW"))
		show = SHOW_ALLOWED;
	else if (!stricmp(showtype, "ALL"))
		show = SHOW_ALL;
	else
	{
		moduleNoticeLang(s_DiceServ, u, DICE_LIST_SYNTAX);
		return MOD_CONT;
	}
	/* The type of entries to find must be one of the following, otherwise we stop processing */
	if (stricmp(what, "CHANNELS") && stricmp(what, "NICKS"))
	{
		moduleNoticeLang(s_DiceServ, u, DICE_LIST_SYNTAX);
		return MOD_CONT;
	}
	/* If the optional regtype argument is given, it must be one of the following, otherwise we stop processing */
	if (regtype)
	{
		if (!stricmp(regtype, "REG"))
			reg = REG_SHOW_REG;
		else if (!stricmp(regtype, "UNREG"))
			reg = REG_SHOW_UNREG;
		else
		{
			moduleNoticeLang(s_DiceServ, u, DICE_LIST_SYNTAX);
			return MOD_CONT;
		}
	}
	/* Show the header */
	moduleNoticeLang(s_DiceServ, u, DICE_LIST_HEADER, moduleGetLangString(u, show == SHOW_IGNORED ? DICE_LIST_HEADER_IGNORED : (show == SHOW_ALLOWED ? DICE_LIST_HEADER_ALLOWED : DICE_LIST_HEADER_ALL)),
		moduleGetLangString(u, !stricmp(what, "CHANNELS") ? DICE_LIST_HEADER_CHANNELS : DICE_LIST_HEADER_NICKS), pattern,
		reg == REG_SHOW_ALL ? "" : moduleGetLangString(u, reg == REG_SHOW_REG ? DICE_LIST_HEADER_REGONLY : DICE_LIST_HEADER_UNREGONLY));
	/* If we are to show channels, we do so */
	if (!stricmp(what, "CHANNELS"))
	{
		/* If no regtype argument is given or we want to look only at unregistered channels, we process the channels list */
		if (reg == REG_SHOW_UNREG || reg == REG_SHOW_ALL)
		{
			Channel *c = firstchan();
			for (; c; c = nextchan())
			{
				int chan_ignore;
				/* Skip the channel if it's registered */
				if (c->ci)
					continue;
				chan_ignore = find_chan_ignore(c->name);
				display = 0;
				/* We will only show the channel if it was part of the ignore type request */
				if (chan_ignore && (show == SHOW_IGNORED || show == SHOW_ALL))
					display = 1;
				else if (!chan_ignore && (show == SHOW_ALLOWED || show == SHOW_ALL))
					display = 1;
				/* If we are to display the channel, make sure it matches the pattern first, and we haven't shown too many entries already */
				if (display && (!stricmp(pattern, c->name) || match_wild_nocase(pattern, c->name)) && ++shown <= 100)
				{
					if (reg == REG_SHOW_ALL)
						notice_user(s_DiceServ, u, "   %-20s  %-5s  %s", c->name, moduleGetLangString(u, DICE_LIST_UNREG), moduleGetLangString(u, chan_ignore == -1 ? DICE_IGNORED : DICE_ALLOWED));
					else
						notice_user(s_DiceServ, u, "   %-20s  %s", c->name, moduleGetLangString(u, chan_ignore == -1 ? DICE_IGNORED : DICE_ALLOWED));
				}
			}
		}
		/* If no regtype argument is given or we want to look only at registered channels, we process the ChanServ list */
		if (reg == REG_SHOW_REG || reg == REG_SHOW_ALL)
			for (; i < 256; ++i)
			{
				ChannelInfo *ci = chanlists[i];
				for (; ci; ci = ci->next)
				{
					/* Skip the channel if it's forbidden or suspended */
					if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED))
						continue;
					ignore = moduleGetData(&ci->moduleData, "diceserv_ignore");
					display = 0;
					/* We will only show the channel if it was part of the ignore type request */
					if (ignore && (show == SHOW_IGNORED || show == SHOW_ALL))
						display = 1;
					else if (!ignore && (show == SHOW_ALLOWED || show == SHOW_ALL))
						display = 1;
					/* If we are to display the channel, make sure it matches the pattern first, and we haven't shown too many entries already */
					if (display && (!stricmp(pattern, ci->name) || match_wild_nocase(pattern, ci->name)) && ++shown <= 100)
					{
						if (reg == REG_SHOW_ALL)
							notice_user(s_DiceServ, u, "   %-20s  %-5s  %s", ci->name, moduleGetLangString(u, DICE_LIST_REG), moduleGetLangString(u, ignore ? DICE_IGNORED : DICE_ALLOWED));
						else
							notice_user(s_DiceServ, u, "   %-20s  %s", ci->name, moduleGetLangString(u, ignore ? DICE_IGNORED : DICE_ALLOWED));
					}
					if (ignore)
						free(ignore);
				}
			}
	}
	/* Otherwise, we are to show nicks */
	else
	{
		/* If no regtype argument is given or we want to look only at unregistered nicks, we process the users list */
		if (reg == REG_SHOW_UNREG || reg == REG_SHOW_ALL)
		{
			User *nu = firstuser();
			for (; nu; nu = nextuser())
			{
				/* Skip the nick if it's registered */
				if (nu->na)
					continue;
				ignore = moduleGetData(&nu->moduleData, "diceserv_ignore");
				display = 0;
				/* We will only show the channel if it was part of the ignore type request */
				if (ignore && (show == SHOW_IGNORED || show == SHOW_ALL))
					display = 1;
				else if (!ignore && (show == SHOW_ALLOWED || show == SHOW_ALL))
					display = 1;
				/* If we are to display the nick, make sure it matches the pattern first, and we haven't shown too many entries already */
				if (display && (!stricmp(pattern, nu->nick) || match_wild_nocase(pattern, nu->nick)) && ++shown <= 100)
				{
					if (reg == REG_SHOW_ALL)
						notice_user(s_DiceServ, u, "   %-20s  %-5s  %s", nu->nick, moduleGetLangString(u, DICE_LIST_UNREG), moduleGetLangString(u, ignore ? DICE_IGNORED : DICE_ALLOWED));
					else
						notice_user(s_DiceServ, u, "   %-20s  %s", nu->nick, moduleGetLangString(u, ignore ? DICE_IGNORED : DICE_ALLOWED));
				}
				if (ignore)
					free(ignore);
			}
		}
		/* If no regtype argument is given or we want to look only at registered nicks, we process the NickServ list */
		if (reg == REG_SHOW_REG || reg == REG_SHOW_ALL)
			for (; i < 1024; i++)
			{
				NickAlias *na = nalists[i];
				for (; na; na = na->next)
				{
					/* Skip the nick if it's forbidden or suspended */
					if ((na->status & NS_VERBOTEN) || (na->nc && (na->nc->flags & NI_SUSPENDED)))
						continue;
					ignore = moduleGetData(&na->nc->moduleData, "diceserv_ignore");
					display = 0;
					/* We will only show the channel if it was part of the ignore type request */
					if (ignore && (show == SHOW_IGNORED || show == SHOW_ALL))
						display = 1;
					else if (!ignore && (show == SHOW_ALLOWED || show == SHOW_ALL))
						display = 1;
					/* If we are to display the nick, make sure it matches the pattern first, and we haven't shown too many entries already */
					if (display && (!stricmp(pattern, na->nick) || match_wild_nocase(pattern, na->nick)) && ++shown <= 100)
					{
						if (reg == REG_SHOW_ALL)
							notice_user(s_DiceServ, u, "   %-20s  %-5s  %s", na->nick, moduleGetLangString(u, DICE_LIST_REG), moduleGetLangString(u, ignore ? DICE_IGNORED : DICE_ALLOWED));
						else
							notice_user(s_DiceServ, u, "   %-20s  %s", na->nick, moduleGetLangString(u, ignore ? DICE_IGNORED : DICE_ALLOWED));
					}
					if (ignore)
						free(ignore);
				}
			}
	}
	/* Show the footer */
	moduleNoticeLang(s_DiceServ, u, DICE_LIST_END, shown > 100 ? 100 : shown, shown);
	return MOD_CONT;
}

/** INFO command (for NickServ)
 * @param u User who requested the command
 *
 * This will show a nick's DiceServ ignore status in NS INFO, only to Services Operators.
 */
static int do_ns_info(User *u)
{
	char *text = moduleGetLastBuffer(), *nick = myStrGetToken(text, ' ', 0);
	if (nick && is_services_oper(u))
	{
		NickAlias *na = findnick(nick);
		if (na && (!(na->status & NS_VERBOTEN) || (na->nc && (na->nc->flags & NI_SUSPENDED))))
		{
			char *ignore = moduleGetData(&na->nc->moduleData, "diceserv_ignore");
			moduleNoticeLang(s_NickServ, u, DICE_INFO_IGNORE, "  ", s_DiceServ, moduleGetLangString(u, ignore ? DICE_IGNORED : DICE_ALLOWED));
			if (ignore)
				free(ignore);
		}
		free(nick);
	}
	return MOD_CONT;
}

/** INFO command (for ChanServ)
 * @param u User who requested the command
 *
 * This will show a nick's DiceServ ignore status in CS INFO, only to Services Operators or the channel's founder.
 */
static int do_cs_info(User *u)
{
	char *text = moduleGetLastBuffer(), *chan = myStrGetToken(text, ' ', 0);
	if (chan)
	{
		ChannelInfo *ci = cs_findchan(chan);
		if (ci && !(ci->flags & CI_VERBOTEN) && ((ci->flags & CI_SECUREFOUNDER ? is_real_founder(u, ci) : is_founder(u, ci)) || is_services_oper(u)))
		{
			char *ignore = moduleGetData(&ci->moduleData, "diceserv_ignore");
			moduleNoticeLang(s_ChanServ, u, DICE_INFO_IGNORE, "", s_DiceServ, moduleGetLangString(u, ignore ? DICE_IGNORED : DICE_ALLOWED));
			if (ignore)
				free(ignore);
		}
		free(chan);
	}
	return MOD_CONT;
}

/** Load DiceServ's configuration.
 *
 * First directive is DiceServDBName, defaults to diceserv.db if not given.
 * Second directive is DiceServName, defaults to DiceServ if not given.
 * Third directive is DiceServChanOpCanIgnore, defaults to 0 if not given.
 */
static void do_load_config(void)
{
	int i;
	char *tmpDBName = NULL, *tmpName = NULL;
	Directive confvalues[][1] = {
		{{"DiceServDBName", {{PARAM_STRING, PARAM_RELOAD, &tmpDBName}}}},
		{{"DiceServName", {{PARAM_STRING, PARAM_RELOAD, &tmpName}}}},
		{{"DiceServChanOpCanIgnore", {{PARAM_INT, PARAM_RELOAD, &DiceServChanOpCanIgnore}}}}
	};

	DiceServChanOpCanIgnore = 0;

	for (i = 0; i < 3; ++i)
		moduleGetConfigDirective(confvalues[i]);

	if (DiceServDBName)
		free(DiceServDBName);
	if (s_DiceServ)
		free(s_DiceServ);

	DiceServDBName = sstrdup(tmpDBName ? tmpDBName : "diceserv.db");
	s_DiceServ = sstrdup(tmpName ? tmpName : "DiceServ");
}

/** Load DiceServ's database.
 *
 * DiceServ's database consists of all the nicks and channels that are registered that were ignored from using DiceServ.
 */
static void do_load_db(void)
{
	char readbuf[1024];
	FILE *in = fopen(DiceServDBName, "rb");

	/* Fail if we were unable to open the database */
	if (!in)
	{
		alog("[diceserv] Unable to open database ('%s') for reading", DiceServDBName);
		return;
	}

	/* Read in the database line by line, format is: {N|C} {nick|channel} */
	while (fgets(readbuf, 1024, in))
	{
		char *type;
		unsigned len = strlen(readbuf);
		readbuf[len - 1] = 0;
		type = myStrGetToken(readbuf, ' ', 0);
		if (type)
		{
			char *name = myStrGetToken(readbuf, ' ', 1);
			if (name)
			{
				/* If type is C, it's a channel, find it and add a ignore to it */
				if (!stricmp(type, "C"))
				{
					ChannelInfo *ci = cs_findchan(name);
					if (ci)
						moduleAddData(&ci->moduleData, "diceserv_ignore", "1");
				}
				/* If type is N, it's a nick, find it and add a ignore to it */
				else if (!stricmp(type, "N"))
				{
					NickAlias *na = findnick(name);
					if (na)
						moduleAddData(&na->nc->moduleData, "diceserv_ignore", "1");
				}
				free(name);
			}
			free(type);
		}
	}

	fclose(in);

	if (debug)
		alog("[diceserv] Successfully loaded database");
}

/** Save DiceServ's database.
 */
static void do_save_db(void)
{
	FILE *out = fopen(DiceServDBName, "wb");
	int i;

	/* Fail if we were unable to open the database */
	if (!out)
	{
		alog("[diceserv] Unable to open database ('%s') for writing", DiceServDBName);
		return;
	}

	/* Go through all the registered channels and add them to the database file if they have a ignore on them */
	for (i = 0; i < 256; ++i)
	{
		ChannelInfo *ci = chanlists[i];
		for (; ci; ci = ci->next)
		{
			char *info = moduleGetData(&ci->moduleData, "diceserv_ignore");
			if (info)
			{
				fprintf(out, "C %s\n", ci->name);
				free(info);
			}
		}
	}

	/* Go through all the registered nicks and add them to the database file if they have a ignore on them */
	for (i = 0; i < 1024; ++i)
	{
		NickCore *nc = nclists[i];
		for (; nc; nc = nc->next)
		{
			char *info = moduleGetData(&nc->moduleData, "diceserv_ignore");
			if (info)
			{
				fprintf(out, "N %s\n", nc->display);
				free(info);
			}
		}
	}

	fclose(out);

	if (debug)
		alog("[diceserv] Successfully saved database");
}

/** Save the database when Anope saves it's own databases.
 * @param argc The number of arguments (should be 1, technically)
 * @param argv The arguments, should be either EVENT_START or EVENT_STOP
 *
 * We only care if the argument is EVENT_START as it doesn't matter either way.
 */
static int do_save_db_evt(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_START))
		do_save_db();

	return MOD_CONT;
}

/** Backup the database when Anope backs up it's own databases.
 * @param argc The number of arguments (should be 1, technically)
 * @param argv The arguments, should be either EVENT_START or EVENT_STOP
 *
 * We only care if the argument is EVENT_START as it doesn't matter either way.
 */
static int do_backup_db_evt(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_START))
		ModuleDatabaseBackup(DiceServDBName);

	return MOD_CONT;
}

/** Process a new user joining the network or changing nicks.
 * @param argc The number of arguments (should be 1, technically)
 * @param argc The arguments, the only one of which should be the nick of the user
 *
 * This checks if the user given has a registered nick, and if they do, checks if they are ignored from using DiceServ and keeps that flag on them.
 */
static int do_nick_evt(int argc, char **argv)
{
	User *u = finduser(argv[0]);

	if (u->na)
	{
		char *ignore = moduleGetData(&u->na->nc->moduleData, "diceserv_ignore");
		if (ignore)
		{
			moduleAddData(&u->moduleData, "diceserv_ignore", "1");
			free(ignore);
		}
	}

	return MOD_CONT;
}

/** Process a newly registered nick.
 * @param argc The number of arguments (should be 1, technically)
 * @param argv The arguments, the only one of which should be the nick that was just registered
 *
 * Upon registration, check if their User record has a ignore on it, and if it does, make sure the newly registered nick is also ignored.
 */
static int do_nick_register_evt(int argc, char **argv)
{
	User *u = finduser(argv[0]);
	char *ignore = moduleGetData(&u->moduleData, "diceserv_ignore");

	if (ignore)
	{
		moduleAddData(&u->na->nc->moduleData, "diceserv_ignore", "1");
		free(ignore);
	}

	return MOD_CONT;
}

/** Process a newly registered channel.
 * @param argc The number of arguments (should be 1, technically)
 * @param argv The arguments, the only one of which should be the channel that was just registered
 *
 * Upon registration, check if the Channel record has a ignore on it, and if it does, make sure the newly registered channel is also ignored.
 */
static int do_chan_register_evt(int argc, char **argv)
{
	Channel *c = findchan(argv[0]);
	int index = find_chan_ignore(c->name);

	if (index != -1)
		moduleAddData(&c->ci->moduleData, "diceserv_ignore", "1");

	return MOD_CONT;
}

/** Process a channel join (mainly for channel creation, but we have no way of knowing that through an event)
 * @param argc The number of arguments (should be 3, technically)
 * @param argv The arguments, being EVENT_START/EVENT_STOP (we only want STOP), the nick that joined (don't need), and the name of the channel joined
 *
 * Upon a channel being joined, check if the channel is registered and has an ignore on it, and if it does, make sure the joined channel is also ignored.
 */
static int do_chan_join_evt(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_STOP))
	{
		Channel *c = findchan(argv[2]);
		int index = find_chan_ignore(c->name);

		if (index == -1)
		{
			char *ignore = c->ci ? moduleGetData(&c->ci->moduleData, "diceserv_ignore") : NULL;
			if (ignore)
			{
				add_chan_ignore(c->name);
				free(ignore);
			}
		}
	}

	return MOD_CONT;
}

/** Process a channel part (mainly for channel deletetion, but we have no way of knowing that through an event)
 * @param argc The number of arguments (should be 3, technically)
 * @param argv The arguments, being EVENT_START/EVENT_STOP (we only want START), the nick that is parting (don't need), and the name of the channel being parted from
 *
 * Prior to a channel being parted, delete that channel's ignore data.
 */
static int do_chan_part_evt(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_START))
	{
		Channel *c = findchan(argv[2]);
		if (c->usercount == 1)
			del_chan_ignore(c->name);
	}

	return MOD_CONT;
}

/** Process BotServ fantasy commands.
 * @param argc The number of arguments, could be 3 or 4
 * @param argv The arguments, will be the fantasy command, the user, the channel, and the remainder if any (we want the remainder)
 *
 * This will allow for DiceServ to be invoked in a channel through BotServ using fantasy commands.
 */
static int do_fantasy(int argc, char **argv)
{
	User *u;
	ChannelInfo *ci;
	char *origdice = NULL, *comment = NULL;
	enum DICE_TYPES type;
	int round_result = 1, arg_count = 4;

	/* We require 4 arguments (only 3 if dnd3echar), stop processing otherwise */
	if (!stricmp(argv[0], "dnd3echar"))
		arg_count = 3;
	if (argc < arg_count)
		return MOD_CONT;

	/* Check for one of the following fantasy commands, and stop processing if the command isn't one of these */
	if (!stricmp(argv[0], "roll"))
	{
		origdice = myStrGetToken(argv[3], ' ', 0);
		comment = myStrGetTokenRemainder(argv[3], ' ', 1);
		type = DICE_TYPE_ROLL;
	}
	else if (!stricmp(argv[0], "calc"))
	{
		origdice = myStrGetToken(argv[3], ' ', 0);
		comment = myStrGetTokenRemainder(argv[3], ' ', 1);
		type = DICE_TYPE_CALC;
		round_result = 0;
	}
	else if (!stricmp(argv[0], "exroll"))
	{
		origdice = myStrGetToken(argv[3], ' ', 0);
		comment = myStrGetTokenRemainder(argv[3], ' ', 1);
		type = !origdice || !strcmp(origdice, "%") || strchr(origdice, 'd') || strchr(origdice, 'D') || strcasestr(origdice, "rand(") ? DICE_TYPE_EXROLL : DICE_TYPE_ROLL;
	}
	else if (!stricmp(argv[0], "excalc"))
	{
		origdice = myStrGetToken(argv[3], ' ', 0);
		comment = myStrGetTokenRemainder(argv[3], ' ', 1);
		type = !origdice || !strcmp(origdice, "%") || strchr(origdice, 'd') || strchr(origdice, 'D') || strcasestr(origdice, "rand(") ? DICE_TYPE_EXCALC : DICE_TYPE_CALC;
		round_result = 0;
	}
	else if (!stricmp(argv[0], "dnd3echar"))
	{
		if (argc > 3)
			comment = sstrdup(argv[3]);
		type = DICE_TYPE_DND3E;
	}
	else if (!stricmp(argv[0], "earthdawn"))
	{
		origdice = myStrGetToken(argv[3], ' ', 0);
		comment = myStrGetTokenRemainder(argv[3], ' ', 1);
		type = DICE_TYPE_EARTHDAWN;
	}
	else
		return MOD_CONT;

	/* Get the user to pass on to the DiceServ core */
	u = finduser(argv[1]);
	/* Find the channel and determine the roll source, should be the channel's BotServ bot unless the IRCd is somehow sending the command
	 * without a BotServ bot in the channel, in which case the result will be echoed by DiceServ */
	ci = cs_findchan(argv[2]);
	RollSource = ci && ci->bi ? ci->bi->nick : s_DiceServ;
	SourceIsBot = 1;
	diceserv_roller(u, origdice, argv[2], comment, type, round_result);

	if (origdice)
		free(origdice);
	if (comment)
		free(comment);

	return MOD_CONT;
}

/** Add the languages for the DiceServ module.
 */
static void do_add_languages(void)
{
	char *langtable_en_us[] = {
		/* DICE_ROLL_SYNTAX */
		"Syntax: ROLL dice [[channel] comment]",
		/* DICE_CALC_SYNTAX */
		"Syntax: CALC dice [[channel] comment]",
		/* DICE_EXROLL_SYNTAX */
		"Syntax: EXROLL dice [[channel] comment]",
		/* DICE_EXCALC_SYNTAX */
		"Syntax: EXCALC dice [[channel] comment]",
		/* DICE_DND3ECHAR_SYNTAX */
		"Syntax: DND3ECHAR [[channel] comment]",
		/* DICE_EARTHDAWN_SYNTAX */
		"Syntax: EARTHDAWN step [[channel] comment]",
		/* DICE_ROLL_USER */
		"<Roll [%s]: %s%s>%s%s",
		/* DICE_CALC_USER */
		"<Calc [%s]: %s%s>%s%s",
		/* DICE_EXROLL_USER */
		"<Exroll [%s]: %s%s>%s%s",
		/* DICE_EXCALC_USER */
		"<Excalc [%s]: %s%s>%s%s",
		/* DICE_DND3ECHAR_USER */
		"<DnD3e Character roll [%s]: %s%s>%s%s",
		/* DICE_EARTHDAWN_USER */
		"<Earthdawn roll [Step %u (%s)]: %s%s>%s%s",
		/* DICE_ERROR_NOPARENAFTERFUNCTION */
		"No open parenthesis found after function.",
		/* DICE_ERROR_BEFORECOMMA */
		"No number or close parenthesis before comma.",
		/* DICE_ERROR_AFTERCOMMA */
		"No number or open parenthesis after comma.",
		/* DICE_ERROR_BEFOREOPERATOR */
		"No number or close parenthesis before operator.",
		/* DICE_ERROR_AFTEROPERATOR */
		"No number or open parenthesis after operator.",
		/* DICE_ERROR_BEFOREOPENPAREN */
		"No operator or open parenthesis found before current open\n"
		"parenthesis.",
		/* DICE_ERROR_AFTEROPENPAREN */
		"No number found after current open parenthesis.",
		/* DICE_ERROR_BEFORECLOSEPAREN */
		"No number found before current close parenthesis.",
		/* DICE_ERROR_AFTERCLOSEPAREN */
		"No operator or close parenthesis found after current close\n"
		"parenthesis.",
		/* DICE_ERROR_INVALIDCHAR */
		"An invalid character was encountered.",
		/* DICE_ERROR_NONUMBERBEFOREOPERATOR */
		"No numbers were found before the operator was encountered.",
		/* DICE_ERROR_NOTENOUGHOPENPARENS */
		"A close parenthesis was found but not enough open\n"
		"parentheses were found before it.",
		/* DICE_ERROR_INVALIDCOMMA */
		"A comma was encountered outside of a function.",
		/* DICE_ERROR_TOOMANYOPENPARENS */
		"There are more open parentheses than close parentheses.",
		/* DICE_ERROR_EMPTYTOKEN */
		"An empty token was found.",
		/* DICE_ERROR_NOTENOUGHFUNCTIONARGS */
		"Not enough numbers for function.",
		/* DICE_ERROR_NOTENOUGHOPERATORARGS */
		"Not enough numbers for operator.",
		/* DICE_ERROR_EMPTYNUMBER */
		"An empty number was found.",
		/* DICE_ERROR_TOOMANYNUMBERS */
		"Too many numbers were found as input.",
		/* DICE_PARSE_ERROR */
		"During parsing, an error was found in the following\n"
		"expression:",
		/* DICE_ERROR_DIVBYZERO */
		"Division by 0 in following expression:",
		/* DICE_ERROR_UNDEFINED */
		"Undefined result in following expression:",
		/* DICE_DICEOVERLIMIT */
		"The number of dice that you entered (%d) was over the\n"
		"limit of %d. Please enter a lower number of dice.",
		/* DICE_SIDESOVERLIMIT */
		"The number of sides that you entered (%d) was over the\n"
		"limit of %d. Please enter a lower number of sides.",
		/* DICE_TIMESOVERLIMIT */
		"The number of times that you entered (%d) was over the\n"
		"limit of %d. Please enter a lower number of times.",
		/* DICE_DICEUNDERLIMIT */
		"The number of dice that you entered (%d) was under\n"
		"1. Please enter a number between 1 and %d.",
		/* DICE_SIDESUNDERLIMIT */
		"The number of sides that you entered (%d) was under\n"
		"1. Please enter a number between 1 and %d.",
		/* DICE_TIMESUNDERLIMIT */
		"The number of times that you entered (%d) was under\n"
		"1. Please enter a number between 1 and %d.",
		/* DICE_STACK_ERR */
		"The following roll expression could not be properly\n"
		"evaluated, please try again or let an administrator know.",
		/* DICE_BUFFER_OVERFLOW */
		"Dice result buffer has an overflow. This could be due to\n"
		"large values that are close to the limits or the size of\n"
		"your comment. Please enter some lower rolls or a smaller\n"
		"comment.",
		/* DICE_EX_OVERFLOW */
		"Too many results for %s to display, omitting results.",
		/* DICE_OVERUNDERFLOW */
		"Dice results in following expression resulted in either\n"
		"overflow or underflow:",
		/* DICE_ERRSTR */
		"Error description is as follows:",
		/* DICE_ERROR_EARTHDAWN_INPUT */
		"step for an Earthdawn roll must be a number.",
		/* DICE_ERROR_EARTHDAWN_RANGE */
		"The step you entered (%d) was out of range, it must be\n"
		"between 1 and 100.",
		/* DICE_ERROR_DND3E_MODTOTALZERO */
		"D&D 3e Character roll resulted in a character that had their\n"
		"total modifiers be 0 or below, rerolling stats again.",
		/* DICE_ERROR_DND3E_MAXSCORETHIRTEEN */
		"D&D 3e Character roll resulted in a character that had a max\n"
		"score of 13 or less for all their abilities, rerolling stats\n"
		"again.",
		/* DICE_SET_DISABLED */
		"Sorry, dice ignore option setting is temporarily disabled.",
		/* DICE_SET_SYNTAX */
		"Syntax: SET option parameters",
		/* DICE_SET_UNKNOWN_OPTION */
		"Unknown SET option %s.",
		/* DICE_SET_IGNORE_SYNTAX */
		"Syntax: SET IGNORE channel {ON|OFF}",
		/* DICE_SERVADMIN_SET_IGNORE_SYNTAX */
		"Syntax: SET IGNORE {channel|nick} {ON|OFF}",
		/* DICE_SET_IGNORE_CHAN_ON */
		"%s will now ignore all dice rolls sent to %s.",
		/* DICE_SET_IGNORE_CHAN_OFF */
		"%s will now allow all dice rolls sent to %s.",
		/* DICE_SET_IGNORE_NICK_ON */
		"%s will now ignore all dice rolls by %s.",
		/* DICE_SET_IGNORE_NICK_OFF */
		"%s will now allow all dice rolls by %s.",
		/* DICE_STATUS_SYNTAX */
		"Syntax: STATUS {channel|nick}",
		/* DICE_STATUS_CHAN_REGGED */
		"Status for registered channel %s: %s",
		/* DICE_STATUS_CHAN */
		"Status for unregistered channel %s: %s",
		/* DICE_STATUS_NICK_ONLINE */
		"Status for registered nick %s: %s\n"
		"  (online as %s)",
		/* DICE_STATUS_NICK_REGGED */
		"Status for registered nick %s: %s",
		/* DICE_STATUS_NICK */
		"Status for unregistered nick %s: %s",
		/* DICE_STATUS_IGNORE */
		"Ignore",
		/* DICE_STATUS_ALLOW */
		"Allow",
		/* DICE_LIST_SYNTAX */
		"Syntax: LIST {IGNORE|ALLOW|ALL} what pattern [{REG|UNREG}]",
		/* DICE_LIST_HEADER */
		"List of %s %s entries matching %s%s:",
		/* DICE_LIST_HEADER_IGNORED */
		"ignored",
		/* DICE_LIST_HEADER_ALLOWED */
		"allowed",
		/* DICE_LIST_HEADER_ALL */
		"all",
		/* DICE_LIST_HEADER_CHANNELS */
		"channels",
		/* DICE_LIST_HEADER_NICKS */
		"nicks",
		/* DICE_LIST_HEADER_REGONLY */
		" (registrered only)",
		/* DICE_LIST_HEADER_UNREGONLY */
		" (unregistered only)",
		/* DICE_LIST_UNREG */
		"Unreg",
		/* DICE_LIST_REG */
		"Reg",
		/* DICE_LIST_END */
		"End of list - %d/%d matches shown.",
		/* DICE_INFO_IGNORE */
		"%s%s Status: %s",
		/* DICE_INVALID_NICK */
		"Nick %s is not a valid nick.",
		/* DICE_IGNORED */
		"Ignored",
		/* DICE_ALLOWED */
		"Allowed",
		/* DICE_HELP_CMD_ROLL */
		"    ROLL           Rolls dice (or performs math too)",
		/* DICE_HELP_CMD_CALC */
		"    CALC           ROLL without rounding, for calculations",
		/* DICE_HELP_CMD_EXROLL */
		"    EXROLL         Rolls dice and shows each dice roll",
		/* DICE_HELP_CMD_EXCALC */
		"    EXCALC         EXROLL without rounding, for calculations",
		/* DICE_HELP_CMD_DND3ECHAR */
		"    DND3ECHAR      Rolls dice for DnD3e character creation",
		/* DICE_HELP_CMD_EARTHDAWN */
		"    EARTHDAWN      Rolls dice for Earthdawn",
		/* DICE_HELP_CMD_SET */
		"    SET            Set options for %s access",
		/* DICE_HELP_CMD_STATUS */
		"    STATUS         Shows allow status of channel or nick",
		/* DICE_HELP_CMD_LIST */
		"    LIST           Gives list of %s access",
		/* DICE_HELP */
		"%s allows you to roll any number of dice with any\n"
		"number of sides. The output of the roll can either be output\n"
		"just to you, or you can have it notice the result to a\n"
		"channel. Available commands are listed below; to use them,\n"
		"type /msg %s command. For more information on a\n"
		"specific command, type /msg %s HELP command.\n"
		" ",
		/* DICE_HELP_FOOTER */
		" \n"
		"%s will check for syntax errors and tell you what\n"
		"errors you have.\n"
		" \n"
		"If a %s bot is in a channel, you can also trigger the\n"
		"bot within the channel using !roll, !exroll, !dnd3echar,\n"
		"or !earthdawn. If a %s bot is in the channel, output\n"
		"will be said by the bot. Otherwise it will be said by\n"
		"%s. Syntax of those is:\n"
		" \n"
		"!roll dice [comment]\n"
		"!exroll dice [comment]\n"
		"!dnd3echar [comment]\n"
		"!earthdawn step [comment]\n"
		" \n"
		"%s by Naram Qashat (CyberBotX, cyberbotx@cyberbotx.com).\n"
		"Questions, comments, or concerns can be directed to email or\n"
		"to #DiceServ on jenna.cyberbotx.com.",
		/* DICE_HELP_ROLL */
		"Syntax: ROLL dice [[channel] comment]\n"
		" \n"
		"Echoes a dice roll to you. If you are using this command\n"
		"while using a registered nick, it will come to you using\n"
		"the method you tell Services to use. Otherwise, it will use\n"
		"the default that Services is set to. Roll will be displayed\n"
		"as follows:\n"
		" \n"
		"<Roll [dice]: result>\n"
		" \n"
		"Channel is an optional argument, it must be a valid\n"
		"channel and one that you are currently in. If you give an\n"
		"invalid channel or one you are not in, the command will be\n"
		"halted. If given and is valid, dice roll will be echoed to\n"
		"the channel given as a NOTICE in the following form:\n"
		" \n"
		"<Roll for nick [dice]: result>\n"
		" \n"
		"Comment is also an optional argument. You do not need to\n"
		"give a channel to use a comment. If given, this comment will\n"
		"be added to the end of the result.\n"
		" \n"
		"For help on the dice expression syntax, see /msg %s\n"
		"HELP ROLL EXPRESSIONS.\n"
		" \n"
		"Examples:\n"
		"  Roll 3d6: /msg %s ROLL 3d6\n"
		"  Roll 3d6+5: /msg %s ROLL 3d6+5\n"
		"  Roll 3d6+5, then double end result:\n"
		"    /msg %s ROLL (3d6+5)*2\n"
		"  Roll 3d6, double the result, then add 5:\n"
		"    /msg %s ROLL 3d6*2+5\n"
		"  Roll 3d6 three consecutive times:\n"
		"    /msg %s ROLL 3~3d6",
		/* DICE_HELP_ROLL_EXPRESSIONS */
		"Dice expression syntax\n"
		" \n"
		"dice expression must be in the form of: [x~]y\n"
		" \n"
		"x and y can support very complex forms of expressions. In\n"
		"order to get an actual dice roll, you must use something in\n"
		"the format of: [z]dw, where z is the number of dice to be\n"
		"thrown, and w is the number of sides on each die. z is\n"
		"optional, will default to 1 if not given. Please note that\n"
		"the sides or number of dice can not be 0 or negative, and\n"
		"both can not be greater than 99999.\n"
		" \n"
		"x~ is used to determine how many consecutive sets of dice\n"
		"will be rolled. This is optional, defaults to 1 if not\n"
		"given. If you use this form, you MUST include the ~ for it\n"
		"to be recognized as you wanting to throw a dice set multiple\n"
		"times.\n"
		" \n"
		"y is normally used for the standard dice roll. You could\n"
		"also do plain math within y, if you want. You must include\n"
		"something here, but if it's not a number, it will usually\n"
		"result in a parsing error.\n"
		" \n"
		"To roll what is called a \"percentile\" die, or a 100-sided\n"
		"die, you can use %% as your roll expression, or include d%%\n"
		"within your roll expression. For the former, the expression\n"
		"will be replaced with 1d100, whereas for the latter, the\n"
		"%% in the expression will be replaced with a 100. For all\n"
		"other cases, the %% will signify modulus of the numbers\n"
		"before and after it, the modulus being the remainder that\n"
		"you'd get from dividing the first number by the second\n"
		"number.\n"
		" \n"
		"The following math operators can be used in expressions:\n"
		" \n"
		"+ - * / ^ %% (in addition to 'd' for dice rolls and\n"
		"parentheses to force order of operatons.)\n"
		" \n"
		"Also note that if you use decimals in your expressions, the\n"
		"result will be returned in integer form, unless you use CALC\n"
		"or EXCALC. An additional note, implicit multiplication with\n"
		"parentheses (example: 2(3d6)) will function as it should (as\n"
		"2*(3d6)).\n"
		" \n"
		"In addition to the above math operators, certain functions\n"
		"are also recognized. For a full list, see /msg\n"
		"%s HELP FUNCTIONS. The following math constants are\n"
		"also recognized and will be filled in automatically:\n"
		" \n"
		"    e              Exponential growth constant\n"
		"                   2.7182818284590452354\n"
		"    pi             Archimedes' constant\n"
		"                   3.14159265358979323846\n"
		" \n"
		"The dice roller will also recognize if you want to have a\n"
		"negative number when prefixed with a -. This will not cause\n"
		"problems even though it is also used for subtraction.",
		/* DICE_HELP_FUNCTIONS */
		"%s recognizes the following functions:\n"
		" \n"
		"    abs(x)         Absolute value of x\n"
		"    acos(x)        Arc cosine of x\n"
		"    acosh(x)       Inverse hyperbolic cosine of x\n"
		"    asin(x)        Arc sine of x\n"
		"    asinh(x)       Inverse hyperbolic sine of x\n"
		"    atan(x)        Arc tangent of x\n"
		"    atanh(x)       Inverse hyperbolic tangent of x\n"
		"    cbrt(x)        Cube root of x\n"
		"    ceil(x)        The next smallest integer greater than\n"
		"                   or equal to x\n"
		"    cos(x)         Cosine of x\n"
		"    cosh(x)        Hyperbolic cosine of x\n"
		"    deg(x)         Convert x from radians to degrees\n"
		"    exp(x)         e (exponential value) to the power\n"
		"                   of x\n"
		"    fac(x)         Factorial of x\n"
		"    floor(x)       The next largest integer less than or\n"
		"                   equal to x\n"
		"    log(x)         Natural logarithm of x\n"
		"    log10(x)       Logarithm of x to base 10\n"
		"    max(x,y)       Maximum of x and y\n"
		"    min(x,y)       Minimum of x and y\n"
		"    rad(x)         Convert x from degrees to radians\n"
		"    rand(x,y)      Random value between x and y\n"
		"    round(x)       Round x to the nearest integer\n"
		"    sin(x)         Sine of x\n"
		"    sinh(x)        Hyperbolic sine of x\n"
		"    sqrt(x)        Square root of x\n"
		"    tan(x)         Tangent of x\n"
		"    tanh(x)        Hyperbolic tangent of x\n"
		"    trunc(x)       The integral portion of x\n"
		" \n"
		"NOTE: All trigonometric functions above (sine, cosine and\n"
		"tangent) return their values in radians.",
		/* DICE_HELP_CALC */
		"Syntax: CALC dice [[channel] comment]\n"
		" \n"
		"This command is identical like ROLL (see /msg %s\n"
		"HELP ROLL for more information on how to use this and\n"
		"ROLL), except the results are not rounded off and are\n"
		"displayed as is.",
		/* DICE_HELP_EXROLL */
		"Syntax: EXROLL dice [[channel] comment]\n"
		" \n"
		"This command is exactly like ROLL (see /msg %s HELP\n"
		"ROLL for more information on how to use this and ROLL),\n"
		"with one slight difference. EXROLL will show what was rolled\n"
		"on each die as it is rolled.\n"
		" \n"
		"Example: Roll a 4d6: {4d6=(6 3 1 4)}=14\n"
		" \n"
		"This can be useful if you want to know exactly what each die\n"
		"said when it was rolled.\n"
		" \n"
		"Syntax of the dice is exactly the same as the syntax of\n"
		"ROLL.",
		/* DICE_HELP_EXCALC */
		"Syntax: EXCALC dice [[channel] comment]\n"
		" \n"
		"This command is identical like EXROLL (see /msg %s\n"
		"HELP EXROLL for more information on how to use this and\n"
		"EXROLL), except the results are not rounded off and are\n"
		"displayed as is.",
		/* DICE_HELP_DND3ECHAR */
		"Syntax: DND3ECHAR [[channel] comment]\n"
		" \n"
		"This command is performs the rolls needs to create a DnD3e\n"
		"character, which consists of 6 sets of 4d6, the lowest\n"
		"result of each set being discarded. The discarded die will\n"
		"be shown in reverse, so you can still see all 4 dice and\n"
		"which was removed. The syntax for channel and comment is the\n"
		"same as with the ROLL command (see /msg %s HELP ROLL\n"
		"for more information on how to use this and ROLL).\n"
		" \n"
		"Example:\n"
		"  Do a DnD3e Character: /msg %s DND3ECHAR\n"
		"    {4d6=(3 5 5 6)}=16\n"
		"  (The above is basically 19 minus the lowest of 3)",
		/* DICE_HELP_EARTHDAWN */
		"Syntax: EARTHDAWN step[+karma] [[channel] comment]\n"
		" \n"
		"This command performs the rolls needed for Earthdawn.\n"
		"Earthdawn's rolling system works on the concept of a step\n"
		"table, with different rolls depending on the given step.\n"
		"Step must be an integer value and must be between 1 and 100.\n"
		"Karma is an optional modifier, and if given, must have come\n"
		"right after the step and have a plus between step and karma.\n"
		"The syntax for channel and comment is the same as with the\n"
		"ROLL command (see /msg %s HELP ROLL for more\n"
		"information on how to use this and ROLL).\n"
		" \n"
		"NOTE: Unlike the ROLL and EXROLL commands, EARTHDAWN does\n"
		"not allow you to use the ~ to specify multiple throws.\n"
		" \n"
		"Examples:\n"
		"  /msg %s EARTHDAWN 5\n"
		"    Same as /msg %s EXROLL 1d8\n"
		"  /msg %s EARTHDAWN 100+1d6\n"
		"    Same as /msg %s EXROLL (4d20+6d10+4d8)+1d6",
		/* DICE_HELP_SET */
		"Syntax: SET option parameters\n"
		" \n"
		"Currently allows you to set who has %s access.\n"
		" \n"
		"    IGNORE         Change ignored/allowed setting\n"
		" \n"
		"Type /msg %s HELP SET option for more information\n"
		"on a specific option.\n"
		" \n"
		"Note: Access to these commands are limited. See help on each\n"
		"option for details.",
		/* DICE_HELP_SET_IGNORE */
		"Syntax: SET IGNORE channel {ON|OFF}\n"
		" \n"
		"This will allow a channel to be set to ignore the use of\n"
		"%s commands inside the channel. If ON is given, then\n"
		"%s will be ignored, otherwise, it will be allowed.\n"
		" \n"
		"If the channel in question is registered, then only the\n"
		"channel's founder (or someone with founder-level access) can\n"
		"use this option. The option set will be persistant as long\n"
		"as the channel stays registered in %s. If the channel\n"
		"is unregistered, then any ops in the channel can set this\n"
		"option, but it will only last as long as the channel is\n"
		"active.",
		/* DICE_SERVADMIN_HELP_SET_IGNORE */
		"Syntax: SET IGNORE {channel|nick} {ON|OFF}\n"
		" \n"
		"This will allow a channel or nick to be set to ignore the\n"
		"use of %s commands inside the channel or by that user.\n"
		"If ON is given, then %s will be ignored, otherwise, it\n"
		"will be allowed.\n"
		" \n"
		"If the channel in question is registered, then only a\n"
		"services admin or the channel's founder (or someone with\n"
		"founder-level access) can use this option. The option set\n"
		"will be persistant as long as the channel stays registered\n"
		"in %s. If the channel is unregistered, then any ops in\n"
		"the channel can set this option, but it will only last as\n"
		"long as the channel is active.\n"
		" \n"
		"A nick may also be given, but this option is limited to\n"
		"services operators and up. If the nick in question is\n"
		"registered, then the option will be set in %s so it\n"
		"will stay persistant as long as the nick stays registered.\n"
		"If the nick is unregistered, then it will only last as long\n"
		"as the user is online.",
		/* DICE_SERVADMIN_HELP_STATUS */
		"Syntax: STATUS {channel|nick}\n"
		" \n"
		"This will give you the allowed or ignored status of a\n"
		"channel or a nick, depending on which one you give. It will\n"
		"also tell you if that status is on an online nick/channel,\n"
		"or set in services due to the nick not being online.",
		/* DICE_SERVADMIN_HELP_LIST */
		"Syntax: LIST {IGNORE|ALLOW|ALL} what pattern [{REG|UNREG}]\n"
		" \n"
		"This will display a list of channels or nicks depending on\n"
		"what options you give. The first parameter is what access\n"
		"types to show, either all ignored, all allowed, or just all.\n"
		" \n"
		"what MUST be one of the following:\n"
		" \n"
		"    CHANNELS       Shows channels based on the display type\n"
		"    NICKS          Shows nicks based on the display type\n"
		" \n"
		"pattern is the mask you want to view.\n"
		" \n"
		"The final parameter is optional, if given, it will allow you\n"
		"to choose if only registered or unregistered entries are\n"
		"shown on the list."
	};
	moduleInsertLanguage(LANG_EN_US, DICE_NUM_STRINGS, langtable_en_us);
}

/** Module initialization.
 * @param argc The number of arguments passed to the module
 * @param argv The arguments themselves
 *
 * Initializes DiceServ by hooking into PRIVMSG, creating the pseudo-client, creating the commands, hooking to events,
 * loading the configuration, adding the language strings, and loading the database.
 */
int AnopeInit(int argc, char **argv)
{
	Command *c = NULL;
	Message *msg = NULL;
	EvtHook *hook = NULL;
	int status;

#ifdef LIBOL
	ignored_channels.compare = ignored_channels_compare;
	ignored_channels.free = free;
#endif

	if (UseTokens)
	{
		msg = createMessage("!", my_privmsg);
		status = moduleAddMessage(msg, MOD_HEAD);
		if (status != MOD_ERR_OK)
		{
			alog("Unable to bind to the ! token for PRIVMSG!");
			return MOD_STOP;
		}
	}

	msg = createMessage("PRIVMSG", my_privmsg);
	status = moduleAddMessage(msg, MOD_HEAD);
	if (status != MOD_ERR_OK)
	{
		alog("Unable to bind to PRIVMSG!");
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	do_load_config();

	anope_cmd_bot_nick(s_DiceServ, ServiceUser, ServiceHost, "Dice Roll Service", ircd->botservmode);

	c = createCommand("HELP", do_help, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("ROLL", do_roll, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_roll);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("ROLL EXPRESSIONS", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_roll_expressions);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("FUNCTIONS", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_functions);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("CALC", do_calc, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_calc);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("EXROLL", do_exroll, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_exroll);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("EXCALC", do_excalc, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_excalc);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("DND3ECHAR", do_dnd3echar, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_dnd3echar);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("EARTHDAWN", do_earthdawn, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_earthdawn);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("SET", do_set, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_set);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("SET IGNORE", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_set_ignore);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("STATUS", do_status, is_services_oper, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_status);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	c = createCommand("LIST", do_list, is_services_oper, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_help_list);
	moduleAddCommand(DiceServ_cmdTable, c, MOD_UNIQUE);

	/* The following two commands are additions to NickServ's and ChanServ's INFO commands, to display DiceServ's ignore status in INFO */
	c = createCommand("INFO", do_ns_info, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_TAIL);

	c = createCommand("INFO", do_cs_info, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_TAIL);

	hook = createEventHook(EVENT_DB_SAVING, do_save_db_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_DB_BACKUP, do_backup_db_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_NEWNICK, do_nick_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_CHANGE_NICK, do_nick_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_NICK_REGISTERED, do_nick_register_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_CHAN_REGISTERED, do_chan_register_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_JOIN_CHANNEL, do_chan_join_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_PART_CHANNEL, do_chan_part_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	moduleAddEventHook(hook);

	Mersenne_RandomInit(time(0));
	do_add_languages();
	do_load_db();
	return MOD_CONT;
}

/** Module deinitialization.
 *
 * Upon Anope shutdown or unload of this module, we make the pesudo-client quit and save it's database, as well as do memory cleanup.
 */
void AnopeFini(void)
{
	do_save_db();
	anope_cmd_quit(s_DiceServ, "Module Unloaded!");
	free(s_DiceServ);
	free(DiceServDBName);
	if (DiceExOutput)
		free(DiceExOutput);
	if (DiceShortExOutput)
		free(DiceShortExOutput);
	clear_chan_ignores();
}
