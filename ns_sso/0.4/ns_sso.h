/* -*- Mode: C; tab-width: 4 -*- */
/**
 * Single Sign-On integration module for Anope IRC Services
 *
 * @package ns_sso
 * @author 'Zabadab'
 * 
 * Copyright (C) 2009 'Zabadab'
 *
 * Portions adapted from code written by the Anope Team.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * For proper viewing of this file, use tab-widh of four characters.
 */

#define AUTHOR "Zabadab"
#define VERSION "0.4"

// Language constants
#define LNG_TICKETIDENTIFY_HELP		0
#define LNG_TICKETGHOST_HELP		1
#define LNG_CHANGE_EMAIL_AT_SITE	2
#define LNG_SIGN_UP_AT_SITE			3
#define LNG_DROP_AT_SITE			4
#define LNG_TICKETIDENTIFY_SYNTAX	5
#define LNG_TICKETGHOST_SYNTAX		6
#define LNG_CHANGING_YOUR_NICK		7
#define LNG_TAKEN_OWNERSHIP_OF_NICK	8
#define LNG_DROPPED_YOUR_NICK		9

#define LNG_NO_CONSTANTS			10

// From ns_register.c
#define TO_COLLIDE		0
#define TO_RELEASE		1

// checkTicket return values
#define TICKET_OK		1
#define TICKET_BAD		-1
#define TICKET_ERROR	-2

// loadSettings return values
#define	SETTINGS_BAD	0
#define SETTINGS_OK		1

// timerCallback task error codes
#define TASK_OK						0 // Task completed successfully
#define TASK_UNKNOWN				1 // Unknown task
#define TASK_READONLY				2 // Database is read-only
#define TASK_DEFCON					3 // Defcon prevents nick registration
#define TASK_NICK_REQUESTED			4 // Someone already claimed the nick but has not confirmed it(?)
#define TASK_GUEST_NICK				5 // Guest nicknames cannot be registered
#define TASK_INVALID_NICK			6 // Invalid nickname according to anope_valid_nick
#define TASK_IS_OPER_NICK			7 // The nick belongs to an IRC operator
#define TASK_NICK_FORBIDDEN			8 // The nickname was forbidden (may happen sometimes but shouldn't really)
#define TASK_ALREADY_REGISTERED		9 // The nickname is already registered (if this happens, something is seriously wrong)
#define TASK_PASS_TOO_LONG			10 // Password is too long (should not happen if config.h was patched to set PASSMAX to 256)
#define TASK_EMAIL_INVALID			11 // E-Mail address is invalid according to MailValidate (should never happen except if Anope can't cope with stuff like .museum)
#define TASK_REQUEST_FAILED			12 // makerequest failed to return a NickRequest (should never happen?)
#define TASK_CREATE_FAILED			13 // makenick failed to return a NickAlias (should never happen?)
#define TASK_NICKS_SAME				14 // Nick and NewNick are the same...
#define TASK_NICK_NOT_FOUND			15 // Nick does not exist (should never happen if DBs stay in sync)
#define TASK_NICK_COLLISION			16 // NewNick already exists; now we're in trouble...
#define TASK_NO_NICKCORE			17 // NickAlias has no NickCore. WTF??
#define TASK_DELNICK_FAILED			18 // DelNick returned 0.
#define TASK_ENCRYPT_FAILED			19 // enc_encrypt failed (returned < 0)
#define TASK_ADD_AKILL_FAILED		20 // add_akill returned -1
#define TASK_AKILL_MASK_NOT_FOUND	21 // Anope could not find the task's BanMask in the akills slist.
#define TASK_INVALID_DB_PARAMS		22 // Required parameters are missing from the database or are invalid.

typedef struct TimerTask
{
	int ID;					// ID of database row
	int UID;				// Website UID of user affected
	
	char *Task;				// Task to perform

	char *Nick;				// (old) Nickname
	char *NewNick;			// New Nickname (may be NULL)
	char *EMail;			// E-Mail (may be NULL)
	char *NewPassword;		// New password to set (may be NULL)

	char *Reason;			// Reason for A-Kill (may be NULL)
	char *BanMask;			// Mask (nick!user@host) to A-Kill (may be NULL)
	int Expires;			// Seconds until A-Kill expires (may be NULL)
	
	int Completed;			// Whether the task has been completed (0 = false or 1 = true)
	int ErrorCode;			// Error code encountered while performing the task (0 = no error)
} TimerTask;

int timerCallback(int argc, char **argv);
int updateTimerTask(TimerTask *Task);
void freeTimerTask(TimerTask *Task);

void registerNick(TimerTask *Task);
void renameNick(TimerTask *Task);
void changePassword(TimerTask *Task);
void changeEMail(TimerTask *Task);
void dropNick(TimerTask *Task);
void addAKill(TimerTask *Task);
void delAKill(TimerTask *Task);

int ticketIdentifyHelpRequest(User * u);
int ticketIdentifyCallback(User *u);
int ticketGhostHelpRequest(User *u);
int ticketGhostCallback(User *u);

int checkTicket(char* ticket, char* hostip, char* usernick, int markused);

int nsRegisterCallback(User *u);
int nsDropCallback(User *u);
int nsSetCallback(User *u);

void createAndRegisterLanguageConstants(void);

int loadSettings(void);