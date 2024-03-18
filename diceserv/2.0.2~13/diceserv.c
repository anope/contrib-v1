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
 * Requires: Anope-1.9.1 or Anope-1.9.1-p1 (Will not work with earlier or any
 *                                          other 1.9.x)
 * ----------------------------------------------------------------------------
 * Description:
 *
 * A dice rolling pseudo-client, mainly useful for playing pen & paper RPGs,
 * such as Dungeons and Dragons, over IRC.
 * ----------------------------------------------------------------------------
 * Usage:
 *
 * This module has 3 configurable options.
 *
 * ----- Put these lines in your services.conf --------------------------------

module { name = "DiceServ" }
diceserv
{
	//
	// The filename of DiceServ's database. The path is relative to the data
	// directory of your services. If not given, defaults to "diceserv.db".
	//
	#database = "diceserv.db"

	//
	// The nickname of the DiceServ pseudo-client. If not given, defaults to
	// "DiceServ".
	//
	#name = "DiceServ"

	//
	// Network-wide setting to allow channel operators to also have the ability
	// to have DiceServ ignore a channel instead of just the channel founder.
	//
	#chanopcanignore = "yes"
}

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

/* RequiredLibraries: m */

#include <stack>
#include <utility>
#include <cctype>
#ifdef _WIN32
# include <float.h>
#endif
#include <cmath>
#include "module.h"
#include "hashcomp.h"

#ifdef tolower
# undef tolower
#endif
#ifdef max
# undef max
#endif
#ifdef min
# undef min
#endif

static const char *const AUTHOR = "Naram Qashat (CyberBotX)";
static const char *const VERSION = "v2.0.2";

static const int DICE_MAX_TIMES = 25;
static const unsigned DICE_MAX_DICE = 99999;
static const unsigned DICE_MAX_SIDES = 99999;
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

static char *s_DiceServ; /* DiceServ client name, I don't like that I had to make this a char* to stop ERRORS (even worse than the 1.8.x version) :/ */
static std::string	DiceServDBName, /* Database file name */
					DiceExOutput, /* Extended output string */
					DiceShortExOutput, /* Shorter extended output string */
					RollSource, /* Source of the roll when SourceIsBot is set */
					DiceErrStr; /* Error string */
static CommandHash *DiceServ_cmdTable[MAX_CMD_HASH]; /* Commands for DiceServ */
static DICE_ERRS DiceErrCode; /* Error code, see DICE_ERRS above */
static int	DiceErrNum, /* Value that caused last error, if needed */
			DnDmods[6][2]; /* Array storing all 6 sets of DnD3e character rolls, index 0 storing the actual values (modified), index 1 storing the mod value */
static unsigned	DiceErrPos, /* Position of last error, if needed */
				DiceEx, /* 1 if any sort of extended roll is requsted, 2 if we were doing an extended roll but the extended output string overflowed as we are going to use the shorter extended output string,
					3 if we are doing an extended roll but both extended output strings overflowed, 0 for no extended roll */
				DnDresults[4], /* Array storing the 4 rolls in a single set for a DnD3e character roll */
				DnDPos = 0; /* Index within above array */
static bool	SourceIsBot, /* true when DiceServ is invoked through a BotServ fantasy command */
			DoDnD3eChar, /* true when a DnD3e character roll is requested */
			DoEarthdawn, /* true when an Earthdawn roll is requested */
			DiceServChanOpCanIgnore; /* true when channel operators should be allowed to have DiceServ ignore a channel, otherwise only channel owners can set that */
static Module *me; /* Pointer to this module */
static BotInfo *DiceServ = NULL; /* Pointer to the DiceServ bot */
static User *CurrUser = NULL; /* Current user that is using DiceServ */

/* Ignored channel handling */
static std::vector<ci::string> ignored_channels; /* Have to do this since the Channel struct doesn't inherit the Extensible class like NickCore, ChannelInfo, and User do */

/** Check the ignored channels array for the given channel name.
 * @param chan Name of the channel to search for
 * @return -1 if the channel is not found in the array, otherwise the index within the array
 *
 * NOTE: -1 will also be returned when the array is empty.
 */
static int find_chan_ignore(const ci::string &chan)
{
	if (ignored_channels.empty())
		return -1;

	for (unsigned i = 0, len = ignored_channels.size(); i < len; ++i)
		if (chan == ignored_channels[i])
			return i;

	return -1;
}

/** Deletes the given channel name from the ignored channels array.
 * @param chan Name of the channel to delete
 *
 * If the channel is not found, nothing will be done.
 */
static void del_chan_ignore(const ci::string &chan)
{
	int index = find_chan_ignore(chan);

	if (index == -1)
		return;

	ignored_channels.erase(ignored_channels.begin() + index);
}

/** Adds the given channel name to the ignored channels array.
 * @param chan Name of the channel to add
 *
 * This will also delete an existing entry before trying to add the channel.
 */
static void add_chan_ignore(const ci::string &chan)
{
	del_chan_ignore(chan);

	ignored_channels.push_back(chan);
}

/** Determine if the double-precision floating point value is infinite or not.
 * @param num The double-precision floating point value to check
 * @return true if the value is infinite, false otherwise
 */
static bool is_infinite(double num)
{
#ifdef _WIN32
	int fpc = _fpclass(num);
	return fpc == _FPCLASS_NINF || fpc == _FPCLASS_PINF;
#else
	return std::isinf(num);
#endif
}

/** Determine if the double-precision floating point value is NaN (Not a Number) or not.
 * @param num The double-precision floating point value to check
 * @return true if the value is NaN, false otherwise
 */
static bool is_notanumber(double num)
{
#ifdef _WIN32
	int fpc = _fpclass(num);
	return fpc == _FPCLASS_SNAN || fpc == _FPCLASS_QNAN;
#else
	return std::isnan(num);
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
	return static_cast<double>(static_cast<int>(num));
}
#endif

static const int MERS_N = 624;
static const int MERS_M = 397;
static const int MERS_R = 31;
static const int MERS_U = 11;
static const int MERS_S = 7;
static const int MERS_T = 15;
static const int MERS_L = 18;
static const uint32 MERS_A = 0x9908B0DF;
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
	/* Initialize and seed */
	Mersenne_Init0(seed);

	/* Randomize some more */
	for (int i = 0; i < 37; ++i)
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
	return static_cast<double>(Mersenne_BRandom()) * (1.0 / (65536.0 * 65536.0));
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
	int r = static_cast<int>(static_cast<double>(static_cast<uint32>(max - min + 1)) * Mersenne_Random() + min);
	if (r > max)
		r = max;
	return r;
}

/** Determine if the given character is a number.
 * @param chr Character to check
 * @return true if the character is a number, false otherwise
 */
inline static bool is_number(char chr) { return (chr >= '0' && chr <= '9') || chr == '.'; }
/** Determine if the given character is a multiplication or division operator.
 * @param chr Character to check
 * @return true if the character is a multiplication or division operator, false otherwise
 */
inline static bool is_muldiv(char chr) { return chr == '%' || chr == '/' || chr == '*'; }
/** Determine if the given character is an addition or subtraction operator.
 * @param chr Character to check
 * @return true if the character is an addition or subtraction operator, false otherwise
 */
inline static bool is_plusmin(char chr) { return chr == '+' || chr == '-'; }
/** Determine if the given character is an operator of any sort, except for parenthesis.
 * @param chr Character to check
 * @return true if the character is a non-parenthesis operator, false otherwise
 */
inline static bool is_op_noparen(char chr) { return is_plusmin(chr) || is_muldiv(chr) || chr == '^' || chr == 'd'; }
/** Determine if the given character is an operator of any sort.
 * @param chr Character to check
 * @return true if the character is an operator, false otherwise
 */
inline static bool is_operator(char chr) { return chr == '(' || chr == ')' || is_op_noparen(chr); }
/** Determine if the substring portion of the given string is a function.
 * @param str String to check
 * @param pos Starting position of the substring to check, defaults to 0
 * @return 0 if the string isn't a function, or a number corresponding to the length of the function name
 */
inline static unsigned is_function(const ci::string &str, unsigned pos = 0)
{
	/* We only need a 5 character substring as that's the largest substring we will be looking at */
	ci::string func = str.substr(pos, 5);
	/* acosh, asinh, atanh, floor, log10, round, trunc */
	ci::string func_5 = func.substr(0, 5);
	if (func_5 == "acosh" || func_5 == "asinh" || func_5 == "atanh" || func_5 == "floor" || func_5 == "log10" || func_5 == "round" || func_5 == "trunc")
		return 5;
	/* acos, asin, atan, cbrt, ceil, cosh, rand, sinh, sqrt, tanh */
	ci::string func_4 = func.substr(0, 4);
	if (func_4 == "acos" || func_4 == "asin" || func_4 == "atan" || func_4 == "cbrt" || func_4 == "ceil" || func_4 == "cosh" || func_4 == "rand" || func_4 == "sinh" || func_4 == "sqrt" || func_4 == "tanh")
		return 4;
	/* abs, cos, deg, exp, fac, log, max, min, rad, sin, tan */
	ci::string func_3 = func.substr(0, 3);
	if (func_3 == "abs" || func_3 == "cos" || func_3 == "deg" || func_3 == "exp" || func_3 == "fac" || func_3 == "log" || func_3 == "max" || func_3 == "min" || func_3 == "rad" || func_3 == "sin" || func_3 == "tan")
		return 3;
	/* None of the above */
	return 0;
}
/** Determine the number of arguments that the given function needs.
 * @param str Function string to check
 * @return Returns 1 except for the min, max, and rand functions which return 2.
 */
inline static unsigned function_argument_count(const ci::string &str)
{
	if (str == "max" || str == "min" || str == "rand")
		return 2;
	return 1;
}
/** Determine if the substring portion of the given string is a constant (currently only e and pi).
 * @param str String to check
 * @param pos Starting position of the substring to check, defaults to 0
 * @return 0 if the string isn't a constant, or a number corresponding to the length of the constant's name
 */
inline static unsigned is_constant(const ci::string &str, unsigned pos = 0)
{
	/* We only need a 2 character substring as that's the largest substring we will be looking at */
	ci::string constant = str.substr(pos, 2);
	/* pi */
	if (constant.substr(0, 2) == "pi")
		return 2;
	/* e */
	if (constant.substr(0, 1) == "e")
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
inline static int would_pop(const ci::string &adding, const ci::string &topstack)
{
	if (adding.empty())
		return topstack.empty() || topstack == "(" ? 0 : 1;
	if (is_function(adding))
		return 0;
	if (topstack.empty() || topstack == "(")
		return 0;
	if (is_function(topstack))
		return 1;
	if (topstack == adding && adding != "^")
		return 1;
	switch (adding[0])
	{
		case 'd':
			return 0;
		case '^':
			return topstack == "^" || topstack == "d" ? 1 : 0;
		case '%':
		case '/':
		case '*':
			return topstack == "^" || topstack == "d" || is_muldiv(topstack[0]) ? 1 : 0;
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
	char buf[20] = "";
	/* Add <x>d<y>=( to the extended output string */
	if (DiceEx == 1)
	{
		snprintf(buf, 20, "%ud%u=(", num, sides);
		DiceShortExOutput += buf;
		DiceExOutput += buf;
	}
	bool bonus = false;
	unsigned sum = 0;
	for (int i = 0; i < num; ++i)
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
				std::string tmpbuf = buf;
				/* If the last character of the extended output string is a [, then this is the first bonus, we remove the space (if any) from the buffer */
				if (DiceExOutput[DiceExOutput.size() - 1] == '[')
				{
					if (*buf == ' ')
						tmpbuf = tmpbuf.substr(1);
				}
				/* Otherwise, if it's the first die in the set, we force add a space */
				else if (!i)
					tmpbuf = std::string(" ") + buf;
				DiceExOutput += tmpbuf;
			}
			/* All other dice roll types, we just add the buffer to the extended output string */
			else
				DiceExOutput += buf;
		}
		/* When doing a DnD3e character roll, add the current roll to the results array */
		if (DoDnD3eChar)
			DnDresults[DnDPos++] = number;
		/* When doing an Earthdawn roll, we have to check for the possiblity of bonus rolls when the roll maxes out */
		if (DoEarthdawn)
		{
			/* When the number rolled is equal to the sides, the roll has maxed out */
			if (number == sides)
			{
				/* If we aren't yet in a bonus roll, we add the wording to the extended output string */
				if (!bonus)
					DiceExOutput += " Bonus[";
				/* We set that we are in a bonus roll, and reduce i by one to "repeat" the current roll again */
				bonus = true;
				--i;
			}
			/* Otherwise, the roll wasn't maxed, we close the bonus part of the buffer if it was open and set that we are no longer doing bonus rolls */
			else
			{
				if (bonus)
					DiceExOutput += ']';
				bonus = false;
			}
		}
		sum += number;
	}
	if (DiceEx == 1)
	{
		/* Add the sum to the shorter extended output string */
		snprintf(buf, 20, "%u", sum);
		DiceShortExOutput += buf;

		/* Close the extended output string with a ) */
		DiceShortExOutput += ") ";
		DiceExOutput += ") ";
	}
	return sum;
}

/** Convert a double-precision floating point value to a string.
 * @param num The value to convert
 * @return A string containing the value
 */
static std::string dtoa(double num)
{
	char temp[33];
	snprintf(temp, sizeof(temp), "%.*g", static_cast<int>(sizeof(temp) - 7), num);
	return temp;
}

/** Round a value to the given number of decimals, originally needed for Windows but also used for other OSes as well due to undefined references.
 * @param val The value to round
 * @param decimals The number of digits after the decimal point, defaults to 0
 * @return The rounded value
 *
 * NOTE: Function is a slightly modified form of the code from this page:
 * http://social.msdn.microsoft.com/forums/en-US/vclanguage/thread/a7d4bf31-6c32-4b25-bc76-21b29f5287a1/
 */
static double my_round(double val, unsigned decimals = 0)
{
	if (!val) /* val must be different from zero to avoid division by zero! */
		return 0;
	double sign = fabs(val) / val; /* we obtain the sign to calculate positive always */
	double tempval = fabs(val * pow(10.0, static_cast<double>(decimals))); /* shift decimal places */
	unsigned tempint = static_cast<unsigned>(tempval);
	double decimalpart = tempval - tempint; /* obtain just the decimal part */
	if (decimalpart >= 0.5)
		tempval = ceil(tempval); /* next integer number if greater or equal to 0.5 */
	else
		tempval = floor(tempval); /* otherwise stay in the current interger part */
	return (tempval * pow(10.0, static_cast<double>(-static_cast<int>(decimals)))) * sign; /* shift again to the normal decimal places */
}

/** Structure to store the infix notation string as well as the positions each character is compared to the original input */
struct Infix
{
	ci::string str;
	std::vector<unsigned> positions;

	Infix(const ci::string &newStr, std::vector<unsigned> newPositions)
	{
		this->str = newStr;
		this->positions = newPositions;
	}
	Infix(const ci::string &newStr, unsigned newPositions[], unsigned num)
	{
		this->str = newStr;
		this->positions = std::vector<unsigned>(newPositions, newPositions + sizeof(unsigned) * num);
	}
};

/** Fix an infix notation equation.
 * @param infix The original infix notation equation
 * @return A fixed infix notation equation
 *
 * This will convert a single % to 1d100, place a 1 in front of any d's that have no numbers before them, change all %'s after a d into 100,
 * add *'s for implicit multiplication, and convert unary -'s to _ for easier parsing later.
 */
static Infix fix_infix(const ci::string &infix)
{
	if (infix == "%")
	{
		unsigned tmp[] = {0, 0, 0, 0, 0};
		Infix newinfix("1d100", tmp, 5);
		return newinfix;
	}
	bool prev_was_func = false, prev_was_const = false;
	ci::string newinfix;
	std::vector<unsigned> positions;
	unsigned len = infix.size();
	for (unsigned x = 0; x < len; ++x)
	{
		/* Check for a function, and skip it if it exists */
		unsigned func = is_function(infix, x);
		if (func)
		{
			if (x && is_number(infix[x - 1]))
			{
				newinfix += '*';
				positions.push_back(x);
			}
			newinfix += infix.substr(x, func);
			for (unsigned y = 0; y < func; ++y)
				positions.push_back(x + y);
			x += func - 1;
			prev_was_func = true;
			continue;
		}
		/* Check for a constant, and skip it if it exists */
		unsigned constant = is_constant(infix, x);
		if (constant)
		{
			if (x && is_number(infix[x - 1]))
			{
				newinfix += '*';
				positions.push_back(x);
			}
			newinfix += infix.substr(x, constant);
			for (unsigned y = 0; y < constant; ++y)
				positions.push_back(x + y);
			if (x + constant < len && (is_number(infix[x + constant]) || is_constant(infix, x + constant) || is_function(infix, x + constant)))
			{
				newinfix += '*';
				positions.push_back(x + constant);
			}
			x += constant - 1;
			prev_was_const = true;
			continue;
		}
		char curr = std::tolower(infix[x]);
		if (curr == 'd')
		{
			positions.push_back(x);
			if (!x)
			{
				newinfix += "1d";
				positions.push_back(x);
			}
			else
			{
				if (!is_number(infix[x - 1]) && infix[x - 1] != ')' && !prev_was_const)
				{
					newinfix += '1';
					positions.push_back(x);
				}
				newinfix += 'd';
			}
			if (x != len - 1 && infix[x + 1] == '%')
			{
				newinfix += "100";
				++x;
				positions.push_back(x);
				positions.push_back(x);
			}
		}
		else if (curr == '(')
		{
			if (x && !prev_was_func && (is_number(infix[x - 1]) || prev_was_const))
			{
				newinfix += '*';
				positions.push_back(x);
			}
			newinfix += '(';
			positions.push_back(x);
		}
		else if (curr == ')')
		{
			newinfix += ')';
			positions.push_back(x);
			if (x != len - 1 && (is_number(infix[x + 1]) || infix[x + 1] == '(' || is_constant(infix, x + 1)))
			{
				newinfix += '*';
				positions.push_back(x);
			}
		}
		else if (curr == '-')
		{
			positions.push_back(x);
			if (x != len - 1 && (!x ? 1 : is_op_noparen(std::tolower(infix[x - 1])) || infix[x - 1] == '(' || infix[x - 1] == ','))
			{
				if (infix[x + 1] == '(' || is_function(infix, x + 1))
				{
					newinfix += "0-";
					positions.push_back(x);
				}
				else if (is_number(infix[x + 1]) || is_constant(infix, x + 1))
					newinfix += '_';
				else
					newinfix += '-';
			}
			else
				newinfix += '-';
		}
		else
		{
			newinfix += curr;
			positions.push_back(x);
		}
		prev_was_func = prev_was_const = false;
	}
	positions.push_back(len);
	return Infix(newinfix, positions);
}

/** Validate an infix notation equation.
 * @param infix The infix notation equation to validate
 * @return false for an invalid equation, true for a valid one
 *
 * The validation is as follows:
 * - All functions must have an open parenthesis after them.
 * - A comma must be prefixed by a number or close parenthesis and must be suffixed by a number, open parenthesis, _ for unary minus, constant, or function.
 * - All non-parentheis operators must be prefixed by a number or close parenthesis and suffixed by a number, open parenthesis, _ for unary minus, constant, or function.
 * - All open parentheses must be prefixed by an operator, open parenthesis, or comma and suffixed by a number, an open parenthesis, _ for unary minus, constant, or function.
 * - All close parentheses must be prefixed by a number or close parenthesis and suffixed by an operator, close parenthesis, or comma.
 */
static bool check_infix(const Infix &infix)
{
	bool prev_was_func = false, prev_was_const = false;
	for (unsigned x = 0, len = infix.str.size(); x < len; ++x)
	{
		unsigned position = infix.positions[x];
		/* Check for a function, and skip it if it exists */
		unsigned func = is_function(infix.str, x);
		if (func)
		{
			if ((x + func < len && infix.str[x + func] != '(') || x + func >= len)
			{
				DiceErrPos = infix.positions[x + func >= len ? len : x + func];
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_NOPARENAFTERFUNCTION);
				return false;
			}
			x += func - 1;
			prev_was_func = true;
			continue;
		}
		/* Check for a constant, and skip it if it exists */
		unsigned constant = is_constant(infix.str, x);
		if (constant)
		{
			x += constant - 1;
			prev_was_const = true;
			continue;
		}
		if (infix.str[x] == ',')
		{
			if (!x ? 1 : !is_number(infix.str[x - 1]) && infix.str[x - 1] != ')' && !prev_was_const)
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_BEFORECOMMA);
				return false;
			}
			if (x == len - 1 ? 1 : !is_number(infix.str[x + 1]) && infix.str[x + 1] != '(' && infix.str[x + 1] != '_' && !is_constant(infix.str, x + 1) && !is_function(infix.str, x + 1))
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_AFTERCOMMA);
				return false;
			}
		}
		else if (is_op_noparen(infix.str[x]))
		{
			if (!x ? 1 : !is_number(infix.str[x - 1]) && infix.str[x - 1] != ')' && !prev_was_const)
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_BEFOREOPERATOR);
				return false;
			}
			if (x == len - 1 ? 1 : !is_number(infix.str[x + 1]) && infix.str[x + 1] != '(' && infix.str[x + 1] != '_' && !is_constant(infix.str, x + 1) && !is_function(infix.str, x + 1))
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_AFTEROPERATOR);
				return false;
			}
		}
		else if (infix.str[x] == '(')
		{
			if (x && !is_op_noparen(infix.str[x - 1]) && infix.str[x - 1] != '(' && infix.str[x - 1] != ',' && !prev_was_func)
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_BEFOREOPENPAREN);
				return false;
			}
			if (x != len - 1 && !is_number(infix.str[x + 1]) && infix.str[x + 1] != '(' && infix.str[x + 1] != '_' && !is_constant(infix.str, x + 1) && !is_function(infix.str, x + 1))
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_AFTEROPENPAREN);
				return false;
			}
		}
		else if (infix.str[x] == ')')
		{
			if (x && !is_number(infix.str[x - 1]) && infix.str[x - 1] != ')' && !prev_was_const)
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_BEFORECLOSEPAREN);
				return false;
			}
			if (x != len - 1 && !is_op_noparen(infix.str[x + 1]) && infix.str[x + 1] != ')' && infix.str[x + 1] != ',')
			{
				DiceErrPos = position;
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_AFTERCLOSEPAREN);
				return false;
			}
		}
		else if (!is_number(infix.str[x]) && !is_muldiv(infix.str[x]) && !is_plusmin(infix.str[x]) && !is_operator(infix.str[x]) && infix.str[x] != '_')
		{
			DiceErrPos = position;
			DiceErrCode = DICE_ERR_PARSE;
			DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_INVALIDCHAR);
			return false;
		}
		prev_was_func = prev_was_const = false;
	}
	return true;
}

/** Tokenize an infix notation equation by adding spaces between operators.
 * @param infix The original infix notation equation to tokenize
 * @return A new infix notation equation with spaces between operators
 */
static Infix tokenize_infix(const Infix &infix)
{
	ci::string newinfix;
	std::vector<unsigned> positions;
	for (unsigned x = 0, len = infix.str.size(); x < len; ++x)
	{
		unsigned position = infix.positions[x], func = is_function(infix.str, x), constant = is_constant(infix.str, x);
		char curr = infix.str[x];
		if (func)
		{
			if (x && !newinfix.empty() && newinfix[newinfix.size() - 1] != ' ')
			{
				newinfix += ' ';
				positions.push_back(position);
			}
			newinfix += infix.str.substr(x, func);
			for (unsigned y = 0; y < func; ++y)
				positions.push_back(infix.positions[x + y]);
			if (x != len - 1)
			{
				newinfix += ' ';
				positions.push_back(infix.positions[x + func]);
			}
			x += func - 1;
		}
		else if (constant)
		{
			if (x && !newinfix.empty() && newinfix[newinfix.size() - 1] != ' ' && newinfix[newinfix.size() - 1] != '_')
			{
				newinfix += ' ';
				positions.push_back(position);
			}
			newinfix += infix.str.substr(x, constant);
			for (unsigned y = 0; y < constant; ++y)
				positions.push_back(infix.positions[x + y]);
			if (x != len - 1)
			{
				newinfix += ' ';
				positions.push_back(infix.positions[x + constant]);
			}
			x += constant - 1;
		}
		else if (curr == ',')
		{
			if (x && !newinfix.empty() && newinfix[newinfix.size() - 1] != ' ')
			{
				newinfix += ' ';
				positions.push_back(position);
			}
			newinfix += ',';
			positions.push_back(position);
			if (x != len - 1)
			{
				newinfix += ' ';
				positions.push_back(position);
			}
		}
		else if (is_operator(curr))
		{
			if (x && !newinfix.empty() && newinfix[newinfix.size() - 1] != ' ')
			{
				newinfix += ' ';
				positions.push_back(position);
			}
			newinfix += curr;
			positions.push_back(position);
			if (x != len - 1)
			{
				newinfix += ' ';
				positions.push_back(position);
			}
		}
		else
		{
			newinfix += curr;
			positions.push_back(position);
		}
	}
	return Infix(newinfix, positions);
}

/** Base class for values in a postfix equation */
class PostfixValueBase
{
 public:
	PostfixValueBase() { }
	virtual ~PostfixValueBase() { }
	virtual void Clear() = 0;
};

/** Version of PostfixValue for double */
class PostfixValueDouble : public PostfixValueBase
{
	double *val;
 public:
	PostfixValueDouble() : PostfixValueBase(), val(NULL) { }
	PostfixValueDouble(double *Val) : PostfixValueBase(), val(Val) { }
	PostfixValueDouble(const PostfixValueDouble &Val) : PostfixValueBase(), val(Val.val) { }
	PostfixValueDouble &operator=(const PostfixValueDouble &Val)
	{
		if (this != &Val)
			*val = *Val.val;
		return *this;
	}
	const double *Get() const
	{
		return val;
	}
	void Clear()
	{
		delete val;
	}
};

/** Version of PostfixValue for ci::string */
class PostfixValueString : public PostfixValueBase
{
	ci::string *val;
 public:
	PostfixValueString() : PostfixValueBase(), val(NULL) { }
	PostfixValueString(ci::string *Val) : PostfixValueBase(), val(Val) { }
	PostfixValueString(const PostfixValueString &Val) : PostfixValueBase(), val(Val.val) { }
	PostfixValueString &operator=(const PostfixValueString &Val)
	{
		if (this != &Val)
			*val = *Val.val;
		return *this;
	}
	const ci::string *Get() const
	{
		return val;
	}
	void Clear()
	{
		delete val;
	}
};

/** Enumeration for PostfixValue to determine it's type */
enum PostfixValueType
{
	POSTFIX_VALUE_DOUBLE,
	POSTFIX_VALUE_STRING
};

typedef std::vector<std::pair<PostfixValueBase *, PostfixValueType> > Postfix;

/** Clears the memory of the given Postfix vector.
 * @param postfix The Postfix vector to clean up
 */
static void cleanup_postfix(Postfix &postfix)
{
	for (unsigned y = 0; y < postfix.size(); ++y)
		postfix[y].first->Clear();
	postfix.clear();
}
/** Adds a double value to the Postfix vector.
 * @param postfix The Postfix vector to add to
 * @param dbl The double value we are adding
 */
static void add_to_postfix(Postfix &postfix, double dbl)
{
	double *dbl_ptr = new double(dbl);
	postfix.push_back(std::make_pair(new PostfixValueDouble(dbl_ptr), POSTFIX_VALUE_DOUBLE));
}
/** Adds an ci::string value to the Postfix vector.
 * @param postfix The Postfix vector to add to
 * @param str The ci::string value we are adding
 */
static void add_to_postfix(Postfix &postfix, const ci::string &str)
{
	ci::string *str_ptr = new ci::string(str);
	postfix.push_back(std::make_pair(new PostfixValueString(str_ptr), POSTFIX_VALUE_STRING));
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
static Postfix infix_to_postfix(const Infix &infix)
{
	Postfix postfix;
	unsigned len = infix.str.size(), x = 0;
	bool prev_was_close = false, prev_was_number = false;
	std::stack<ci::string> op_stack;
	spacesepstream tokens(infix.str.c_str());
	std::string token;
	ci::string lastone;
	while (tokens.GetToken(token))
	{
		ci::string ci_token = token.c_str();
		if (token[0] == '_')
		{
			double number = 0.0;
			if (is_constant(ci_token, 1))
			{
				if (ci_token.substr(1) == "e")
					number = -exp(1.0);
				else if (ci_token.substr(1) == "pi")
					number = -atan(1.0) * 4;
			}
			else
				number = -atof(token.substr(1).c_str());
			if (is_infinite(number) || is_notanumber(number))
			{
				DiceErrCode = is_infinite(number) ? DICE_ERR_OVERUNDERFLOW : DICE_ERR_UNDEFINED;
				cleanup_postfix(postfix);
				return postfix;
			}
			add_to_postfix(postfix, number);
			prev_was_number = true;
		}
		else if (is_number(token[0]))
		{
			double number = atof(token.c_str());
			if (is_infinite(number) || is_notanumber(number))
			{
				DiceErrCode = is_infinite(number) ? DICE_ERR_OVERUNDERFLOW : DICE_ERR_UNDEFINED;
				cleanup_postfix(postfix);
				return postfix;
			}
			add_to_postfix(postfix, number);
			prev_was_number = true;
		}
		else if (is_function(ci_token))
			op_stack.push(ci_token);
		else if (is_constant(ci_token))
		{
			double number = 0.0;
			if (ci_token == "e")
				number = exp(1.0);
			else if (ci_token == "pi")
				number = atan(1.0) * 4;
			add_to_postfix(postfix, number);
			prev_was_number = true;
		}
		else if (is_operator(token[0]))
		{
			lastone = op_stack.empty() ? "" : op_stack.top();
			if (!prev_was_number && token != "(" && token != ")" && !prev_was_close)
			{
				DiceErrPos = infix.positions[x];
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_NONUMBERBEFOREOPERATOR);
				cleanup_postfix(postfix);
				return postfix;
			}
			prev_was_number = false;
			if (token == "(")
			{
				op_stack.push(ci_token);
				prev_was_close = false;
			}
			else if (token == ")")
			{
				while (would_pop(ci_token, lastone))
				{
					add_to_postfix(postfix, lastone);
					op_stack.pop();
					lastone = op_stack.empty() ? "" : op_stack.top();
				}
				if (lastone != "(")
				{
					DiceErrPos = infix.positions[x];
					DiceErrCode = DICE_ERR_PARSE;
					DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_NOTENOUGHOPENPARENS);
					cleanup_postfix(postfix);
					return postfix;
				}
				else
					op_stack.pop();
				prev_was_close = true;
			}
			else
			{
				if (!would_pop(ci_token, lastone))
					op_stack.push(ci_token);
				else
				{
					while (would_pop(ci_token, lastone))
					{
						add_to_postfix(postfix, lastone);
						op_stack.pop();
						lastone = op_stack.empty() ? "" : op_stack.top();
					}
					op_stack.push(ci_token);
				}
				prev_was_close = false;
			}
		}
		else if (token[0] == ',')
		{
			lastone = op_stack.empty() ? "" : op_stack.top();
			while (would_pop(ci_token, lastone))
			{
				add_to_postfix(postfix, lastone);
				op_stack.pop();
				lastone = op_stack.empty() ? "" : op_stack.top();
			}
			if (lastone != "(")
			{
				DiceErrPos = infix.positions[x];
				DiceErrCode = DICE_ERR_PARSE;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_INVALIDCOMMA);
				cleanup_postfix(postfix);
				return postfix;
			}
			else
			{
				ci::string paren = lastone;
				op_stack.pop();
				lastone = op_stack.empty() ? "" : op_stack.top();
				if (!is_function(lastone))
				{
					DiceErrPos = infix.positions[x];
					DiceErrCode = DICE_ERR_PARSE;
					DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_INVALIDCOMMA);
					cleanup_postfix(postfix);
					return postfix;
				}
				else
					op_stack.push(paren);
			}
		}
		else
		{
			DiceErrPos = infix.positions[x];
			DiceErrCode = DICE_ERR_PARSE;
			DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_INVALIDCHAR);
			cleanup_postfix(postfix);
			return postfix;
		}
		x += token.size() + (x ? 1 : 0);
	}
	if (!op_stack.empty())
	{
		lastone = op_stack.top();
		while (would_pop("", lastone))
		{
			add_to_postfix(postfix, lastone);
			op_stack.pop();
			if (op_stack.empty())
				break;
			else
				lastone = op_stack.top();
		}
		if (!op_stack.empty())
		{
			DiceErrPos = infix.positions[len];
			DiceErrCode = DICE_ERR_PARSE;
			DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_TOOMANYOPENPARENS);
			cleanup_postfix(postfix);
			return postfix;
		}
	}
	return postfix;
}

/** Evaluate a postfix notation equation.
 * @param The postfix notation equation to evaluate
 * @return The final result after calcuation of the equation
 *
 * The evaluation pops a single value from the operand stack for a function, and 2 values from the operand stack for an operator. The result
 * of either one is placed back on the operand stack, hopefully leaving a single result at the end.
 */
static double eval_postfix(const Postfix &postfix)
{
	double val = 0;
	std::stack<double> num_stack;
	for (unsigned x = 0, len = postfix.size(); x < len; ++x)
	{
		if (postfix[x].second == POSTFIX_VALUE_STRING)
		{
			const PostfixValueString *str = dynamic_cast<const PostfixValueString *>(postfix[x].first);
			const ci::string *token_ptr = str->Get();
			ci::string token = token_ptr ? *token_ptr : "";
			if (token.empty())
			{
				DiceErrCode = DICE_ERR_STACK;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_EMPTYTOKEN);
				return 0;
			}
			if (is_function(token))
			{
				unsigned function_arguments = function_argument_count(token);
				if (num_stack.empty() || num_stack.size() < function_arguments)
				{
					DiceErrCode = DICE_ERR_STACK;
					DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_NOTENOUGHFUNCTIONARGS);
					return 0;
				}
				double val1 = num_stack.top();
				num_stack.pop();
				if (token == "abs")
					val = fabs(val1);
				else if (token == "acos")
				{
					if (fabs(val1) > 1)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						return 0;
					}
					val = acos(val1);
				}
				else if (token == "acosh")
				{
					if (val1 < 1)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						return 0;
					}
					val = acosh(val1);
				}
				else if (token == "asin")
				{
					if (fabs(val1) > 1)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						return 0;
					}
					val = asin(val1);
				}
				else if (token == "asinh")
					val = asinh(val1);
				else if (token == "atan")
					val = atan(val1);
				else if (token == "atanh")
				{
					if (fabs(val1) >= 1)
					{
						DiceErrCode = fabs(val1) == 1 ? DICE_ERR_DIV0 : DICE_ERR_UNDEFINED;
						return 0;
					}
					val = atanh(val1);
				}
				else if (token == "cbrt")
					val = cbrt(val1);
				else if (token == "ceil")
					val = ceil(val1);
				else if (token == "cos")
					val = cos(val1);
				else if (token == "cosh")
					val = cosh(val1);
				else if (token == "deg")
					val = val1 * 45.0 / atan(1.0);
				else if (token == "exp")
					val = exp(val1);
				else if (token == "fac")
				{
					if (static_cast<int>(val1) < 0)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						return 0;
					}
					val = 1;
					for (unsigned n = 2; n <= static_cast<unsigned>(val1); ++n)
						val *= n;
				}
				else if (token == "floor")
					val = floor(val1);
				else if (token == "log")
				{
					if (val1 <= 0)
					{
						DiceErrCode = DICE_ERR_DIV0;
						return 0;
					}
					val = log(val1);
				}
				else if (token == "log10")
				{
					if (val1 <= 0)
					{
						DiceErrCode = DICE_ERR_DIV0;
						return 0;
					}
					val = log10(val1);
				}
				else if (token == "max")
				{
					double val2 = val1;
					val1 = num_stack.top();
					num_stack.pop();
					val = std::max(val1, val2);
				}
				else if (token == "min")
				{
					double val2 = val1;
					val1 = num_stack.top();
					num_stack.pop();
					val = std::min(val1, val2);
				}
				else if (token == "rad")
					val = val1 * atan(1.0) / 45.0;
				else if (token == "rand")
				{
					double val2 = val1;
					val1 = num_stack.top();
					num_stack.pop();
					if (val1 > val2)
					{
						double tmp = val2;
						val2 = val1;
						val1 = tmp;
					}
					val = Mersenne_IRandom(static_cast<int>(val1), static_cast<int>(val2));
					char buf[40] = "";
					snprintf(buf, 40, "rand(%d,%d)=(%d) ", static_cast<int>(val1), static_cast<int>(val2), static_cast<int>(val));
					DiceShortExOutput += buf;
					DiceExOutput += buf;
				}
				else if (token == "round")
					val = my_round(val1);
				else if (token == "sin")
					val = sin(val1);
				else if (token == "sinh")
					val = sinh(val1);
				else if (token == "sqrt")
				{
					if (val1 < 0)
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						return 0;
					}
					val = sqrt(val1);
				}
				else if (token == "tan")
				{
					if (!fmod(val1 + 2 * atan(1.0), atan(1.0) * 4))
					{
						DiceErrCode = DICE_ERR_UNDEFINED;
						return 0;
					}
					val = tan(val1);
				}
				else if (token == "tanh")
					val = tanh(val1);
				else if (token == "trunc")
					val = trunc(val1);
				if (is_infinite(val) || is_notanumber(val))
				{
					DiceErrCode = is_infinite(val) ? DICE_ERR_OVERUNDERFLOW : DICE_ERR_UNDEFINED;
					return 0;
				}
				num_stack.push(val);
			}
			else if (is_operator(token[0]) && token.size() == 1)
			{
				if (num_stack.empty() || num_stack.size() < 2)
				{
					DiceErrCode = DICE_ERR_STACK;
					DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_NOTENOUGHOPERATORARGS);
					return 0;
				}
				double val2 = num_stack.top();
				num_stack.pop();
				double val1 = num_stack.top();
				num_stack.pop();
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
							return 0;
						}
						val = val1 / val2;
						break;
					case '%':
						if (!val2)
						{
							DiceErrCode = DICE_ERR_DIV0;
							return 0;
						}
						val = fmod(val1, val2);
						break;
					case '^':
						if (val1 < 0 && static_cast<double>(static_cast<int>(val2)) != val2)
						{
							DiceErrCode = DICE_ERR_UNDEFINED;
							return 0;
						}
						if (!val1 && !val2)
						{
							DiceErrCode = DICE_ERR_DIV0;
							return 0;
						}
						if (!val1 && val2 < 0)
						{
							DiceErrCode = DICE_ERR_OVERUNDERFLOW;
							return 0;
						}
						val = pow(val1, val2);
						break;
					case 'd':
						if (val1 < 1 || val1 > DICE_MAX_DICE)
						{
							DiceErrCode = DICE_ERR_UNACCEPT_DICE;
							DiceErrNum = static_cast<int>(val1);
							return 0;
						}
						if (val2 < 1 || val2 > DICE_MAX_SIDES)
						{
							DiceErrCode = DICE_ERR_UNACCEPT_SIDES;
							DiceErrNum = static_cast<int>(val2);
							return 0;
						}
						val = static_cast<double>(Dice(static_cast<int>(val1), static_cast<unsigned>(val2)));
				}
				if (is_infinite(val) || is_notanumber(val))
				{
					DiceErrCode = is_infinite(val) ? DICE_ERR_OVERUNDERFLOW : DICE_ERR_UNDEFINED;
					return 0;
				}
				num_stack.push(val);
			}
		}
		else
		{
			const PostfixValueDouble *dbl = dynamic_cast<const PostfixValueDouble *>(postfix[x].first);
			const double *val_ptr = dbl->Get();
			if (!val_ptr)
			{
				DiceErrCode = DICE_ERR_STACK;
				DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_EMPTYNUMBER);
				return 0;
			}
			num_stack.push(*val_ptr);
		}
	}
	val = num_stack.top();
	num_stack.pop();
	if (!num_stack.empty())
	{
		DiceErrCode = DICE_ERR_STACK;
		DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_TOOMANYNUMBERS);
		return 0;
	}
	return val;
}

/** Parse an infix notation expression and convert the expression to postfix notation.
 * @param infix The original expression, in infix notation, to convert to postfix notation
 * @return A postfix notation expression equivilent to the infix notation expression given, or an empty string if the infix notation expression could not be parsed or converted
 */
static Postfix DoParse(const ci::string &infix)
{
	Infix infixcpy = fix_infix(infix);
	Postfix postfix;
	if (infixcpy.str.empty())
		return postfix;
	if (!check_infix(infixcpy))
		return postfix;
	Infix tokenized_infix = tokenize_infix(infixcpy);
	if (tokenized_infix.str.empty())
		return postfix;
	postfix = infix_to_postfix(tokenized_infix);
	return postfix;
}

/** Evaluate a postfix notation expression.
 * @param postfix The postfix notation expression to evaluate
 * @return The final result after evaluation
 */
static double DoEvaluate(const Postfix &postfix)
{
	DiceErrCode = DICE_ERR_NONE;
	DiceErrPos = 0;
	DiceExOutput.clear();
	DiceShortExOutput.clear();
	double ret = eval_postfix(postfix);
	if (ret > INT_MAX || ret < INT_MIN)
		DiceErrCode = DICE_ERR_OVERUNDERFLOW;
	return ret;
}

/** DiceServ's error handler, if any step along the way fails, this will display the cause of the error to the user.
 * @param u The user that invoked DiceServ
 * @param dice The dice expression that was used to invoke DiceServ
 * @param ErrorPos The position of the error (usually the same DiceErrPos, but not always)
 * @return true in almost every case to signify there was an error, false if an unknown error code was passed in or no error code was set
 */
static bool ErrorHandler(User *u, const ci::string &dice, unsigned ErrorPos)
{
	bool WasError = true;
	switch (DiceErrCode)
	{
		case DICE_ERR_PARSE:
		{
			me->NoticeLang(s_DiceServ, u, DICE_PARSE_ERROR);
			u->SendMessage(s_DiceServ, " %s", dice.c_str());
			std::string spaces(ErrorPos > dice.size() ? dice.size() : ErrorPos, ' ');
			u->SendMessage(s_DiceServ, "(%s^)", spaces.c_str());
			me->NoticeLang(s_DiceServ, u, DICE_ERRSTR);
			u->SendMessage(s_DiceServ, "%s", DiceErrStr.c_str());
			break;
		}
		case DICE_ERR_DIV0:
			me->NoticeLang(s_DiceServ, u, DICE_ERROR_DIVBYZERO);
			u->SendMessage(s_DiceServ, " %s", dice.c_str());
			break;
		case DICE_ERR_UNDEFINED:
			me->NoticeLang(s_DiceServ, u, DICE_ERROR_UNDEFINED);
			u->SendMessage(s_DiceServ, " %s", dice.c_str());
			break;
		case DICE_ERR_UNACCEPT_DICE:
			if (DiceErrNum <= 0)
				me->NoticeLang(s_DiceServ, u, DICE_DICEUNDERLIMIT, DiceErrNum, DICE_MAX_DICE);
			else
				me->NoticeLang(s_DiceServ, u, DICE_DICEOVERLIMIT, DiceErrNum, DICE_MAX_DICE);
			break;
		case DICE_ERR_UNACCEPT_SIDES:
			if (DiceErrNum <= 0)
				me->NoticeLang(s_DiceServ, u, DICE_SIDESUNDERLIMIT, DiceErrNum, DICE_MAX_SIDES);
			else
				me->NoticeLang(s_DiceServ, u, DICE_SIDESOVERLIMIT, DiceErrNum, DICE_MAX_SIDES);
			break;
		case DICE_ERR_OVERUNDERFLOW:
			me->NoticeLang(s_DiceServ, u, DICE_OVERUNDERFLOW);
			u->SendMessage(s_DiceServ, " %s", dice.c_str());
			break;
		case DICE_ERR_STACK:
			me->NoticeLang(s_DiceServ, u, DICE_STACK_ERR);
			u->SendMessage(s_DiceServ, " %s", dice.c_str());
			me->NoticeLang(s_DiceServ, u, DICE_ERRSTR);
			u->SendMessage(s_DiceServ, "%s", DiceErrStr.c_str());
			break;
		default:
			WasError = false;
	}
	return WasError;
}

/** Find the lowest result out of the 4 6-sided dice thrown for a DnD3e character.
 * @param themin Reference to an integer storing the lowest result
 * @return The index in DnDresult of the lowest result
 */
static unsigned GetMinDnD(unsigned &min)
{
	min = DnDresults[0];
	unsigned minpos = 0;
	for (unsigned x = 1; x < 4; ++x)
		if (DnDresults[x] < min)
		{
			minpos = x;
			min = DnDresults[x];
		}
	return minpos;
}

/** Surround the lowest value with reverse control codes in the extended roll string for DnD3e character creation, and subtract the lowest value from the total score.
 * @param v Reference to the original score
 */
static void DnDRollCorrect(double &v)
{
	unsigned min, minpos = GetMinDnD(min);
	bool inex = false;
	std::string tmpstr;
	for (unsigned x = 0, y = 0, len = DiceExOutput.size(); x < len; ++x)
	{
		if (x && DiceExOutput[x - 1] == '(')
			inex = true;
		char curr = DiceExOutput[x];
		if (inex && y == minpos && curr != ' ' && curr != ')')
			tmpstr += static_cast<char>(22);
		tmpstr += curr;
		if (inex && y == minpos && curr != ' ' && curr != ')')
			tmpstr += static_cast<char>(22);
		if (inex && curr == ' ')
			++y;
		if (curr == ')')
			inex = false;
	}
	v -= min;
	DiceExOutput = tmpstr;
}

/** Calculate the sum of all the modifier values of all the rolls for DnD3e character creation.
 * @return The sum of the modifiers
 */
static int DnDmodadd()
{
	int mods = 0;
	for (unsigned x = 0; x < 6; ++x)
		mods += DnDmods[x][1];
	return mods;
}

/** Determine the highest roll of all the rolls for DnD3e character creation.
 * @return The highest value
 */
static unsigned DnDmaxatt()
{
	int maxatt = DnDmods[0][0];
	for (unsigned x = 1; x < 6; ++x)
		if (maxatt < DnDmods[x][0])
			maxatt = DnDmods[x][0];
	return maxatt;
}

/** Get the syntax string for the given dice type.
 * @param type The dice type
 * @return The syntax string for the type
 */
static int syntax_string(DICE_TYPES type)
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
static void diceserv_roller(User *u, const ci::string &origdice, const ci::string &chan, const ci::string &comment, DICE_TYPES type, bool round_result = true)
{
	/* Reset variables, and set DiceEx depending on if we are requesting an extended dice roll or not */
	DiceEx = type == DICE_TYPE_ROLL || type == DICE_TYPE_CALC ? 0 : 1;
	DiceErrCode = DICE_ERR_NONE;
	DiceErrPos = DnDPos = 0;
	DiceErrNum = 0;
	DoDnD3eChar = DoEarthdawn = false;
	DiceExOutput.clear();
	DiceShortExOutput.clear();
	CurrUser = u;
	/* Check for a ignore on the user or their registered nick, if any, and deny them access if they are ignored */
	if (u->GetExt("diceserv_ignore"))
		return;
	if (u->nc && u->nc->GetExt("diceserv_ignore"))
		return;
	/* Check if there was even a dice argument given for all rolls except for the DnD3e character creation roll */
	if (type != DICE_TYPE_DND3E && origdice.empty())
	{
		if (!SourceIsBot)
			me->NoticeLang(s_DiceServ, u, syntax_string(type));
		return;
	}
	/* Make copies of the passed arguments */
	ci::string origdicecpy = origdice, chancpy = chan, commentcpy = comment;
	/* Always yse 6~4d6 (6 sets of 4d6) for a DnD3e character creation roll */
	if (type == DICE_TYPE_DND3E)
	{
		origdicecpy = "6~4d6";
		DoDnD3eChar = true;
	}
	/* Set if an Earthdawn roll is requested */
	else if (type == DICE_TYPE_EARTHDAWN)
		DoEarthdawn = true;
	/* If the channel doesn't start with #, we'll treat it as if it was part of the comment */
	bool	chaniscomm = false, /* true if the channel should actually be part of the comment, false if it should be treated as a channel name */
			inchan = true; /* true if the result is to go to a channel, false otherwise */
	if (!chancpy.empty() && chancpy[0] != '#')
	{
		chaniscomm = true;
		inchan = false;
	}
	else if (chancpy.empty())
		inchan = false;
	/* If a channel was given, ignore the roll if the user isn't in the channel.
	 * Also, check if the channel has ignored rolls to it or if it's been moderated (+m) and the user has no status to the channel. */
	if (inchan)
	{
		Channel *c = findchan(chancpy.c_str());
		if (c)
		{
			if (!is_on_chan(c, u))
			{
				if (!SourceIsBot)
				{
					notice_lang(s_DiceServ, u, CHAN_X_INVALID, chancpy.c_str());
					me->NoticeLang(s_DiceServ, u, syntax_string(type));
				}
				return;
			}
			ChannelInfo *ci = cs_findchan(chancpy.c_str());
			int found_c_ignore = find_chan_ignore(c->name);
			bool found_ci_ignore = ci ? ci->GetExt("dicesev_ignore") : false;
			std::string modes = chan_get_modes(c, 0, 1);
			bool CMODE_m = modes.find('m') != std::string::npos;
			if (found_c_ignore != -1 || found_ci_ignore || (CMODE_m && !chan_get_user_status(c, u)))
			{
				inchan = false;
				chancpy.clear();
			}
			if (!inchan && SourceIsBot)
				return;
		}
		else
		{
			inchan = false;
			chancpy.clear();
		}
	}
	/* Generate the comment string */
	ci::string commentstr; /* Full comment, constructed from given arguments */
	if (!chancpy.empty() && !inchan)
		commentstr += chancpy + " ";
	if (!commentcpy.empty())
		commentstr += commentcpy;
	/* Calculate max message length */
	int MaxMessageLength = 510;
	BotInfo *bi = findbot(inchan && SourceIsBot ? RollSource.c_str() : s_DiceServ);
	MaxMessageLength -= strlen(bi->nick);
	MaxMessageLength -= strlen(bi->user);
	MaxMessageLength -= strlen(bi->host);
	MaxMessageLength -= 7; /* For the :, !, and @, plus the space after that and after the PRIVMSG/NOTICE and after the target and the : for the message */
	MaxMessageLength -= inchan && SourceIsBot ? 7 : 6; /* inchan with BotServ bot == PRIVMSG, otherwise NOTICE */
	MaxMessageLength -= inchan ? chancpy.size() : strlen(u->nick); /* inchan uses channel's name, otherwise uses user's nick */
	MaxMessageLength -= 7; /* For the < > [ ] :, and the space before the [ and after the : */
	MaxMessageLength -= origdicecpy.size(); /* The dice expression itself */
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
	if (!commentstr.empty())
		MaxMessageLength -= commentstr.size() + 1; /* Comment plus space */
	/* Check for overflow prior to adding in dice results */
	if (MaxMessageLength <= 0)
	{
		me->NoticeLang(s_DiceServ, u, DICE_BUFFER_OVERFLOW);
		return;
	}
	/* End calculating max message length */
	std::string	sep = " ", /* Separator between extended output sets, might change in the future if we need a different seperator */
				buf, /* Buffer to store output */
				extmp, /* Temporary storage for extended result buffer */
				shortextmp; /* Temporary storage for shorter extended result buffer */
	ci::string dice; /* Dice buffer */
	bool error = false; /* true when there really is an error, false if there is no error */
	unsigned step; /* Used with Earthdawn, the step that the user requested the roll for */
	double v; /* Stores the result of the dice expression */
	/* When a ~ is in the dice expression, the expression before the ~ will be parsed to determine the number of sets to throw, the actual set will be after the ~ */
	size_t tilde = origdicecpy.find('~');
	int oldn, n; /* Temporary counter for number of sets to roll */
	if (tilde != ci::string::npos)
	{
		/* Earthdawn rolls do not support using ~ syntax */
		if (type == DICE_TYPE_EARTHDAWN)
		{
			me->NoticeLang(s_DiceServ, u, DICE_ERROR_EARTHDAWN_INPUT);
			return;
		}
		/* Extract the expression before the ~ into times, and place the expression after the ~ into dice, then parse the expression in times */
		dice = origdicecpy.substr(tilde + 1);
		ci::string times = origdicecpy.substr(0, tilde);
		if (times.empty() || dice.empty())
		{
			DiceErrCode = DICE_ERR_PARSE;
			DiceErrStr = me->GetLangString(CurrUser, DICE_ERROR_EMPTYNUMBER);
			ErrorHandler(u, origdicecpy, tilde);
			return;
		}
		Postfix times_postfix = DoParse(times);
		/* If the parsing failed, display the error and leave */
		if (times_postfix.empty())
		{
			ErrorHandler(u, origdicecpy, DiceErrPos);
			return;
		}
		/* Evaulate the expression */
		v = DoEvaluate(times_postfix);
		cleanup_postfix(times_postfix);
		/* Check if the evaluated number of times is out of bounds */
		if (DiceErrCode == DICE_ERR_NONE)
		{
			oldn = n = static_cast<int>(v);
			if (n > DICE_MAX_TIMES)
			{
				me->NoticeLang(s_DiceServ, u, DICE_TIMESOVERLIMIT, n, DICE_MAX_TIMES);
				return;
			}
			else if (n <= 0)
			{
				me->NoticeLang(s_DiceServ, u, DICE_TIMESUNDERLIMIT, n, DICE_MAX_TIMES);
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
			ci::string step_str;
			size_t plus = origdicecpy.find('+');
			if (plus != ci::string::npos)
			{
				step_str = origdicecpy.substr(0, plus);
				dice = origdicecpy.substr(plus);
			}
			else
				step_str = origdicecpy;
			int num = atoi(step_str.c_str());
			std::string tmp = dtoa(num);
			if (step_str != tmp)
			{
				me->NoticeLang(s_DiceServ, u, DICE_ERROR_EARTHDAWN_INPUT);
				return;
			}
			if (num < 1 || num > 100)
			{
				me->NoticeLang(s_DiceServ, u, DICE_ERROR_EARTHDAWN_RANGE, num);
				return;
			}
			step = num;
			if (!dice.empty())
				origdicecpy = ci::string("(") + EarthdawnStepTable[step] + ")" + dice;
			else
				origdicecpy = EarthdawnStepTable[step];
		}
		dice = origdicecpy;
		oldn = n = 1;
		tilde = 0;
	}
	/* As long as there was no error, either roll the dice and stop, or if doing a DnD3e character, continue rolling dice as
	 * long as we get a set of 6 scores that have a max score of 13 or less, or have the sum of their modifiers be 0 or less */
	if (!error)
	{
		/* Parse the dice */
		Postfix dice_postfix = DoParse(dice);
		/* If the parsing failed, display the error and leave */
		if (dice_postfix.empty())
		{
			ErrorHandler(u, origdicecpy, (tilde ? tilde + 1 : 0) + DiceErrPos);
			return;
		}
		bool DnDcontinue; /* true when the dice need to be rerolled for a DnD3e character, if the sum of their modifiers was 0 or less, or if their highest score was a 13 or less */
		unsigned DnDmodpos; /* Current position within the DnDmods array */
		do
		{
			/* Reset DnD3e values regardless, and set n to the original number of sets */
			DnDcontinue = false;
			DnDmodpos = 0;
			n = oldn;
			/* Roll as many sets as were requested, also resetting the DnD3e position after each roll */
			for (; n > 0; --n, DnDPos = 0)
			{
				/* Evaluate the dice, then check for errors */
				v = DoEvaluate(dice_postfix);
				error = ErrorHandler(u, origdicecpy, (tilde ? tilde + 1 : 0) + DiceErrPos);
				/* As long as we didn't have an error, we will continue */
				if (!error)
				{
					/* If this is not the first set, add the seperator to the buffer */
					if (n != oldn)
						buf += sep;
					/* Process extended output if we are still doing so */
					if (DiceEx == 1)
					{
						/* Make sure that we have some extended output first */
						if (!DiceExOutput.empty())
						{
							/* Add a seperator on the extended output */
							if (n == oldn)
							{
								shortextmp = "{";
								extmp = "{";
							}
							else
							{
								shortextmp += " | ";
								extmp += " | ";
							}
							/* Remove the extra space at the end */
							DiceExOutput.erase(DiceExOutput.end() - 1);
							DiceShortExOutput.erase(DiceShortExOutput.end() - 1);
							/* If a DnD3e character roll was requested, correct the result and calculate the modifier */
							if (type == DICE_TYPE_DND3E)
							{
								DnDRollCorrect(v);
								DnDmods[DnDmodpos][0] = static_cast<int>(my_round(v));
								DnDmods[DnDmodpos++][1] = static_cast<int>(floor((v - 10) / 2));
							}
							/* Add the extended output string to the extended result buffer */
							extmp += DiceExOutput;
							shortextmp += DiceShortExOutput;
							/* Add a closing seperator on the last roll */
							if (n == 1)
							{
								shortextmp += "} ";
								extmp += "} ";
							}
						}
						/* Without extended output, we treat this as if there was no extended output requested */
						else
							DiceEx = 0;
					}
					/* Round the result and add it the buffer */
					std::string tmp = dtoa(round_result ? static_cast<int>(my_round(v)) : v);
					buf += tmp;
				}
				/* Leave if there was an error */
				else
					return;
			}
			/* If a DnD3e character roll was requested, and the sum of the modifiers is 0 or less, or the maximum score is 13 or less,
			 * we reset things and redo the rolls */
			if (type == DICE_TYPE_DND3E && (DnDmodadd() <= 0 || DnDmaxatt() <= 13))
			{
				me->NoticeLang(s_DiceServ, u, DnDmodadd() <= 0 ? DICE_ERROR_DND3E_MODTOTALZERO : DICE_ERROR_DND3E_MAXSCORETHIRTEEN);
				DnDcontinue = true;
				buf.clear();
				DiceExOutput.clear();
				DiceShortExOutput.clear();
				extmp.clear();
				shortextmp.clear();
			}
		} while (DnDcontinue);
		cleanup_postfix(dice_postfix);
	}
	/* Determine if we have any sort of overflow with the extended roll buffer or the full buffer */
	if (MaxMessageLength - static_cast<int>(buf.size()) >= 0)
	{
		if (DiceEx == 1 && MaxMessageLength - static_cast<int>(buf.size()) - static_cast<int>(extmp.size()) < 0)
			DiceEx = 2;
		if (DiceEx == 2 && MaxMessageLength - static_cast<int>(buf.size()) - static_cast<int>(shortextmp.size()) < 0)
			DiceEx = 3;
		if (DiceEx == 3)
			me->NoticeLang(s_DiceServ, u, DICE_EX_OVERFLOW, type == DICE_TYPE_EXROLL ? "EXROLL" : (type == DICE_TYPE_EXCALC ? "EXCALC" : "DND3ECHAR"));
	}
	else
	{
		me->NoticeLang(s_DiceServ, u, DICE_BUFFER_OVERFLOW);
		return;
	}
	/* If we got this far, there should be no errors, but just making sure :P */
	if (DiceErrCode == DICE_ERR_NONE && !error)
	{
		/* Temporarily store which extended buffer, if any, to use */
		std::string exbuffer = DiceEx == 1 ? extmp : (DiceEx == 2 ? shortextmp : "");
		/* If the result is expected to go to a channel, send a notice to the channel unless DiceServ was triggered through
		 * BotServ's fantasy, then send a privmsg instead */
		if (inchan)
		{
			if (type == DICE_TYPE_EARTHDAWN)
			{
				if (SourceIsBot)
					ircdproto->SendPrivmsg(findbot(RollSource.c_str()), chancpy.c_str(), "<Earthdawn roll for %s [Step %u (%s)]: %s%s>%s%s", u->nick, step, origdicecpy.c_str(), exbuffer.c_str(), buf.c_str(),
						!commentstr.empty() ? " " : "", !commentstr.empty() ? commentstr.c_str() : "");
				else
					ircdproto->SendNotice(DiceServ, chancpy.c_str(), "<Earthdawn roll for %s [Step %u (%s)]: %s%s>%s%s", u->nick, step, origdicecpy.c_str(), exbuffer.c_str(), buf.c_str(), !commentstr.empty() ? " " : "",
						!commentstr.empty() ? commentstr.c_str() : "");
			}
			else
			{
				if (SourceIsBot)
					ircdproto->SendPrivmsg(findbot(RollSource.c_str()), chancpy.c_str(), "<%s for %s [%s]: %s%s>%s%s",
						type == DICE_TYPE_ROLL ? "Roll" : (type == DICE_TYPE_CALC ? "Calc" : (type == DICE_TYPE_EXROLL ? "Exroll" : (type == DICE_TYPE_EXCALC ? "Excalc" : "DnD3e Character Roll"))), u->nick,
						origdicecpy.c_str(), exbuffer.c_str(), buf.c_str(), !commentstr.empty() ? " " : "", !commentstr.empty() ? commentstr.c_str() : "");
				else
					ircdproto->SendNotice(DiceServ, chancpy.c_str(), "<%s for %s [%s]: %s%s>%s%s",
						type == DICE_TYPE_ROLL ? "Roll" : (type == DICE_TYPE_CALC ? "Calc" : (type == DICE_TYPE_EXROLL ? "Exroll" : (type == DICE_TYPE_EXCALC ? "Excalc" : "DnD3e Character Roll"))), u->nick,
						origdicecpy.c_str(), exbuffer.c_str(), buf.c_str(), !commentstr.empty() ? " " : "", !commentstr.empty() ? commentstr.c_str() : "");
			}
		}
		/* Otherwise, the result is meant to go only to the user requesting the roll */
		else
		{
			if (type == DICE_TYPE_DND3E)
				me->NoticeLang(s_DiceServ, u, DICE_DND3ECHAR_USER, origdicecpy.c_str(), exbuffer.c_str(), buf.c_str(), !commentstr.empty() ? " " : "", !commentstr.empty() ? commentstr.c_str() : "");
			else if (type == DICE_TYPE_EARTHDAWN)
				me->NoticeLang(s_DiceServ, u, DICE_EARTHDAWN_USER, step, origdicecpy.c_str(), exbuffer.c_str(), buf.c_str(), !commentstr.empty() ? " " : "", !commentstr.empty() ? commentstr.c_str() : "");
			else
				me->NoticeLang(s_DiceServ, u, DiceEx == 1 || DiceEx == 2 ? (type == DICE_TYPE_EXCALC ? DICE_EXCALC_USER : DICE_EXROLL_USER) : (type == DICE_TYPE_CALC ? DICE_CALC_USER : DICE_ROLL_USER), origdice.c_str(),
					exbuffer.c_str(), buf.c_str(), !commentstr.empty() ? " " : "", !commentstr.empty() ? commentstr.c_str() : "");
		}
	}
}

/** Load DiceServ's configuration.
 *
 * Only directive is DiceServDBName, defaults to diceserv.db if not given.
 */
static void do_load_config()
{
	ConfigReader config;
	DiceServDBName = config.ReadValue("diceserv", "database", "diceserv.db", 0);
	s_DiceServ = sstrdup(config.ReadValue("diceserv", "name", "DiceServ", 0).c_str());
	DiceServChanOpCanIgnore = config.ReadFlag("diceserv" "chanopcanignore", "no", 0);
}

/** HELP command
 */
class CommandDiceServHelp : public Command
{
 public:
	CommandDiceServHelp() : Command("HELP", 1, 1)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string cmd = params[0];

		if (cmd == "FUNCTIONS")
			me->NoticeLang(s_DiceServ, u, DICE_HELP_FUNCTIONS, s_DiceServ);
		else
			mod_help_cmd(s_DiceServ, u, DiceServ_cmdTable, cmd.c_str());

		return MOD_CONT;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, DICE_HELP, s_DiceServ, s_DiceServ, s_DiceServ);
		for (int i = DICE_HELP_CMD_ROLL; i < DICE_HELP; ++i)
			me->NoticeLang(s_DiceServ, u, i, s_DiceServ);
		me->NoticeLang(s_DiceServ, u, DICE_HELP_FOOTER, s_DiceServ, s_BotServ ? s_BotServ : "BotServ", s_BotServ ? s_BotServ : "BotServ", s_DiceServ, s_DiceServ);
	}
};

/** ROLL command
 *
 * Handles regular dice rolls.
 */
class CommandDiceServRoll : public Command
{
 public:
	CommandDiceServRoll() : Command("ROLL", 1, 3)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string origdice = params[0], chan = params.size() > 1 ? params[1] : "", comment = params.size() > 2 ? params[2] : "";
		SourceIsBot = false;
		diceserv_roller(u, origdice, chan, comment, DICE_TYPE_ROLL);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &subcommand)
	{
		if (subcommand.empty())
			me->NoticeLang(s_DiceServ, u, DICE_HELP_ROLL, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ);
		else if (subcommand == "EXPRESSIONS")
			me->NoticeLang(s_DiceServ, u, DICE_HELP_ROLL_EXPRESSIONS, s_DiceServ);
		return true;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, DICE_ROLL_SYNTAX);
	}
};

/** CALC command
 *
 * Handles regular dice rolls, sans rounding, resulting in more of a calcualtion.
 */
class CommandDiceServCalc : public Command
{
 public:
	CommandDiceServCalc() : Command("CALC", 1, 3)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string origdice = params[0], chan = params.size() > 1 ? params[1] : "", comment = params.size() > 2 ? params[2] : "";
		SourceIsBot = false;
		diceserv_roller(u, origdice, chan, comment, DICE_TYPE_CALC, false);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &)
	{
		me->NoticeLang(s_DiceServ, u, DICE_HELP_CALC, s_DiceServ);
		return true;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, DICE_CALC_SYNTAX);
	}
};

/** EXROLL command
 *
 * Handles dice rolls with extended output.
 */
class CommandDiceServExRoll : public Command
{
 public:
	CommandDiceServExRoll() : Command("EXROLL", 1, 3)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string origdice = params[0], chan = params.size() > 1 ? params[1] : "", comment = params.size() > 2 ? params[2] : "";
		SourceIsBot = false;
		diceserv_roller(u, origdice, chan, comment, origdice.empty() || origdice == "%" || origdice.find('d') != ci::string::npos || origdice.find("rand(") != ci::string::npos ? DICE_TYPE_EXROLL : DICE_TYPE_ROLL);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &)
	{
		me->NoticeLang(s_DiceServ, u, DICE_HELP_EXROLL, s_DiceServ);
		return true;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, DICE_EXROLL_SYNTAX);
	}
};

/** EXCALC command
 *
 * Handles dice rolls with extended output, sans rounding, resulting in more of a calculation.
 */
class CommandDiceServExCalc : public Command
{
 public:
	CommandDiceServExCalc() : Command("EXCALC", 1, 3)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string origdice = params[0], chan = params.size() > 1 ? params[1] : "", comment = params.size() > 2 ? params[2] : "";
		SourceIsBot = false;
		diceserv_roller(u, origdice, chan, comment, origdice.empty() || origdice == "%" || origdice.find('d') != ci::string::npos || origdice.find("rand(") != ci::string::npos ? DICE_TYPE_EXCALC : DICE_TYPE_CALC, false);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &)
	{
		me->NoticeLang(s_DiceServ, u, DICE_HELP_EXCALC, s_DiceServ);
		return true;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, DICE_EXCALC_SYNTAX);
	}
};

/** DND3ECHAR command
 *
 * Handles the dice rolls that make up character creation in DnD3e.
 */
class CommandDiceServDnD3eChar : public Command
{
 public:
	CommandDiceServDnD3eChar() : Command("DND3ECHAR", 0, 2)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string chan = params.size() ? params[0] : "", comment = params.size() > 1 ? params[1] : "";
		SourceIsBot = false;
		diceserv_roller(u, "", chan, comment, DICE_TYPE_DND3E);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &)
	{
		me->NoticeLang(s_DiceServ, u, DICE_HELP_DND3ECHAR, s_DiceServ, s_DiceServ);
		return true;
	}
};

/** EARTHDAWN command
 *
 * Handles dice rolls for the pen & paper RPG Earthdawn.
 */
class CommandDiceServEarthdawn : public Command
{
 public:
	CommandDiceServEarthdawn() : Command("EARTHDAWN", 1, 3)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string origdice = params[0], chan = params.size() > 1 ? params[1] : "", comment = params.size() > 2 ? params[2] : "";
		SourceIsBot = false;
		diceserv_roller(u, origdice, chan, comment, DICE_TYPE_EARTHDAWN);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &)
	{
		me->NoticeLang(s_DiceServ, u, DICE_HELP_EARTHDAWN, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ, s_DiceServ);
		return true;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, DICE_EARTHDAWN_SYNTAX);
	}
};

/** SET command
 *
 * Currently only has a IGNORE sub-command, which can set a channel or nickname/user to be ignored by DiceServ.
 */
class CommandDiceServSet : public Command
{
 private:
	CommandReturn DoIgnore(User *u, std::vector<ci::string> &params)
	{
		const char *where = params[1].c_str();
		ci::string param = params[2];
		/* The parameter must be ON or OFF, all others should stop processing */
		enum
		{
			IGNORE_ADD,
			IGNORE_DEL
		} mode;
		if (param == "ON")
			mode = IGNORE_ADD;
		else if (param == "OFF")
			mode = IGNORE_DEL;
		else
		{
			this->OnSyntaxError(u);
			return MOD_CONT;
		}
		bool is_servoper = u->nc && u->nc->HasCommand("diceserv/set");
		/* If the thing to ignore starts with a #, assume it's a channel */
		if (*where == '#')
		{
			/* Find the Channel record and ChannelInfo record from ChanServ */
			Channel *c = findchan(where);
			ChannelInfo *ci = cs_findchan(where);
			/* If the channel wasn't found and a ChanServ entry wasn't found (or the channel is forbidden or suspended), display an error */
			if (!c && (!ci || (ci && ((ci->flags & CI_FORBIDDEN) || (ci->flags & CI_SUSPENDED)))))
				notice_lang(s_DiceServ, u, CHAN_X_INVALID, where);
			/* If we found a registered channel, we will store the ignore there */
			else if (ci)
			{
				/* The only ones who can set an ignore on a registered channel are Services Admins or the channel's founder (unless DiceServChanOpCanIgnore is set, then channel operators can as well) */
				bool isOp = check_access(u, ci, CA_AUTOOP);
				if (is_servoper || (DiceServChanOpCanIgnore ? isOp : false) || ((ci->flags & CI_SECUREFOUNDER) ? is_real_founder(u, ci) : is_founder(u, ci)))
				{
					/* Either add or delete the ignore data */
					if (mode == IGNORE_ADD)
					{
						ci->Extend("diceserv_ignore", new bool(true));
						if (c)
							add_chan_ignore(c->name);
					}
					else
					{
						bool *diceserv_ignore;
						if (ci->GetExt("diceserv_ignore", diceserv_ignore))
						{
							delete diceserv_ignore;
							ci->Shrink("diceserv_ignore");
						}
						if (c)
							del_chan_ignore(c->name);
					}
					me->NoticeLang(s_DiceServ, u, mode == IGNORE_ADD ? DICE_SET_IGNORE_CHAN_ON : DICE_SET_IGNORE_CHAN_OFF, s_DiceServ, where);
				}
				/* Otherwise, deny access */
				else
					notice_lang(s_DiceServ, u, ACCESS_DENIED);
			}
			/* No registered channel was found, but a channel was, so store temporary data on the channel */
			else
			{
				/* The only ones who can set an ignore on an unregistered channel are Services Operators with diceserv/set or channel operators */
				if (is_servoper || chan_has_user_status(c, u, CUS_OP))
				{
					/* Either add or delete the ignore data */
					if (mode == IGNORE_ADD)
						add_chan_ignore(c->name);
					else
						del_chan_ignore(c->name);
					me->NoticeLang(s_DiceServ, u, mode == IGNORE_ADD ? DICE_SET_IGNORE_CHAN_ON : DICE_SET_IGNORE_CHAN_OFF, s_DiceServ, where);
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
			/* Only Services Operators with diceserv/set can set ignores on nicks, deny access to those who aren't */
			if (!is_servoper)
				notice_lang(s_DiceServ, u, ACCESS_DENIED);
			/* If the nick wasn't found and a NickServ entry wasn't found (or the nick is forbidden or suspended), display an error */
			else if (!nu && (!na || (na && ((na->status & NS_FORBIDDEN) || (na->nc && (na->nc->flags & NI_SUSPENDED))))))
				me->NoticeLang(s_DiceServ, u, DICE_INVALID_NICK, where);
			/* If we found a registered nick, we will store the ignore there */
			else if (na)
			{
				/* If a User record was not found, we will check all the users to find one that has a matching NickCore */
				if (!nu)
					for (nu = firstuser(); nu; nu = nextuser())
						if (nu->nc == na->nc)
							break;
				/* Either add or delete the ignore data */
				if (mode == IGNORE_ADD)
				{
					na->nc->Extend("diceserv_ignore", new bool(true));
					if (nu)
						nu->Extend("diceserv_ignore", new bool(true));
				}
				else
				{
					bool *diceserv_ignore;
					if (na->nc->GetExt("diceserv_ignore", diceserv_ignore))
					{
						delete diceserv_ignore;
						na->nc->Shrink("diceserv_ignore");
					}
					if (nu && nu->GetExt("diceserv_ignore", diceserv_ignore))
					{
						delete diceserv_ignore;
						nu->Shrink("diceserv_ignore");
					}
				}
				me->NoticeLang(s_DiceServ, u, mode == IGNORE_ADD ? DICE_SET_IGNORE_NICK_ON : DICE_SET_IGNORE_NICK_OFF, s_DiceServ, where);
			}
			/* No registered nick was found, but a user was, so store temporary data on the user */
			else
			{
				/* Either add or delete the ignore data */
				if (mode == IGNORE_ADD)
					nu->Extend("diceserv_ignore", new bool(true));
				else
				{
					bool *diceserv_ignore;
					if (nu->GetExt("diceserv_ignore", diceserv_ignore))
					{
						delete diceserv_ignore;
						nu->Shrink("diceserv_ignore");
					}
				}
				me->NoticeLang(s_DiceServ, u, mode == IGNORE_ADD ? DICE_SET_IGNORE_NICK_ON : DICE_SET_IGNORE_NICK_OFF, s_DiceServ, where);
			}
		}
		return MOD_CONT;
	}
 public:
	CommandDiceServSet() : Command("SET", 3, 3)
	{
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		/* Don't allow this command to run if the databases are in read-only mode */
		if (readonly)
		{
			me->NoticeLang(s_DiceServ, u, DICE_SET_DISABLED);
			return MOD_CONT;
		}
		ci::string cmd = params[0];
		/* SET IGNORE sub-command */
		if (cmd == "IGNORE")
			return this->DoIgnore(u, params);
		/* Unknown SET command given */
		else
		{
			me->NoticeLang(s_DiceServ, u, DICE_SET_UNKNOWN_OPTION, cmd.c_str());
			notice_lang(s_DiceServ, u, MORE_INFO, s_DiceServ, "SET");
		}
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &subcommand)
	{
		if (subcommand.empty())
			me->NoticeLang(s_DiceServ, u, DICE_HELP_SET, s_DiceServ, s_DiceServ);
		else if (subcommand == "IGNORE")
		{
			if (u->nc && u->nc->HasCommand("diceserv/set"))
				me->NoticeLang(s_DiceServ, u, DICE_SERVADMIN_HELP_SET_IGNORE, s_DiceServ, s_DiceServ, s_ChanServ, s_NickServ);
			else
				me->NoticeLang(s_DiceServ, u, DICE_HELP_SET_IGNORE, s_DiceServ, s_DiceServ, s_ChanServ);
		}
		return true;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, u->nc && u->nc->HasCommand("diceserv/set") ? DICE_SERVADMIN_SET_IGNORE_SYNTAX : DICE_SET_IGNORE_SYNTAX);
	}
};

/** STATUS command
 *
 * This will allow Services Operators to view the ignore status of a single channel or a single nickname/user.
 */
class CommandDiceServStatus : public Command
{
 public:
	CommandDiceServStatus() : Command("STATUS", 1, 1, "diceserv/status")
	{
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		const char *what = params[0].c_str();
		/* If the argument starts with a #, assume it's a channel */
		if (*what == '#')
		{
			/* Find the Channel record and ChannelInfo record from ChanServ */
			Channel *c = findchan(what);
			ChannelInfo *ci = cs_findchan(what);
			/* If the channel wasn't found and a ChanServ entry wasn't found (or the channel is forbidden or suspended), display an error */
			if (!c && (!ci || (ci && ((ci->flags & CI_FORBIDDEN) || (ci->flags & CI_SUSPENDED)))))
				notice_lang(s_DiceServ, u, CHAN_X_INVALID, what);
			/* If we found a registered channel, show the data for that */
			else if (ci)
				me->NoticeLang(s_DiceServ, u, DICE_STATUS_CHAN_REGGED, what, me->GetLangString(u, ci->GetExt("diceserv_ignore") ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW));
			/* No registered channel was found, but a channel was, so show the data for that */
			else
			{
				int chan_ignore = find_chan_ignore(c->name);
				me->NoticeLang(s_DiceServ, u, DICE_STATUS_CHAN, what, me->GetLangString(u, chan_ignore != -1 ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW));
			}
		}
		/* Otherwise, the argument is going to be assumed to be a nick */
		else
		{
			/* Find the User record and the NickAlias record from NickServ */
			User *nu = finduser(what);
			NickAlias *na = findnick(what);
			/* If the nick wasn't found and a NickServ entry wasn't found (or the nick is forbidden or suspended), display an error */
			if (!nu && (!na || (na && ((na->status & NS_FORBIDDEN) || (na->nc && (na->nc->flags & NI_SUSPENDED))))))
				me->NoticeLang(s_DiceServ, u, DICE_INVALID_NICK, what);
			/* If we found a registered nick, show the data for that */
			else if (na)
			{
				/* If a User record was not found, we will check all the users to find one that has a matching NickCore */
				if (!nu)
					for (nu = firstuser(); nu; nu = nextuser())
						if (nu->nc == na->nc)
							break;
				/* If we have a User record, then the given nick is online from another nick in their group, show that */
				if (nu && nu->nick != na->nick)
					me->NoticeLang(s_DiceServ, u, DICE_STATUS_NICK_ONLINE, what, me->GetLangString(u, na->nc->GetExt("diceserv_ignore") ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW), nu->nick);
				/* Otherwise, they are not online, just show that it is registered and it's data */
				else
					me->NoticeLang(s_DiceServ, u, DICE_STATUS_NICK_REGGED, what, me->GetLangString(u, na->nc->GetExt("diceserv_ignore") ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW));
			}
			/* No registered nick was found, but a user was, so show the data for that */
			else
				me->NoticeLang(s_DiceServ, u, DICE_STATUS_NICK, what, me->GetLangString(u, nu->GetExt("diceserv_ignore") ? DICE_STATUS_IGNORE : DICE_STATUS_ALLOW));
		}
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &)
	{
		me->NoticeLang(s_DiceServ, u, DICE_SERVADMIN_HELP_STATUS);
		return true;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, DICE_STATUS_SYNTAX);
	}
};

/** LIST command
 *
 * This will allow Services Operators to list all the nicknames/users or channels (either registered or not) matching a mask that are either ignored, allowed, or both.
 */
class CommandDiceServList : public Command
{
 public:
	CommandDiceServList() : Command("LIST", 3, 4, "diceserv/list")
	{
	}

	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string showtype = params[0], what = params[1];
		const char *pattern = params[2].c_str();
		/* Ignore type must be one of the following, otherwise we stop processing */
		enum
		{
			SHOW_IGNORED,
			SHOW_ALLOWED,
			SHOW_ALL
		} show;
		if (showtype == "IGNORE")
			show = SHOW_IGNORED;
		else if (showtype == "ALLOW")
			show = SHOW_ALLOWED;
		else if (showtype == "ALL")
			show = SHOW_ALL;
		else
		{
			this->OnSyntaxError(u);
			return MOD_CONT;
		}
		/* The type of entries to find must be one of the following, otherwise we stop processing */
		if (what != "CHANNELS" && what != "NICKS")
		{
			this->OnSyntaxError(u);
			return MOD_CONT;
		}
		enum
		{
			REG_SHOW_ALL,
			REG_SHOW_REG,
			REG_SHOW_UNREG
		} reg = REG_SHOW_ALL;
		/* If the optional regtype argument is given, it must be one of the following, otherwise we stop processing */
		if (params.size() > 3)
		{
			if (params[3] == "REG")
				reg = REG_SHOW_REG;
			if (params[3] == "UNREG")
				reg = REG_SHOW_UNREG;
			else
			{
				this->OnSyntaxError(u);
				return MOD_CONT;
			}
		}
		/* Show the header */
		me->NoticeLang(s_DiceServ, u, DICE_LIST_HEADER, me->GetLangString(u, show == SHOW_IGNORED ? DICE_LIST_HEADER_IGNORED : (show == SHOW_ALLOWED ? DICE_LIST_HEADER_ALLOWED : DICE_LIST_HEADER_ALL)),
			me->GetLangString(u, what == "CHANNELS" ? DICE_LIST_HEADER_CHANNELS : DICE_LIST_HEADER_NICKS), pattern,
			reg == REG_SHOW_ALL ? "" : me->GetLangString(u, reg == REG_SHOW_REG ? DICE_LIST_HEADER_REGONLY : DICE_LIST_HEADER_UNREGONLY));
		bool display;
		int shown = 0, i = 0;
		/* If we are to show channels, we do so */
		if (what == "CHANNELS")
		{
			/* If no regtype argument is given or we want to look only at unregistered channels, we process the channels list */
			if (reg == REG_SHOW_UNREG || reg == REG_SHOW_ALL)
			{
				Channel *c = firstchan();
				for (; c; c = nextchan())
				{
					/* Skip the channel if it's registered */
					if (c->ci)
						continue;
					display = false;
					/* We will only show the channel if it was part of the ignore type request */
					int diceserv_ignore = find_chan_ignore(c->name);
					if (diceserv_ignore && (show == SHOW_IGNORED || show == SHOW_ALL))
						display = true;
					else if (!diceserv_ignore && (show == SHOW_ALLOWED || show == SHOW_ALL))
						display = true;
					/* If we are to display the channel, make sure it matches the pattern first, and we haven't shown too many entries already */
					if (display && (!stricmp(pattern, c->name) || Anope::Match(c->name, pattern)) && ++shown <= 100)
					{
						if (reg == REG_SHOW_ALL)
							u->SendMessage(s_DiceServ, "   %-20s  %-5s  %s", c->name, me->GetLangString(u, DICE_LIST_UNREG), me->GetLangString(u, diceserv_ignore == -1 ? DICE_IGNORED : DICE_ALLOWED));
						else
							u->SendMessage(s_DiceServ, "   %-20s  %s", c->name, me->GetLangString(u, diceserv_ignore == -1 ? DICE_IGNORED : DICE_ALLOWED));
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
						if ((ci->flags & CI_FORBIDDEN) || (ci->flags & CI_SUSPENDED))
							continue;
						display = false;
						/* We will only show the channel if it was part of the ignore type request */
						bool diceserv_ignore = ci->GetExt("diceserv_ignore");
						if (diceserv_ignore && (show == SHOW_IGNORED || show == SHOW_ALL))
							display = true;
						else if (!diceserv_ignore && (show == SHOW_ALLOWED || show == SHOW_ALL))
							display = true;
						/* If we are to display the channel, make sure it matches the pattern first, and we haven't shown too many entries already */
						if (display && (!stricmp(pattern, ci->name) || Anope::Match(ci->name, pattern)) && ++shown <= 100)
						{
							if (reg == REG_SHOW_ALL)
								u->SendMessage(s_DiceServ, "   %-20s  %-5s  %s", ci->name, me->GetLangString(u, DICE_LIST_REG), me->GetLangString(u, diceserv_ignore ? DICE_IGNORED : DICE_ALLOWED));
							else
								u->SendMessage(s_DiceServ, "   %-20s  %s", ci->name, me->GetLangString(u, diceserv_ignore ? DICE_IGNORED : DICE_ALLOWED));
						}
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
					if (nu->nc)
						continue;
					display = false;
					/* We will only show the channel if it was part of the ignore type request */
					bool diceserv_ignore = nu->GetExt("diceserv_ignore");
					if (diceserv_ignore && (show == SHOW_IGNORED || show == SHOW_ALL))
						display = true;
					else if (!diceserv_ignore && (show == SHOW_ALLOWED || show == SHOW_ALL))
						display = true;
					/* If we are to display the nick, make sure it matches the pattern first, and we haven't shown too many entries already */
					if (display && (!stricmp(pattern, nu->nick) || Anope::Match(nu->nick, pattern)) && ++shown <= 100)
					{
						if (reg == REG_SHOW_ALL)
							u->SendMessage(s_DiceServ, "   %-20s  %-5s  %s", nu->nick, me->GetLangString(u, DICE_LIST_UNREG), me->GetLangString(u, diceserv_ignore ? DICE_IGNORED : DICE_ALLOWED));
						else
							u->SendMessage(s_DiceServ, "   %-20s  %s", nu->nick, me->GetLangString(u, diceserv_ignore ? DICE_IGNORED : DICE_ALLOWED));
					}
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
						if ((na->status & NS_FORBIDDEN) || (na->nc && (na->nc->flags & NI_SUSPENDED)))
							continue;
						display = false;
						/* We will only show the channel if it was part of the ignore type request */
						bool diceserv_ignore = na->nc->GetExt("diceserv_ignore");
						if (diceserv_ignore && (show == SHOW_IGNORED || show == SHOW_ALL))
							display = true;
						else if (!diceserv_ignore && (show == SHOW_ALLOWED || show == SHOW_ALL))
							display = true;
						/* If we are to display the nick, make sure it matches the pattern first, and we haven't shown too many entries already */
						if (display && (!stricmp(pattern, na->nick) || Anope::Match(na->nick, pattern)) && ++shown <= 100)
						{
							if (reg == REG_SHOW_ALL)
								u->SendMessage(s_DiceServ, "   %-20s  %-5s  %s", na->nick, me->GetLangString(u, DICE_LIST_REG), me->GetLangString(u, diceserv_ignore ? DICE_IGNORED : DICE_ALLOWED));
							else
								u->SendMessage(s_DiceServ, "   %-20s  %s", na->nick, me->GetLangString(u, diceserv_ignore ? DICE_IGNORED : DICE_ALLOWED));
						}
					}
				}
		}
		/* Show the footer */
		me->NoticeLang(s_DiceServ, u, DICE_LIST_END, shown > 100 ? 100 : shown, shown);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &)
	{
		me->NoticeLang(s_DiceServ, u, DICE_SERVADMIN_HELP_LIST);
		return true;
	}

	void OnSyntaxError(User *u)
	{
		me->NoticeLang(s_DiceServ, u, DICE_LIST_SYNTAX);
	}
};

class DiceServCore : public Module
{
 public:
	DiceServCore(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		me = this;

		this->AddCommand(DiceServ_cmdTable, new CommandDiceServHelp());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServRoll());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServCalc());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServExRoll());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServExCalc());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServDnD3eChar());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServEarthdawn());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServSet());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServStatus());
		this->AddCommand(DiceServ_cmdTable, new CommandDiceServList());

		Implementation events[] = {I_OnBotPreLoad, I_OnPostCommand, I_OnPostLoadDatabases, I_OnSaveDatabase, I_OnBackupDatabase, I_OnUserConnect, I_OnUserNickChange, I_OnUserLogoff, I_OnNickRegister, I_OnDelCore,
			I_OnChanRegistered, I_OnJoinChannel, I_OnPrePartChannel, I_OnDelChan, I_OnBotFantasy};
		ModuleManager::Attach(events, this, sizeof(events) / sizeof(events[0]));

		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);
		do_load_config();

		DiceServ = findbot(s_DiceServ);
		if (!DiceServ)
			DiceServ = new BotInfo(s_DiceServ, ServiceUser, ServiceHost, "Dice Roll Service");
		DiceServ->cmdTable = DiceServ_cmdTable;

		Mersenne_RandomInit(time(0));

		this->add_languages();
	}
	~DiceServCore()
	{
		OnSaveDatabase();
		delete [] s_DiceServ;
		for (int i = 0; i < MAX_CMD_HASH; ++i)
			for (CommandHash *current = DiceServ_cmdTable[i]; current; current = current->next)
				for (Command *c = current->c; c; c = c->next)
					this->DelCommand(DiceServ_cmdTable, c->name.c_str());
		if (DiceServ)
		{
			ircdproto->SendQuit(DiceServ, "Module Unloaded!");
			delete DiceServ;
		}
	}

	void OnBotPreLoad(BotInfo *bi)
	{
		if (!strcmp(bi->nick, s_DiceServ))
		{
			delete DiceServ;
			DiceServ = bi;
			DiceServ->cmdTable = DiceServ_cmdTable;
		}
	}

	/** Handles adding a line for DiceServ status to NS INFO and CS INFO
	 */
	void OnPostCommand(User *u, const std::string &service, const ci::string &command, const std::vector<ci::string> &params)
	{
		if (command == "INFO")
		{
			if (service == s_NickServ)
			{
				ci::string nick = params.size() ? params[0] : "";

				if (!nick.empty() && u->nc && u->nc->HasCommand("diceserv/info"))
				{
					NickAlias *na = findnick(nick.c_str());
					if (na)
						me->NoticeLang(s_NickServ, u, DICE_INFO_IGNORE, "  ", s_DiceServ, me->GetLangString(u, na->nc->GetExt("diceserv_ignore") ? DICE_IGNORED : DICE_ALLOWED));
				}
			}
			else if (service == s_ChanServ)
			{
				ci::string chan = params.size() ? params[0] : "";

				if (!chan.empty())
				{
					ChannelInfo *ci = cs_findchan(chan.c_str());
					if (ci && (((ci->flags & CI_SECUREFOUNDER) ? is_real_founder(u, ci) : is_founder(u, ci)) || (u->nc && u->nc->HasCommand("diceserv/info"))))
						me->NoticeLang(s_ChanServ, u, DICE_INFO_IGNORE, "", s_DiceServ, me->GetLangString(u, ci->GetExt("diceserv_ignore") ? DICE_IGNORED : DICE_ALLOWED));
				}
			}
		}
	}

	void OnPostLoadDatabases()
	{
		char readbuf[1024];
		FILE *in = fopen(DiceServDBName.c_str(), "rb");

		/* Fail if we were unable to open the database */
		if (!in)
		{
			alog("[diceserv] Unable to open database ('%s') for reading", DiceServDBName.c_str());
			return;
		}

		/* Read in the database line by line, format is: {N|C} {nick|channel} */
		while (fgets(readbuf, 1024, in))
		{
			char *ignore_type = myStrGetToken(readbuf, ' ', 0), *ignore_name = myStrGetToken(readbuf, ' ', 1);
			if (ignore_type)
			{
				if (ignore_name)
				{
					unsigned len = strlen(ignore_name);
					/* Make sure to get rid of the trailing \n */
					ignore_name[len - 1] = 0;

					/* If type is C, it's a channel, find it and add a ignore to it */
					if (!stricmp(ignore_type, "C"))
					{
						ChannelInfo *ci = cs_findchan(ignore_name);
						if (ci)
							ci->Extend("diceserv_ignore", new bool(true));
					}
					/* If type is N, it's a nick, find it and add a ignore to it */
					else if (!stricmp(ignore_type, "N"))
					{
						NickAlias *na = findnick(ignore_name);
						if (na)
							na->nc->Extend("diceserv_ignore", new bool(true));
					}

					delete [] ignore_name;
				}
				delete [] ignore_type;
			}
		}

		fclose(in);

		if (debug)
			alog("[diceserv] Successfully loaded database");
	}

	void OnSaveDatabase()
	{
		FILE *out = fopen(DiceServDBName.c_str(), "wb");

		/* Fail if we were unable to open the database */
		if (!out)
		{
			alog("[diceserv] Unable to open database ('%s') for writing", DiceServDBName.c_str());
			return;
		}

		int i;
		/* Go through all the registered channels and add them to the database file if they have a ignore on them */
		for (i = 0; i < 256; ++i)
		{
			ChannelInfo *ci = chanlists[i];
			for (; ci; ci = ci->next)
				if (ci->GetExt("diceserv_ignore"))
					fprintf(out, "C %s\n", ci->name);
		}

		/* Go through all the registered nicks and add them to the database file if they have a ignore on them */
		for (i = 0; i < 1024; ++i)
		{
			NickCore *nc = nclists[i];
			for (; nc; nc = nc->next)
				if (nc->GetExt("diceserv_ignore"))
					fprintf(out, "N %s\n", nc->display);
		}

		fclose(out);

		if (debug)
			alog("[diceserv] Successfully saved database");
	}

	void OnBackupDatabase()
	{
		ModuleDatabaseBackup(DiceServDBName.c_str());
	}

	void OnUserConnect(User *u)
	{
		this->NickEvent(u);
	}

	void OnUserNickChange(User *u, const char *oldnick)
	{
		this->NickEvent(u);
	}

	void OnUserLogoff(User *u)
	{
		bool *diceserv_ignore;
		if (u->GetExt("diceserv_ignore", diceserv_ignore))
		{
			delete diceserv_ignore;
			u->Shrink("diceserv_ignore");
		}
	}

	void OnNickRegister(NickAlias *na)
	{
		User *u = finduser(na->nick);

		if (u->GetExt("diceserv_ignore"))
			u->nc->Extend("diceserv_ignore", new bool(true));
	}

	void OnDelCore(NickCore *nc)
	{
		bool *diceserv_ignore;
		if (nc->GetExt("diceserv_ignore", diceserv_ignore))
		{
			delete diceserv_ignore;
			nc->Shrink("diceserv_ignore");
		}
	}

	void OnChanRegistered(ChannelInfo *ci)
	{
		int index = find_chan_ignore(ci->name);

		if (index != -1)
			ci->Extend("diceserv_ignore", new bool(true));
	}

	void OnJoinChannel(User *u, Channel *c)
	{
		int index = find_chan_ignore(c->name);

		if (index == -1 && c->ci && c->ci->GetExt("diceserv_ignore"))
			add_chan_ignore(c->name);
	}

	void OnPrePartChannel(User *u, Channel *c)
	{
		/* When there is 1 user in the channel and we get this event, the channel is being emptied */
		if (c->usercount == 1)
			del_chan_ignore(c->name);
	}

	void OnDelChan(ChannelInfo *ci)
	{
		bool *diceserv_ignore;
		if (ci->GetExt("diceserv_ignore", diceserv_ignore))
		{
			delete diceserv_ignore;
			ci->Shrink("diceserv_ignore");
		}
	}

	void OnBotFantasy(char *command, User *u, ChannelInfo *ci, char *params)
	{
		if (stricmp(command, "dnd3echar") && !params)
			return;

		char *origdice = NULL, *comment = NULL;
		DICE_TYPES dice_type;
		bool round_result = true;
		/* Check for one of the following fantasy commands, and stop processing if the command isn't one of these */
		if (!stricmp(command, "roll"))
		{
			origdice = myStrGetToken(params, ' ', 0);
			comment = myStrGetTokenRemainder(params, ' ', 1);
			dice_type = DICE_TYPE_ROLL;
		}
		else if (!stricmp(command, "calc"))
		{
			origdice = myStrGetToken(params, ' ', 0);
			comment = myStrGetTokenRemainder(params, ' ', 1);
			dice_type = DICE_TYPE_CALC;
			round_result = false;
		}
		else if (!stricmp(command, "exroll"))
		{
			origdice = myStrGetToken(params, ' ', 0);
			comment = myStrGetTokenRemainder(params, ' ', 1);
			dice_type = !origdice || !strcmp(origdice, "%") || strchr(origdice, 'd') || strchr(origdice, 'D') || strcasestr(origdice, "rand(") ? DICE_TYPE_EXROLL : DICE_TYPE_ROLL;
		}
		else if (!stricmp(command, "excalc"))
		{
			origdice = myStrGetToken(params, ' ', 0);
			comment = myStrGetTokenRemainder(params, ' ', 1);
			dice_type = !origdice || !strcmp(origdice, "%") || strchr(origdice, 'd') || strchr(origdice, 'D') || strcasestr(origdice, "rand(") ? DICE_TYPE_EXCALC : DICE_TYPE_CALC;
			round_result = false;
		}
		else if (!stricmp(command, "dnd3echar"))
		{
			if (params)
				comment = sstrdup(params);
			dice_type = DICE_TYPE_DND3E;
		}
		else if (!stricmp(command, "earthdawn"))
		{
			origdice = myStrGetToken(params, ' ', 0);
			comment = myStrGetTokenRemainder(params, ' ', 1);
			dice_type = DICE_TYPE_EARTHDAWN;
		}
		else
			return;

		/* Determine the roll source, should be the channel's BotServ bot unless the IRCd is somehow sending the command
		 * without a BotServ bot in the channel, in which case the result will be echoed by DiceServ */
		RollSource = ci->bi ? ci->bi->nick : s_DiceServ;
		SourceIsBot = true;
		diceserv_roller(u, origdice ? origdice : "", ci->name, comment ? comment : "", dice_type, round_result);

		if (origdice)
			delete [] origdice;
		if (comment)
			delete [] comment;
	}
 private:
	void NickEvent(User *u)
	{
		if (u->nc && u->nc->GetExt("diceserv_ignore"))
			u->Extend("diceserv_ignore", new bool(true));
	}

	void add_languages()
	{
		const char *langtable_en_us[] = {
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
			"the format of: [z]dw, where z is the number of dice to\n"
			"be thrown, and w is the number of sides on each die. z is\n"
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
			"  /msg %s DND3ECHAR\n"
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
		this->InsertLanguage(LANG_EN_US, DICE_NUM_STRINGS, langtable_en_us);
	}
};

MODULE_INIT(DiceServCore)
