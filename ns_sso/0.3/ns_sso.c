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
 * For proper viewing of this file, use tab-width of four characters.
 */

#include "libpq-fe.h" // PostgreSQL
#include "module.h"
#include "ns_sso.h"

PGconn *dbConn;

char *CONN_STR;
char *SITE_NAME;
char *BASE_URL;
int CALLBACK_TIMEOUT;
int TICKETIDENTIFY_ONLY;
int TICKET_TTL;

int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	if (!loadSettings())
		return MOD_STOP;

	createAndRegisterLanguageConstants();

	// Create NickServ TICKETIDENTIFY command
	c = createCommand("TICKETIDENTIFY", ticketIdentifyCallback, NULL, -1, -1, -1, -1, -1);
	
	moduleAddHelp(c, ticketIdentifyHelpRequest);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	alog("ns_sso: INFO: Add command 'TICKETIDENTIFY' status: %d", status);

	if (status)
		return MOD_STOP;

	if (!TICKETIDENTIFY_ONLY)
	{
		if (NSMaxAliases != 1)
			alog("ns_sso: WARNING: NSMaxAliases is %d. You should set it to %d, otherwise your "
				"user database WILL eventually go out of sync with Anope's database.", NSMaxAliases, 1);

		// Overwrite NickServ REGISTER command
		c = createCommand("REGISTER", nsRegisterCallback, NULL, -1, -1, -1, -1, -1);

		status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
		alog("ns_sso: INFO: Add command 'REGISTER' status: %d", status);
		
		if (status)
			return MOD_STOP;

		// Overwrite NickServ DROP command
		c = createCommand("DROP", nsDropCallback, NULL, -1, -1, -1, -1, -1);

		status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
		alog("ns_sso: INFO: Add command 'DROP' status: %d", status);
		
		if (status)
			return MOD_STOP;

		// Overwrite NickServ SET command to trap change of E-Mail or password
		c = createCommand("SET", nsSetCallback, NULL, -1, -1, -1, -1, -1);

		status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
		alog("ns_sso: INFO: Add command 'SET' status: %d", status);

		if (status)
			return MOD_STOP;

		// Add timer callback
		status = moduleAddCallback("SyncChanges", time(NULL) + CALLBACK_TIMEOUT, timerCallback, 0, NULL);
		alog("ns_sso: INFO: Add callback 'SyncChanges' status: %d", status);

		if (status)
			return MOD_STOP;

		alog("ns_sso: INFO: Callback will now be called every %d seconds", CALLBACK_TIMEOUT);

		// Connect to database
		alog("ns_sso: INFO: Connecting to PostgreSQL using connection string \"%s\"", CONN_STR);
		dbConn = PQconnectdb(CONN_STR);

		if (PQstatus(dbConn) != CONNECTION_OK)
		{
			alog("ns_sso: ERROR: Connection to PostgreSQL failed: %s", PQerrorMessage(dbConn));
			PQfinish(dbConn);
			return MOD_STOP;
		}
	}

	return MOD_CONT;
}

void AnopeFini(void)
{
	if (dbConn)
		PQfinish(dbConn);

	alog("ns_sso: INFO: Unloaded successfully.");
}

int timerCallback(int argc, char **argv)
{
	unsigned short COL_ID, COL_UID, COL_TASK, COL_NICK, COL_NEWNICK,
		COL_EMAIL, COL_NEWPASSWORD, COL_REASON, COL_BANMASK, COL_EXPIRES,
		COL_COMPLETED, COL_ERRORCODE;

	unsigned short numTasks;
	int i, ret;

	PGresult *res;
	TimerTask *timerTasks = NULL;

	//alog(sso: INFO: timerCallback(): Looking for work...");

	// Since this is C, and C is a synonym for "difficult", using PostgreSQL
	// is also difficult. We need to create a transaction, then a cursor for
	// our query, an then read in tasks row by row.

	// Start transaction
	res = PQexec(dbConn, "BEGIN");

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		alog("ns_sso: ERROR: timerCallback(): Unable to start a transaction: %s", PQerrorMessage(dbConn));
		PQclear(res);

		goto done;
	}

	PQclear(res);

	// Create cursor
	res = PQexec(dbConn, "DECLARE tasks CURSOR FOR SELECT * FROM \"irctasks\" WHERE \"Completed\" = false ORDER BY \"ID\" ASC");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		alog("ns_sso: ERROR: timerCallback(): Unable to declare cursor: %s", PQerrorMessage(dbConn));
		PQclear(res);

		goto done;
	}

	// Now get all results
	res = PQexec(dbConn, "FETCH ALL in tasks");
	
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		alog("ns_sso: ERROR: timerCallback(): Unable to fetch all rows: %s", PQerrorMessage(dbConn));
		PQclear(res);

		goto done;
	}

	// Get the column positions
	COL_ID = PQfnumber(res, "\"ID\"");
	COL_UID = PQfnumber(res, "\"UID\"");
	COL_TASK = PQfnumber(res, "\"Task\"");
	COL_NICK = PQfnumber(res, "\"Nick\"");
	COL_NEWNICK = PQfnumber(res, "\"NewNick\"");
	COL_EMAIL = PQfnumber(res, "\"EMail\"");
	COL_NEWPASSWORD = PQfnumber(res, "\"NewPassword\"");
	COL_REASON = PQfnumber(res, "\"Reason\"");
	COL_BANMASK = PQfnumber(res, "\"BanMask\"");
	COL_EXPIRES = PQfnumber(res, "\"Expires\"");
	COL_COMPLETED = PQfnumber(res, "\"Completed\"");
	COL_ERRORCODE = PQfnumber(res, "\"ErrorCode\"");

	if (COL_ID == -1 || COL_UID == -1 || COL_TASK == -1 || COL_NICK == -1 || COL_NEWNICK == -1 ||
		COL_EMAIL == -1 || COL_NEWPASSWORD == -1 || COL_REASON == -1 || COL_BANMASK == -1 ||
		COL_EXPIRES == -1 || COL_COMPLETED == -1 || COL_ERRORCODE == -1)
	{
		alog("ns_sso: ERROR: timerCallback(): Some columns do not exist.");
		alog("ns_sso: ERROR: timerCallback(): ID=%d, UID=%d, TASK=%d, NICK=%d, NEWNICK=%d, EMAIL=%d, NEWPASSWORD=%d, COMPLETED=%d, ERRORCODE=%d",
			COL_ID, COL_UID, COL_TASK, COL_NICK, COL_NEWNICK, COL_EMAIL, COL_NEWPASSWORD, COL_COMPLETED, COL_ERRORCODE);

		PQclear(res);

		goto done;
	}

	// Put tasks in memory
	//alog("ns_sso: INFO: timerCallback(): %d tasks to work on.", PQntuples(res));

	timerTasks = malloc(sizeof(TimerTask) * PQntuples(res));

	for (i = 0; i < PQntuples(res); i++)
	{
		/* We check for NULL for *all* columns, because we don't want
		 * to segfault Anope in case some idiot admin changed the DB
		 * schema so columns that must not have NULL values suddenly
		 * do. */

		// ID
		if (PQgetisnull(res, i, COL_ID)) // May not actually be NULL
			timerTasks[i].ID = 0;
		else
			timerTasks[i].ID = atoi(PQgetvalue(res, i, COL_ID));

		// UID
		if (PQgetisnull(res, i, COL_UID)) // May not actually be NULL
			timerTasks[i].UID = 0;
		else
			timerTasks[i].UID = atoi(PQgetvalue(res, i, COL_UID));

		// Task
		if (PQgetisnull(res, i, COL_TASK)) // May not actually be NULL
			timerTasks[i].Task = NULL;
		else
			timerTasks[i].Task = strdup(PQgetvalue(res, i, COL_TASK));

		// Nick
		if (PQgetisnull(res, i, COL_NICK)) // May not actually be NULL
			timerTasks[i].Nick = NULL;
		else
			timerTasks[i].Nick = strdup(PQgetvalue(res, i, COL_NICK));

		// NewNick
		if (PQgetisnull(res, i, COL_NEWNICK))
			timerTasks[i].NewNick = NULL;
		else
			timerTasks[i].NewNick = strdup(PQgetvalue(res, i, COL_NEWNICK));

		// EMail
		if (PQgetisnull(res, i, COL_EMAIL))
			timerTasks[i].EMail = NULL;
		else
			timerTasks[i].EMail = strdup(PQgetvalue(res, i, COL_EMAIL));

		// NewPassword
		if (PQgetisnull(res, i, COL_NEWPASSWORD))
			timerTasks[i].NewPassword = NULL;
		else
			timerTasks[i].NewPassword = strdup(PQgetvalue(res, i, COL_NEWPASSWORD));

		// Reason
		if (PQgetisnull(res, i, COL_REASON))
			timerTasks[i].Reason = NULL;
		else
			timerTasks[i].Reason = strdup(PQgetvalue(res, i, COL_REASON));

		// BanMask
		if (PQgetisnull(res, i, COL_BANMASK))
			timerTasks[i].BanMask = NULL;
		else
			timerTasks[i].BanMask = strdup(PQgetvalue(res, i, COL_BANMASK));

		// Expires
		if (PQgetisnull(res, i, COL_EXPIRES))
			timerTasks[i].Expires = 0;
		else
			timerTasks[i].Expires = atoi(PQgetvalue(res, i, COL_EXPIRES));

		// Completed
		if (PQgetisnull(res, i, COL_COMPLETED)) // May not actually be NULL
			timerTasks[i].Completed = 0;
		else
			timerTasks[i].Completed = (strcmp(PQgetvalue(res, i, COL_COMPLETED), "t") == 0 ? 1 : 0);

		// ErrorCode
		if (PQgetisnull(res, i, COL_ERRORCODE)) // May not actually be NULL
			timerTasks[i].ErrorCode = 0;
		else
			timerTasks[i].ErrorCode = atoi(PQgetvalue(res, i, COL_ERRORCODE));
	}

	// Clear PostgreSQL result
	numTasks = PQntuples(res); // Keep the number of tasks around since it will go away now
	PQclear(res);

	// Close cursor and end transaction;
	res = PQexec(dbConn, "CLOSE tasks");
	PQclear(res);

	res = PQexec(dbConn, "END");
	PQclear(res);

	// Perform the tasks
	for (i = 0; i < numTasks; i++)
	{
		alog("ns_sso: INFO: timerCallback(): Processing task %d of %d.", (i + 1), numTasks);

		if (timerTasks[i].Completed == 1)
		{
			// If this happens, we should really figure out why PostgreSQL returned the task at all!

			alog("ns_sso: WARNING: timerCallback(): Task %d of %d is already marked as completed. "
				" It will not be performed a second time.", (i + 1), numTasks);

			goto taskDone;
		}

		if (!timerTasks[i].Task)
		{
			// This should never happen unless the user is purposely messing around with the schema!

			timerTasks[i].Completed = true;
			timerTasks[i].ErrorCode = TASK_INVALID_DB_PARAMS;
			alog("ns_sso: ERROR: timerCallback(): \"Task\" column for ID %d is NULL. If you have "
				"modified the database schema to force NULL values for columns that shouldn't "
				"have them, don't come asking for help when Anope segfaults.", timerTasks[i].ID);

			goto taskDone;
		}

		/* REGISTER Task */

		if (strcmp(timerTasks[i].Task, "REGISTER") == 0)
		{
			alog("ns_sso: INFO: timerCallback(): Registering new nick. Nick = %s, NewPassword = %s, EMail = %s",
				timerTasks[i].Nick, timerTasks[i].NewPassword, timerTasks[i].EMail);

			registerNick(&timerTasks[i]);

			goto taskDone;
		}

		/* RENAME Task */

		if (strcmp(timerTasks[i].Task, "RENAME") == 0)
		{
			alog("ns_sso: INFO: timerCallback(): Renaming nick. Nick = %s, NewNick = %s",
				timerTasks[i].Nick, timerTasks[i].NewNick);

			renameNick(&timerTasks[i]);

			goto taskDone;
		}

		/* CHGPASSWD Task */

		if (strcmp(timerTasks[i].Task, "CHGPASSWD") == 0)
		{
			alog("ns_sso: INFO: timerCallback(): Changing password. Nick = %s, NewPassword = %s",
				timerTasks[i].Nick, timerTasks[i].NewPassword);

			changePassword(&timerTasks[i]);

			goto taskDone;
		}

		/* CHGEMAIL Task */

		if (strcmp(timerTasks[i].Task, "CHGEMAIL") == 0)
		{
			alog("ns_sso: INFO: timerCallback(): Changing E-Mail address. Nick = %s, EMail = %s",
				timerTasks[i].Nick, timerTasks[i].EMail);

			changeEMail(&timerTasks[i]);

			goto taskDone;
		}

		/* DROP Task */

		if (strcmp(timerTasks[i].Task, "DROP") == 0)
		{
			alog("ns_sso: INFO: timerCallback(): Dropping nick. Nick = %s", timerTasks[i].Nick);

			dropNick(&timerTasks[i]);

			goto taskDone;
		}

		/* AKILLADD Task */

		if (strcmp(timerTasks[i].Task, "AKILLADD") == 0)
		{
			alog("ns_sso: INFO: timerCallback(): Adding A-Kill. Nick = %s, BanMask = %s, Reason = %s",
				timerTasks[i].Nick, timerTasks[i].BanMask, timerTasks[i].Reason);

			addAKill(&timerTasks[i]);

			goto taskDone;
		}

		/* AKILLDEL Task */

		if (strcmp(timerTasks[i].Task, "AKILLDEL") == 0)
		{
			alog("ns_sso: INFO: timerCallback(): Deleting A-Kill. Nick = %s, BanMask = %s",
				timerTasks[i].Nick, timerTasks[i].BanMask);

			delAKill(&timerTasks[i]);

			goto taskDone;
		}

		/* Unknown Task (we should never get here) */

		timerTasks[i].Completed = true;
		timerTasks[i].ErrorCode = TASK_UNKNOWN;
		alog("ns_sso: ERROR: timerCallback(): Don't know what to do for task: %s", timerTasks[i].Task);

	taskDone:

		if (timerTasks[i].ErrorCode != TASK_OK)
			alog("ns_sso: ERROR: timerCallback(): Task %d of %d failed with ErrorCode %d",
				(i + 1), numTasks, timerTasks[i].ErrorCode);

		alog("ns_sso: INFO: timerCallback(): Writing result of task %d of %d to database.",
			(i + 1), numTasks);

		ret = updateTimerTask(&timerTasks[i]);

		alog("ns_sso: INFO: timerCallback(): Finished processing task %d of %d.", (i + 1), numTasks);

		freeTimerTask(&timerTasks[i]);
	}

done:
	if (timerTasks)
		free(timerTasks);

	moduleAddCallback("SyncChanges", time(NULL) + CALLBACK_TIMEOUT, timerCallback, 0, NULL);

	//alog("ns_sso: INFO: timerCallback(): Finished.");

	return MOD_CONT;
}

void addAKill(TimerTask *Task)
{
	NickAlias *na;

	if (!Task->Nick || !Task->BanMask || !Task->Reason)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_INVALID_DB_PARAMS;
		return;
	}

	na = findnick(Task->Nick);

	if (!na)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NICK_NOT_FOUND;
		return;
	}

	// Suspend the nick on NickServ
	na->nc->flags |= NI_SUSPENDED;
	na->nc->flags |= NI_SECURE;
	na->nc->flags &= ~(NI_KILLPROTECT | NI_KILL_QUICK | NI_KILL_IMMED);

	{
		int i;
		User *u;
		NickAlias *na2;

		for (i = 0; i < na->nc->aliases.count; i++)
		{
			na2 = na->nc->aliases.list[i];
			if (na2->nc == na->nc)
			{
				na2->status &= ~(NS_IDENTIFIED | NS_RECOGNIZED);
				
				if (na2->last_quit)
					free(na2->last_quit);
				
				na2->last_quit = sstrdup(Task->Reason);

				if ((u = finduser(na2->nick)))
				{
					if (u->nickTrack)
					{
						free(u->nickTrack);
						u->nickTrack = NULL;
					}
				}

				/* force guestnick */
				collide(na2, 0);
			}
		}
	}

	send_event(EVENT_NICK_SUSPENDED, 1, Task->Nick);

	// Add the A-Kill
	
	/* 
	 * NB: add_akill will try to send errors to a user pointer, but the underlying
	 * notice function will stop if it finds that the destination user is a NULL
	 * pointer.
	 */

	{
		int ret;

		ret = add_akill(NULL, Task->BanMask, s_OperServ, Task->Expires, Task->Reason);
		
		if (ret == -1)
		{
			Task->Completed = true;
			Task->ErrorCode = TASK_ADD_AKILL_FAILED;
			return;
		}
	}

	Task->Completed = true;
	Task->ErrorCode = TASK_OK;
}

void delAKill(TimerTask *Task)
{
	NickAlias *na;
	int ret;

	if (!Task->Nick || !Task->BanMask)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_INVALID_DB_PARAMS;
		return;
	}

	na = findnick(Task->Nick);

	if (!na)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NICK_NOT_FOUND;
		return;
	}

	// Unsuspend the nick on NickServ
	na->nc->flags &= ~NI_SUSPENDED;
	send_event(EVENT_NICK_UNSUSPEND, 1, Task->Nick);

	// Remove the A-Kill

	if ((ret = slist_indexof(&akills, Task->BanMask)) == -1) {
		Task->Completed = true;
		Task->ErrorCode = TASK_AKILL_MASK_NOT_FOUND;
		return;
	}

	slist_delete(&akills, ret);
	
	if (ret == -1)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_ADD_AKILL_FAILED;
		return;
	}

	Task->Completed = true;
	Task->ErrorCode = TASK_OK;
}

void dropNick(TimerTask *Task)
{
	NickAlias *na;
	NickRequest *nr;
	int inUse = 0;

	if (!Task->Nick)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_INVALID_DB_PARAMS;
		return;
	}

	if (readonly)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_READONLY;
		return;
	}

	na = findnick(Task->Nick);

	if (!na)
	{
		// If we don't have a NickAlias, we might have a NickRequest.
		// If so, we just delete that and we are done.

		if ((nr = findrequestnick(Task->Nick)))
		{
			delnickrequest(nr);

			Task->Completed = true;
			Task->ErrorCode = TASK_OK;
			return;
		}

		Task->Completed = true;
		Task->ErrorCode = TASK_NICK_NOT_FOUND;
		return;
	}

	inUse = (na->u && nick_identified(na->u));

	if (ircd->sqline && (na->status & NS_VERBOTEN))
		anope_cmd_unsqline(na->nick);

	if (inUse)
		moduleNoticeLang(s_NickServ, na->u, LNG_DROPPED_YOUR_NICK, SITE_NAME);

	delnick(na);

	send_event(EVENT_NICK_DROPPED, 1, Task->Nick);

	Task->Completed = true;
	Task->ErrorCode = TASK_OK;
	return;
}

void changeEMail(TimerTask *Task)
{
	NickAlias *na;
	NickCore *nc;

	if (!Task->Nick || !Task->EMail)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_INVALID_DB_PARAMS;
		return;
	}

	na = findnick(Task->Nick);

	if (!na)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NICK_NOT_FOUND;
		return;
	}

	nc = na->nc;

	if (!nc)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NO_NICKCORE;
		return;
	}

	if (nc->email)
		free(nc->email);

	nc->email = sstrdup(Task->EMail);
	
	if (na->u && nick_identified(na->u))
		notice_lang(s_NickServ, na->u, NICK_SET_EMAIL_CHANGED, Task->EMail);

	Task->Completed = true;
	Task->ErrorCode = TASK_OK;
}

void changePassword(TimerTask *Task)
{
	NickAlias *na;
	NickCore *nc;

	int len;

	if (!Task->Nick || !Task->NewPassword)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_INVALID_DB_PARAMS;
		return;
	}

	len = strlen(Task->NewPassword);

	na = findnick(Task->Nick);

	if (!na)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NICK_NOT_FOUND;
		return;
	}

	nc = na->nc;

	if (!nc)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NO_NICKCORE;
		return;
	}

	if (enc_encrypt_check_len(len, PASSMAX - 1))
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_PASS_TOO_LONG;
		return;
	}

	if (enc_encrypt(Task->NewPassword, len, nc->pass, PASSMAX - 1) < 0)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_ENCRYPT_FAILED;
		return;
	}

	if (na->u && nick_identified(na->u))
		notice_lang(s_NickServ, na->u, NICK_SET_PASSWORD_CHANGED_TO, Task->NewPassword);

	Task->Completed = true;
	Task->ErrorCode = TASK_OK;
}

void renameNick(TimerTask *Task)
{
	NickCore *nc;
	NickAlias *naOld;
	NickAlias *naNew;
	int inUse = 0;

	if (!Task->Nick || !Task->NewNick)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_INVALID_DB_PARAMS;
		return;
	}

	if (strcmp(Task->Nick, Task->NewNick) == 0)
	{
		// That was easy...
		Task->Completed = true;
		Task->ErrorCode = TASK_NICKS_SAME;
		return;
	}

	naOld = findnick(Task->Nick);

	if (!naOld)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NICK_NOT_FOUND;
		return;
	}

	naNew = findnick(Task->NewNick);
	
	if (naNew)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NICK_COLLISION;
		return;
	}

	nc = naOld->nc;

	if (!nc)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_NO_NICKCORE; // WTF??
		return;
	}

	{
		// Guest any user using the old nick. At this point we know that this nick
		// is not registered with NickServ, so it's probably just some anonymous
		// idiot we don't care about.

		char guestnick[NICKMAX];
		User *u = finduser(Task->NewNick);

		if (u)
		{
			do
			{
				snprintf(guestnick, sizeof(guestnick), "%s%d", NSGuestNickPrefix, getrandom16());
			} while (finduser(guestnick));

			moduleNoticeLang(s_NickServ, u, LNG_TAKEN_OWNERSHIP_OF_NICK, SITE_NAME);

			anope_cmd_svsnick(u->nick, guestnick, time(NULL));
		}
	}

	if (naOld->u && nick_identified(naOld->u))
		inUse = 1;

	// Make a new NickAlias
	naNew = scalloc(1, sizeof(NickAlias));
	naNew->nick = sstrdup(Task->NewNick);
	naNew->nc = nc;
	slist_add(&nc->aliases, naNew);
	alpha_insert_alias(naNew);

	// Change the name of NickCore
	change_core_display(nc, Task->NewNick);

	if (NSNickTracking && naOld->u && nick_identified(naOld->u))
		nsStartNickTracking(naOld->u);

	// If a user used the old NickAlias and was identified, move them to the new one.
	if (inUse)
	{
		moduleNoticeLang(s_NickServ, naOld->u, LNG_CHANGING_YOUR_NICK, SITE_NAME);

		anope_cmd_svsnick(Task->Nick, Task->NewNick, time(NULL));
	}

	// Remove the old NickAlias (which logs anyone using it out too)
	if (!delnick(naOld))
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_DELNICK_FAILED;
		return;
	}

	Task->Completed = true;
	Task->ErrorCode = TASK_OK;
}

void registerNick(TimerTask *Task)
{
	NickRequest *nr = NULL, *anr = NULL;
	NickCore *nc = NULL;
	NickAlias *na = NULL;
	User *u;
	
	int i, prefixlen, nicklen;

	if (!Task->Nick || !Task->NewPassword || !Task->EMail)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_INVALID_DB_PARAMS;
		return;
	}

	prefixlen = strlen(NSGuestNickPrefix);
	nicklen = strlen(Task->Nick);

	if (readonly)
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_READONLY;
		return;
	}

	if (checkDefCon(DEFCON_NO_NEW_NICKS))
	{
		Task->Completed = true;
		Task->ErrorCode = TASK_DEFCON;
		return;
	}

	if ((anr = findrequestnick(Task->Nick)))
	{
		// Should never happen
		Task->Completed = true;
		Task->ErrorCode = TASK_NICK_REQUESTED;
		return;
	}

	// ["TheShadow" and "lara" are Epona developers!]

	/* Prevent "Guest" nicks from being registered. -TheShadow */

	/* Guest nick can now have a series of between 1 and 7 digits.
	*	  --lara
	*/

	if (nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 &&
		stristr(Task->Nick, NSGuestNickPrefix) == Task->Nick &&
		strspn(Task->Nick + prefixlen, "1234567890") == nicklen - prefixlen)
	{
		// Should never happen
		Task->Completed = true;
		Task->ErrorCode = TASK_GUEST_NICK;
		return;
	}

	if (!anope_valid_nick(Task->Nick))
	{
		// Should never happen
		Task->Completed = true;
		Task->ErrorCode = TASK_INVALID_NICK;
		return;
	}

	if (RestrictOperNicks)
	{
		for (i = 0; i < RootNumber; i++)
		{
			if (stristr(Task->Nick, ServicesRoots[i]))
			{
				// Should never happen
				Task->Completed = true;
				Task->ErrorCode = TASK_IS_OPER_NICK;
				return;
			}
		}
	}

	for (i = 0; i < servadmins.count && (nc = servadmins.list[i]); i++)
	{
		if (stristr(Task->Nick, nc->display))
		{
			// Should never happen
			Task->Completed = true;
			Task->ErrorCode = TASK_IS_OPER_NICK;
			return;
		}
	}

	for (i = 0; i < servopers.count && (nc = servopers.list[i]); i++)
	{
		if (stristr(Task->Nick, nc->display))
		{
			// Should never happen
			Task->Completed = true;
			Task->ErrorCode = TASK_IS_OPER_NICK;
			return;
		}
	}

	na = findnick(Task->Nick);

	if (na)
	{
		// Nickname already registered
		if (na->status & NS_VERBOTEN)
		{
			// Might happen some day but shouldn't
			Task->Completed = true;
			Task->ErrorCode = TASK_NICK_FORBIDDEN;
			return;
		}
		else
		{
			// Should never happen -- if it does, something is seriously wrong
			Task->Completed = true;
			Task->ErrorCode = TASK_ALREADY_REGISTERED;
			return;
		}
	}
	
	if (enc_encrypt_check_len(strlen(Task->NewPassword), PASSMAX - 1))
	{
		// Should not happen if config.h was patched to set PASSMAX to 256
		Task->Completed = true;
		Task->ErrorCode = TASK_PASS_TOO_LONG;
		return;
	}
	
	if (!MailValidate(Task->EMail))
	{
		// Should never happen except if Anope can't cope with stuff like .museum
		Task->Completed = true;
		Task->ErrorCode = TASK_EMAIL_INVALID;
		return;
	}

	/******* Here we actually do the registration. Above was just checks. *******/

	// This was makerequest function (sort of)
	nr = scalloc(1, sizeof(NickRequest));

	if (!nr)
	{
		// Should never happen(?)
		Task->Completed = true;
		Task->ErrorCode = TASK_REQUEST_FAILED;
		return;
	}

	nr->nick = sstrdup(Task->Nick);
	insert_requestnick(nr);
	nr->passcode = NULL; // Noone cares about it
	nr->email = sstrdup(Task->EMail);	
	nr->requested = time(NULL);
	// End

	strscpy(nr->password, Task->NewPassword, PASSMAX);
	enc_encrypt_in_place(nr->password, PASSMAX);

	// This was makenick function
	/* First make the core */
	nc = scalloc(1, sizeof(NickCore));
	nc->display = sstrdup(Task->Nick);
	slist_init(&nc->aliases);
	insert_core(nc);

	/* Then make the alias */
	na = scalloc(1, sizeof(NickAlias));
	na->nick = sstrdup(Task->Nick);
	na->nc = nc;
	slist_add(&nc->aliases, na);
	alpha_insert_alias(na);
	// End

	if (!na)
	{
		// Should never happen(?)
		Task->Completed = true;
		Task->ErrorCode = TASK_CREATE_FAILED;
		return;
	}
	
	memcpy(na->nc->pass, nr->password, PASSMAX);

	na->status = (int16) 0;
	na->nc->flags |= NSDefFlags;
	
	for (i = 0; i < RootNumber; i++)
	{
		if (!stricmp(ServicesRoots[i], nr->nick))
		{
			na->nc->flags |= NI_SERVICES_ROOT;
			break;
		}
	}

	na->nc->memos.memomax = MSMaxMemos;
	na->nc->channelmax = CSMaxReg;

	na->last_usermask = sstrdup("*@*");
	na->last_realname = sstrdup("unknown");
	na->time_registered = na->last_seen = time(NULL);
	na->nc->accesscount = 0;
	na->nc->access = NULL;
	na->nc->language = NSDefLanguage;
	na->nc->email = sstrdup(Task->EMail);

	u = finduser(Task->Nick);

	if (u)
	{
		u->na = na;
		na->u = u;
	}

	// Clean up
	delnickrequest(nr);

	send_event(EVENT_NICK_REGISTERED, 1, Task->Nick);

	// Save NickServ database. We do it explicitly because we're paranoid.
	save_ns_dbase();

	if (u)
		validate_user(u);

	Task->Completed = 1;
	Task->ErrorCode = TASK_OK;
}

void freeTimerTask(TimerTask *Task)
{
	free(Task->Task);
	free(Task->Nick);
	
	if (Task->NewNick)
		free(Task->NewNick);

	if (Task->EMail)
		free(Task->EMail);

	if (Task->NewPassword)
		free(Task->NewPassword);

	if (Task->Reason)
		free(Task->Reason);

	if (Task->BanMask)
		free(Task->BanMask);
}

int updateTimerTask(TimerTask *Task)
{
	char *paramValue1, *paramValue2;
	const char *paramValues[3];
	PGresult *res;
	
	paramValue1 = malloc(255 * sizeof(char));
	sprintf(paramValue1, "%d", Task->ErrorCode);

	paramValue2 = malloc(255 * sizeof(char));
	sprintf(paramValue2, "%d", Task->ID);

	paramValues[0] = (Task->Completed == 1 ? "true" : "false");
	paramValues[1] = paramValue1;
	paramValues[2] = paramValue2;

	res = PQexecParams(dbConn,
					   "UPDATE \"irctasks\" SET \"Completed\" = $1, \"ErrorCode\" = $2 WHERE \"ID\" = $3",
					   3, NULL, paramValues, NULL, NULL, 0);

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		free(paramValue1);
		free(paramValue2);
		PQclear(res);

		alog("ns_sso: ERROR: updateTimerTask(%d): Unable to update task: %s", Task->ID, PQerrorMessage(dbConn));

		return 1;
	}

	free(paramValue1);
	free(paramValue2);
	PQclear(res);

	alog("ns_sso: SUCCESS: updateTimerTask(%d): Task updated successfully.", Task->ID);

	return 0;
}

int checkTicket(char* ticket, char* hostip, char* usernick)
{
	const char *paramValues[2];
	PGresult *res;
	int COL_CREATED, COL_NICKNAME;

	char* createdTimeStampChars;
	int createdTimeStamp, timeNow;

	char* nicknameTicketIsFor;

	if (!ticket || !hostip || !usernick)
	{
		alog("ns_sso: ERROR: CheckTicket(%s, %s, %s): supplied values are not valid.",
			ticket, hostip, usernick);
		
		return TICKET_ERROR;
	}

	paramValues[0] = ticket;
	paramValues[1] = hostip;

	// Query the database
	res = PQexecParams(dbConn,
		"SELECT date_part('epoch', \"Created\")::int AS \"Created\", \"Nickname\" "
		"FROM \"tickets\" WHERE \"Ticket\" = $1 AND \"Used\" = false AND "
		"\"HostIP\" = $2 ORDER BY \"ID\" DESC LIMIT 1",
		2, NULL, paramValues, NULL, NULL, 0);

	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		alog("ns_sso: ERROR: CheckTicket(%s, %s, %s): Unable to query for ticket: %s",
			ticket, hostip, usernick, PQerrorMessage(dbConn));

		PQclear(res);

		return TICKET_ERROR;
	}

	if (PQntuples(res) != 1)
	{
		alog("ns_sso: FAIL: CheckTicket(%s, %s, %s): No valid ticket exists (expected %d "
			"ticket(s), got %d ticket(s))", ticket, hostip, usernick, 1, PQntuples(res));

		PQclear(res);

		return TICKET_BAD;
	}

	// Get the column positions
	COL_CREATED = PQfnumber(res, "\"Created\"");
	COL_NICKNAME = PQfnumber(res, "\"Nickname\"");

	if (COL_CREATED == -1 || COL_NICKNAME == -1)
	{
		alog("ns_sso: ERROR: checkTicket(%s, %s, %s): Some columns do not exist.",
			ticket, hostip, usernick);

		alog("ns_sso: ERROR: checkTicket(%s, %s, %s): Created=%d, Nickname=%d",
			ticket, hostip, usernick, COL_CREATED, COL_NICKNAME);

		PQclear(res);

		return TICKET_BAD;
	}

	// Get the timestamp as a string from libpq
	createdTimeStampChars = PQgetvalue(res, 0, COL_CREATED);

	if (!createdTimeStampChars || strlen(createdTimeStampChars) == 0)
	{
		alog("ns_sso: ERROR: CheckTicket(%s, %s, %s): creation time stamp is empty",
			ticket, hostip, usernick);

		PQclear(res);

		return TICKET_ERROR;
	}

	// Next get the nickname the ticket is for
	nicknameTicketIsFor = PQgetvalue(res, 0, COL_NICKNAME);

	if (!nicknameTicketIsFor || strlen(nicknameTicketIsFor) == 0)
	{
		alog("ns_sso: ERROR: CheckTicket(%s, %s, %s): nickname ticket is for is empty",
			ticket, hostip, usernick);

		PQclear(res);

		return TICKET_ERROR;
	}

	// Check whether the nickname the ticket is for matches the user's nickname
	if (stricmp(nicknameTicketIsFor, usernick) != 0)
	{
		alog("ns_sso: FAIL: CheckTicket(%s, %s, %s): A ticket exists, but it is "
			"not for this nickname (expected: %s, got: %s).",
			ticket, hostip, usernick, nicknameTicketIsFor, usernick);

		PQclear(res);

		return TICKET_BAD;
	}
	
	// Now check if the timestamp is in range
	createdTimeStamp = atoi(createdTimeStampChars);

	timeNow = (int)time(NULL);

	if ((timeNow - createdTimeStamp) > TICKET_TTL)
	{
		alog("ns_sso: FAIL: CheckTicket(%s, %s, %s): Ticket expired (%d seconds is " 
			"older than TTL of %d seconds)", ticket, hostip, usernick,
			(timeNow - createdTimeStamp), TICKET_TTL);

		PQclear(res);

		return TICKET_ERROR;
	}

	alog("ns_sso: SUCCESS: CheckTicket(%s, %s, %s): The ticket seems valid and has " 
		"a remaining TTL of %d seconds", ticket, hostip, usernick,
		(timeNow - createdTimeStamp));

	PQclear(res);

	// Finally, mark the ticket as used

	res = PQexecParams(dbConn,
	   "UPDATE \"tickets\" SET \"Used\" = true WHERE \"Ticket\" = $1",
	   1, NULL, paramValues, NULL, NULL, 0);

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		alog("ns_sso: ERROR: CheckTicket(%s, %s, %s): Unable to set ticket to "
			"used: %s", ticket, hostip, usernick, PQerrorMessage(dbConn));

		PQclear(res);

		return TICKET_ERROR;
	}

	alog("ns_sso: SUCCESS: CheckTicket(%s, %s, %s): The ticket has been marked as "
		"used in the database.", ticket, hostip, usernick);

	PQclear(res);

	return TICKET_OK;
}

int ticketIdentifyCallback(User *u)
{
	// Most of this code is shoplifted from ns_identify.c and converted into
	// our nicer code style that gets rid of stupid brackets, uses goto because
	// goto is never harmful and Dijkstra is a moron, and uses tabs instead
	// of spaces.

	char *ticket = strtok(NULL, " ");

	NickAlias *na;
	NickRequest *nr;

	int res;
	char tsbuf[16];
	char modes[512];
	int len;

	if (!ticket)
	{
		moduleNoticeLang(s_NickServ, u, LNG_TICKETIDENTIFY_SYNTAX);

		alog("ns_sso: NOTICE: %s!%s@%s missing ticket for TICKETIDENTIFY command.",
			u->nick, u->username, u->host);

		goto done;
	}

	if (!(na = u->na))
	{
		if ((nr = findrequestnick(u->nick)))
			notice_lang(s_NickServ, u, NICK_IS_PREREG);
		else
			notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);

		goto done;
	}
	
	if (na->status & NS_VERBOTEN)
	{
		notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, na->nick);
		goto done;
	}
	
	if (na->nc->flags & NI_SUSPENDED)
	{
		notice_lang(s_NickServ, u, NICK_X_SUSPENDED, na->nick);
		goto done;
	}
	
	if (nick_identified(u))
	{
		notice_lang(s_NickServ, u, NICK_ALREADY_IDENTIFIED);
		goto done;
	}
	
	res = checkTicket(ticket, u->hostip, u->nick);

	if (res != TICKET_OK)
	{
		switch (res)
		{
			case TICKET_BAD: // Ticket not found, wrong IP, etc...
				alog("ns_sso: NOTICE: Failed TICKETIDENTIFY for %s!%s@%s",
					u->nick, u->username, u->host);

				notice_lang(s_NickServ, u, PASSWORD_INCORRECT);
				bad_password(u);

				goto done;

			default:
			case TICKET_ERROR: // checkTicket could not do its job
				alog("ns_sso: NOTICE: Error during TICKETIDENTIFY for %s!%s@%s: "
					"checkTicket returned %d", u->nick, u->username, u->host, res);

				notice_lang(s_NickServ, u, NICK_IDENTIFY_FAILED);

				goto done;
		}
	}

	// Go go gadget shoplifter :|

	if (!(na->status & NS_IDENTIFIED) && !(na->status & NS_RECOGNIZED))
	{
		if (na->last_usermask)
			free(na->last_usermask);
		
		na->last_usermask = scalloc(strlen(common_get_vident(u)) + strlen(common_get_vhost(u)) + 2, 1);
		sprintf(na->last_usermask, "%s@%s", common_get_vident(u), common_get_vhost(u));
		
		if (na->last_realname)
			free(na->last_realname);
		
		na->last_realname = sstrdup(u->realname);
	}

	na->status |= NS_IDENTIFIED;
	na->last_seen = time(NULL);
	snprintf(tsbuf, sizeof(tsbuf), "%lu", (unsigned long int) u->timestamp);

	if (ircd->modeonreg)
	{
		len = strlen(ircd->modeonreg);
		strncpy(modes,ircd->modeonreg,512);

		if(ircd->rootmodeonid && is_services_root(u))
			strncat(modes,ircd->rootmodeonid,512-len);
		else if(ircd->adminmodeonid && is_services_admin(u))
			strncat(modes,ircd->adminmodeonid,512-len);
		else if(ircd->opermodeonid && is_services_oper(u))
			strncat(modes,ircd->opermodeonid,512-len);

		if (ircd->tsonmode)
			common_svsmode(u, modes, tsbuf);
		else
			common_svsmode(u, modes, "");
	}
	
	send_event(EVENT_NICK_IDENTIFY, 1, u->nick);
	
	alog("ns_sso: NOTICE: %s!%s@%s ticket-identified for nick %s", u->nick, u->username, u->host, u->nick);
	notice_lang(s_NickServ, u, NICK_IDENTIFY_SUCCEEDED);
	
	if (ircd->vhost)
		do_on_id(u);

	if (NSModeOnID)
		do_setmodes(u);

	if (!(na->status & NS_RECOGNIZED))
		check_memos(u);

	/* Enable nick tracking if enabled */
	if (NSNickTracking)
		nsStartNickTracking(u);

	/* Clear any timers */
	if (na->nc->flags & NI_KILLPROTECT)
		del_ns_timeout(na, TO_COLLIDE);

done:
	return MOD_STOP; // We are the only ones who may handle this
}

int nsRegisterCallback(User *u)
{
	moduleNoticeLang(s_NickServ, u, LNG_SIGN_UP_AT_SITE, BASE_URL);
	return MOD_STOP;
}

int nsDropCallback(User *u)
{
	moduleNoticeLang(s_NickServ, u, LNG_DROP_AT_SITE, BASE_URL);
	return MOD_STOP;
}

int nsSetCallback(User *u)
{
	char *tmp = strdup(moduleGetLastBuffer());
	char *cmd = myStrGetToken(tmp, ' ', 0);

	NickAlias *na = u->na;

	if (!cmd || !na)
	{
cont:
		free(cmd);
		free(tmp);

		return MOD_CONT; // Let Anope complain to the user...
	}

	if (stricmp(cmd, "EMAIL") == 0)
	{
		free(cmd);
		free(tmp);

		moduleNoticeLang(s_NickServ, u, LNG_CHANGE_EMAIL_AT_SITE, BASE_URL);
		return MOD_STOP;
	}

	goto cont;
}

int ticketIdentifyHelpRequest(User *u)
{
	moduleNoticeLang(s_NickServ, u, LNG_TICKETIDENTIFY_HELP, SITE_NAME);
	return MOD_CONT;
}

void createAndRegisterLanguageConstants(void)
{
	// Originally we just used English strings in the code. But we're
	// trying to become a grown up Anope module now. At least they make
	// this task relatively easy. :x

	// I only speak fluent English and German. If you speak more, contribute!

	char *langtable_en_us[] = {
		/* LNG_TICKETIDENTIFY_HELP */
		"Syntax: TICKETIDENTIFY ticket\n"
			"\n"
			"Used internally by %s to automatically tell NickServ\n"
			"that you are really the owner of this nick. This is done\n"
			"using a one-time password (a \"ticket\") which expires\n"
			"quickly if left unused, or directly after being used.\n"
			"This is essentially a form of Single Sign-On. If you are\n"
			"interested in how this works exactly, ask us for details.",
		/* LNG_CHANGE_EMAIL_AT_SITE */
		"Please change your E-Mail address at %s instead.",
		/* LNG_SIGN_UP_AT_SITE */
		"Please sign up for an account at %s instead.",
		/* LNG_DROP_AT_SITE */
		"Please close your account at %s instead.",
		/* LNG_TICKETIDENTIFY_SYNTAX */
		"Syntax: TICKETIDENTIFY TICKET\n"
			"/msg NickServ HELP TICKETIDENTIFY for more information.",
		/* LNG_CHANGING_YOUR_NICK */
		"Your nickname is being changed to as part of a username change\n"
			"on %s. Remember to reconfigure your client if you have\n"
			"set it to automatically identify you with NickServ.",
		/* LNG_TAKEN_OWNERSHIP_OF_NICK */
		"Someone else has taken ownership of this nickname because of a\n"
			"username change on %s. Signed up members have preference\n",
			"over anonymous users. We apologize for the inconvenience.\n",
			"Your nickname is now being changed to %s",
		/* LNG_DROPPED_YOUR_NICK */
		"Your nickname has been dropped because you have closed your\n"
			"account at %s."
	};

	char *langtable_de[] = {
		/* LNG_TICKETIDENTIFY_HELP */
		"Syntax: TICKETIDENTIFY ticket\n"
			"\n"
			"Wird intern von %s benutzt, um NickServ zu sagen, dass du\n"
			"wirklich der Besitzer dieses Nicknamens bist. Dies wird mit\n"
			"Hilfe eines Einmalkennworts (einem sog. \"Ticket\") getan,\n"
			"welches bei Nichtverwendung oder sofort nach Gebrauch ungültig\n"
			"wird. Im Wesentlichen handelt es sich hierbei um eine Form von\n"
			"Single Sign-On. Wenn du an der genauen Funktionsweise\n"
			"interessiert bist, frag uns doch einfach.",
		/* LNG_CHANGE_EMAIL_AT_SITE */
		"Bitte besuche %s um deine E-Mail-Adresse zu ändern.",
		/* LNG_SIGN_UP_AT_SITE */
		"Bitte registriere dich stattdessen bei %s.",
		/* LNG_DROP_AT_SITE */
		"Bitte besuche %s um dein Konto zu schließen.",
		/* LNG_TICKETIDENTIFY_SYNTAX */
		"Syntax: TICKETIDENTIFY TICKET\n"
			"/msg NickServ HELP TICKETIDENTIFY für weitere Informationen.",
		/* LNG_CHANGING_YOUR_NICK */
		"Dein Nickname wird jetzt auf Grund einer Benutzernamensänderung\n"
			"bei %s geändert. Vergiss nicht deinen Client neu zu konfigurieren,\n"
			"falls du ihn so eingestellt hast, dass er sich automatisch bei\n"
			"NickServ identifiziert.",
		/* LNG_TAKEN_OWNERSHIP_OF_NICK */
		"Jemand anders hat den Besitz dieses Nicknamens übernommen, weil es\n"
			"auf %s eine Benutzernamensänderung gab. Registrierte Benutzer\n",
			"haben Vorrang vor anonymen Benutzern. Wir entschuldigen uns für die\n"
			"Unannehmlichkeit.\n",
			"Dein Nickname wurde geändert in %s",
		/* LNG_DROPPED_YOUR_NICK */
		"Die Registrierung deines Nicknamens wurde gelöscht, da Du dein Konto\n"
			"bei %s geschlossen hast."
	};

	moduleInsertLanguage(LANG_EN_US,	LNG_NO_CONSTANTS,	langtable_en_us);
	moduleInsertLanguage(LANG_DE,		LNG_NO_CONSTANTS,	langtable_de);
}

int loadSettings(void)
{
	Directive confvalues[][1] = 
	{
		{{"SSOConnectionString",	{{PARAM_STRING,	PARAM_RELOAD,	&CONN_STR}}}},
		{{"SSOCallbackTimeout",		{{PARAM_INT,	PARAM_RELOAD,	&CALLBACK_TIMEOUT}}}},
		{{"SSOSiteName",			{{PARAM_STRING,	PARAM_RELOAD,	&SITE_NAME}}}},
		{{"SSOBaseUrl",				{{PARAM_STRING,	PARAM_RELOAD,	&BASE_URL}}}},
		{{"SSOTicketOnly",			{{PARAM_SET,	PARAM_RELOAD,	&TICKETIDENTIFY_ONLY}}}},
		{{"SSOTicketTTL",			{{PARAM_INT,	PARAM_RELOAD,	&TICKET_TTL}}}}
	};

	// Set sentinel values...
	CALLBACK_TIMEOUT = TICKET_TTL = -1;

	// Load settings
	{
		int i;

		for (i = 0; i < 6; i++)
			moduleGetConfigDirective(confvalues[i]);
	}

	// Prevent disasters
	if (!CONN_STR)
	{
		alog("ns_sso: ERROR: SSOConnectionString is not set");
		return SETTINGS_BAD;
	}

	if (!CALLBACK_TIMEOUT || CALLBACK_TIMEOUT < 10)
	{
		alog("ns_sso: ERROR: SSOCallbackTimeout must be >= 10 but current value is: %d", CALLBACK_TIMEOUT);
		return SETTINGS_BAD;
	}

	if (!SITE_NAME)
	{
		alog("ns_sso: ERROR: SSOSiteName is not set");
		return SETTINGS_BAD;
	}

	if (!BASE_URL)
	{
		alog("ns_sso: ERROR: SSOBaseUrl is not set");
		return SETTINGS_BAD;
	}

	if (!BASE_URL)
	{
		alog("ns_sso: ERROR: SSOBaseUrl is not set");
		return SETTINGS_BAD;
	}

	if (!TICKET_TTL || TICKET_TTL < 5)
	{
		alog("ns_sso: ERROR: SSOTicketTTL must be >= 5 but current value is: %d", CALLBACK_TIMEOUT);
		return SETTINGS_BAD;
	}

	return SETTINGS_OK;
}