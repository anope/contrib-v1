/**
 * Error codes generated and returned through SQL by this module.
 *
 ***********
 * Module Name    : misc_sqlcmd
 * Author         : Viper <Viper@Anope.org>
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
 * Last Updated   : 15/03/2011
 *
 **/

/* No error occurred! */
#define SQLCMD_ERROR_NONE						1

/* Common error return values */
#define SQLCMD_ERROR_READ_ONLY					-1
#define SQLCMD_ERROR_DEFCON						-2
#define SQLCMD_ERROR_SYNTAX_ERROR				-3
#define SQLCMD_ERROR_ACCESS_DENIED				-4
#define SQLCMD_ERROR_PERMISSION_DENIED			-5
#define SQLCMD_ERROR_MORE_OBSCURE_PASS			-6		/* Currently not send by module. */
#define SQLCMD_ERROR_PASS_TOO_LONG				-7
#define SQLCMD_ERROR_CHECKSUM					-500
#define SQLCMD_ERROR_UNKNOWN_CMD				-501
#define SQLCMD_ERROR_TIMESTAMP_PAST				-502
#define SQLCMD_ERROR_TIMESTAMP_FUTURE			-503

/* Nickname related errors */
#define SQLCMD_ERROR_NICK_NOT_REGISTERED		-20
#define SQLCMD_ERROR_NICK_ALREADY_REQUESTED		-21
#define SQLCMD_ERROR_NICK_ALREADY_REGISTERED	-22
#define SQLCMD_ERROR_NICK_FORBIDDEN				-23
#define SQLCMD_ERROR_NICK_SUSPENDED				-24
#define SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED  -25
#define SQLCMD_ERROR_NICK_IN_USE				-26
#define SQLCMD_ERROR_EMAIL_INVALID				-27
#define SQLCMD_ERROR_NICK_CONF_NOT_FOUND		-28
#define SQLCMD_ERROR_NICK_CONF_INVALID			-29
#define SQLCMD_ERROR_NICK_REG_FAILED			-30
#define SQLCMD_ERROR_NICK_GROUP_CHANGE_DISABLED	-31
#define SQLCMD_ERROR_NICK_GROUP_SAME			-32
#define SQLCMD_ERROR_NICK_GROUP_TOO_MANY		-33

/* Channel related errors */
#define SQLCMD_ERROR_CHAN_NOT_REGISTERED		-50
#define SQLCMD_ERROR_CHAN_FORBIDDEN				-51
#define SQLCMD_ERROR_CHAN_SUSPENDED				-52
#define SQLCMD_ERROR_CHAN_SYM_REQ				-53
#define SQLCMD_ERROR_CHAN_INVALID				-54
#define SQLCMD_ERROR_CHAN_MUST_BE_EMPTY			-55
#define SQLCMD_ERROR_CHAN_NOT_IN_USE			-56
#define SQLCMD_ERROR_CHAN_ALREADY_REG			-57
#define SQLCMD_ERROR_CHAN_MAY_NOT_BE_REG		-58
#define SQLCMD_ERROR_REACHED_CHAN_LIMIT			-59
#define SQLCMD_ERROR_CHAN_REG_FAILED			-60
#define SQLCMD_ERROR_CHAN_NOT_XOP				-61
#define SQLCMD_ERROR_CHAN_NOT_ACC				-62
#define SQLCMD_ERROR_CHAN_ACCESS_REACHED_LIMIT	-63
#define SQLCMD_ERROR_CHAN_ACC_NOT_FOUND			-64
#define SQLCMD_ERROR_CHAN_ACC_LVL_NONZERO		-65
#define SQLCMD_ERROR_CHAN_ACC_LVL_RANGE			-66

/* Memo related errors */
#define SQLCMD_ERROR_MEMO_X_GETS_NO_MEMOS		-80
#define SQLCMD_ERROR_MEMO_X_HAS_TOO_MANY_MEMOS	-81
#define SQLCMD_ERROR_NO_MEMOS					-82
#define SQLCMD_ERROR_MEMO_DOES_NOT_EXIST		-83
#define SQLCMD_ERROR_MEMO_DELETED_NONE			-84

/* Bot related errors */
#define SQLCMD_ERROR_BOT_NO_EXIST               -90
#define SQLCMD_ERROR_BOT_ALREADY_ASSIGNED       -91
#define SQLCMD_ERROR_BOT_NOT_ASSIGNED           -92
#define SQLCMD_ERROR_BOT_NOT_ON_CHAN            -93

/* EOF */
