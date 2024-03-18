/**
 * Methods to modify the BotServ kick settings. - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 21/07/2006
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * Based on the code of Anope by The Anope Dev Team
 *
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated: 28/10/2011
 *
 **/

#ifdef ENABLE_BKICK
#include "bkick.h"

int do_set_kick(User * u, ChannelInfo * ci, char *option, char *value, char *ttb, char *param1, char *param2) {
	if (readonly)
		notice_lang(ci->bi->nick, u, BOT_KICK_DISABLED);

	else if (!option || !value)
		syntax_error(ci->bi->nick, u, "KICK", BOT_KICK_SYNTAX);
	else if (stricmp(value, "ON") && stricmp(value, "OFF"))
		syntax_error(ci->bi->nick, u, "KICK", BOT_KICK_SYNTAX);
	else if (ci->flags & CI_VERBOTEN)
		notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN, ci->name);
	else if (!is_services_admin(u) && !check_access(u, ci, CA_SET))
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	else {
		if (!stricmp(option, "BADWORDS")) {
			if (!stricmp(value, "ON")) {
				if (ttb) {
					errno = 0;
					ci->ttb[TTB_BADWORDS] = strtol(ttb, (char **) NULL, 10);
					/* Only error if errno returns ERANGE or EINVAL or we are less then 0 - TSL */
					if (errno == ERANGE || errno == EINVAL || ci->ttb[TTB_BADWORDS] < 0) {
						/* leaving the debug behind since we might want to know what these are */
						if (debug) {
							alog("debug: errno is %d ERANGE %d EINVAL %d ttb %d", errno, ERANGE, EINVAL, ci->ttb[TTB_BADWORDS]);
						}
						/* reset the value back to 0 - TSL */
						ci->ttb[TTB_BADWORDS] = 0;
						notice_lang(ci->bi->nick, u, BOT_KICK_BAD_TTB, ttb);
						return MOD_CONT;
					}
				} else {
					ci->ttb[TTB_BADWORDS] = 0;
				}
				ci->botflags |= BS_KICK_BADWORDS;
				if (ci->ttb[TTB_BADWORDS]) {
					alog("%s: %s!%s@%s enabled kicking for badwords on %s with time to ban of %d",
							ci->bi->nick, u->nick, u->username, u->host, ci->name, ci->ttb[TTB_BADWORDS]);
					notice_lang(ci->bi->nick, u, BOT_KICK_BADWORDS_ON_BAN, ci->ttb[TTB_BADWORDS]);
				} else {
					alog("%s: %s!%s@%s enabled kicking for badwords on %s",
							ci->bi->nick, u->nick, u->username, u->host, ci->name);
					notice_lang(ci->bi->nick, u, BOT_KICK_BADWORDS_ON);
				}
			} else {
				ci->botflags &= ~BS_KICK_BADWORDS;
				alog("%s: %s!%s@%s disabled kicking for badwords on %s",
						ci->bi->nick, u->nick, u->username, u->host, ci->name);
				notice_lang(ci->bi->nick, u, BOT_KICK_BADWORDS_OFF);
			}
		} else if (!stricmp(option, "BOLDS")) {
			if (!stricmp(value, "ON")) {
				if (ttb) {
					errno = 0;
					ci->ttb[TTB_BOLDS] = strtol(ttb, (char **) NULL, 10);
					if (errno == ERANGE || errno == EINVAL || ci->ttb[TTB_BOLDS] < 0) {
						if (debug) {
							alog("debug: errno is %d ERANGE %d EINVAL %d ttb %d", errno, ERANGE, EINVAL, ci->ttb[TTB_BOLDS]);
						}
						ci->ttb[TTB_BOLDS] = 0;
						notice_lang(ci->bi->nick, u, BOT_KICK_BAD_TTB, ttb);
						return MOD_CONT;
					}
				} else
					ci->ttb[TTB_BOLDS] = 0;
				ci->botflags |= BS_KICK_BOLDS;
				if (ci->ttb[TTB_BOLDS]) {
					alog("%s: %s!%s@%s enabled kicking for bolds on %s with time to ban of %d",
							ci->bi->nick, u->nick, u->username, u->host, ci->name, ci->ttb[TTB_BOLDS]);
					notice_lang(ci->bi->nick, u, BOT_KICK_BOLDS_ON_BAN, ci->ttb[TTB_BOLDS]);
				} else {
					alog("%s: %s!%s@%s enabled kicking for bolds on %s",
							ci->bi->nick, u->nick, u->username, u->host, ci->name);
					notice_lang(ci->bi->nick, u, BOT_KICK_BOLDS_ON);
				}
			} else {
				ci->botflags &= ~BS_KICK_BOLDS;
				alog("%s: %s!%s@%s disabled kicking for bolds on %s",
						ci->bi->nick, u->nick, u->username, u->host, ci->name);
				notice_lang(ci->bi->nick, u, BOT_KICK_BOLDS_OFF);
			}
		} else if (!stricmp(option, "CAPS")) {
			if (!stricmp(value, "ON")) {
				char *min = param1;
				char *percent = param2;

				if (ttb) {
					errno = 0;
					ci->ttb[TTB_CAPS] = strtol(ttb, (char **) NULL, 10);
					if (errno == ERANGE || errno == EINVAL || ci->ttb[TTB_CAPS] < 0) {
						if (debug) {
							alog("debug: errno is %d ERANGE %d EINVAL %d ttb %d", errno, ERANGE, EINVAL, ci->ttb[TTB_CAPS]);
						}
						ci->ttb[TTB_CAPS] = 0;
						notice_lang(ci->bi->nick, u, BOT_KICK_BAD_TTB, ttb);
						return MOD_CONT;
					}
				} else
					ci->ttb[TTB_CAPS] = 0;

				if (!min)
					ci->capsmin = 10;
				else
					ci->capsmin = atol(min);
				if (ci->capsmin < 1)
					ci->capsmin = 10;

				if (!percent)
					ci->capspercent = 25;
				else
					ci->capspercent = atol(percent);
				if (ci->capspercent < 1 || ci->capspercent > 100)
					ci->capspercent = 25;

				ci->botflags |= BS_KICK_CAPS;
				if (ci->ttb[TTB_CAPS]) {
					alog("%s: %s!%s@%s enabled kicking for caps on %s with time to ban of %d",
							ci->bi->nick, u->nick, u->username, u->host, ci->name, ci->ttb[TTB_CAPS]);
					notice_lang(ci->bi->nick, u, BOT_KICK_CAPS_ON_BAN, ci->capsmin, ci->capspercent, ci->ttb[TTB_CAPS]);
				} else {
					alog("%s: %s!%s@%s enabled kicking for caps on %s",
							ci->bi->nick, u->nick, u->username, u->host, ci->name);
					notice_lang(ci->bi->nick, u, BOT_KICK_CAPS_ON, ci->capsmin, ci->capspercent);
				}
			} else {
				ci->botflags &= ~BS_KICK_CAPS;
				alog("%s: %s!%s@%s disabled kicking for caps on %s",
						ci->bi->nick, u->nick, u->username, u->host, ci->name);
				notice_lang(ci->bi->nick, u, BOT_KICK_CAPS_OFF);
			}
		} else if (!stricmp(option, "COLORS")) {
			if (!stricmp(value, "ON")) {
				if (ttb) {
					errno = 0;
					ci->ttb[TTB_COLORS] = strtol(ttb, (char **) NULL, 10);
					if (errno == ERANGE || errno == EINVAL || ci->ttb[TTB_COLORS] < 0) {
						if (debug) {
							alog("debug: errno is %d ERANGE %d EINVAL %d ttb %d", errno, ERANGE, EINVAL, ci->ttb[TTB_COLORS]);
						}
						ci->ttb[TTB_COLORS] = 0;
						notice_lang(ci->bi->nick, u, BOT_KICK_BAD_TTB, ttb);
						return MOD_CONT;
					}
				} else
					ci->ttb[TTB_COLORS] = 0;
				ci->botflags |= BS_KICK_COLORS;
				if (ci->ttb[TTB_COLORS]) {
					alog("%s: %s!%s@%s enabled kicking for colors on %s with time to ban of %d",
							ci->bi->nick, u->nick, u->username, u->host, ci->name, ci->ttb[TTB_COLORS]);
					notice_lang(ci->bi->nick, u, BOT_KICK_COLORS_ON_BAN, ci->ttb[TTB_COLORS]);
				} else {
					alog("%s: %s!%s@%s enabled kicking for colors on %s",
							ci->bi->nick, u->nick, u->username, u->host, ci->name);
					notice_lang(ci->bi->nick, u, BOT_KICK_COLORS_ON);
				}
			} else {
				ci->botflags &= ~BS_KICK_COLORS;
				alog("%s: %s!%s@%s disabled kicking for colors on %s",
						ci->bi->nick, u->nick, u->username, u->host, ci->name);
				notice_lang(s_BotServ, u, BOT_KICK_COLORS_OFF);
			}
		} else if (!stricmp(option, "FLOOD")) {
			if (!stricmp(value, "ON")) {
				char *lines = param1;
				char *secs = param2;

				if (ttb) {
					errno = 0;
					ci->ttb[TTB_FLOOD] = strtol(ttb, (char **) NULL, 10);
					if (errno == ERANGE || errno == EINVAL || ci->ttb[TTB_FLOOD] < 0) {
						if (debug) {
							alog("debug: errno is %d ERANGE %d EINVAL %d ttb %d", errno, ERANGE, EINVAL, ci->ttb[TTB_FLOOD]);
						}
						ci->ttb[TTB_FLOOD] = 0;
						notice_lang(ci->bi->nick, u, BOT_KICK_BAD_TTB, ttb);
						return MOD_CONT;
					}
				} else
					ci->ttb[TTB_FLOOD] = 0;

				if (!lines)
					ci->floodlines = 6;
				else
					ci->floodlines = atol(lines);
				if (ci->floodlines < 2)
					ci->floodlines = 6;

				if (!secs)
					ci->floodsecs = 10;
				else
					ci->floodsecs = atol(secs);
				if (ci->floodsecs < 1 || ci->floodsecs > BSKeepData)
					ci->floodsecs = 10;

				ci->botflags |= BS_KICK_FLOOD;
				if (ci->ttb[TTB_FLOOD]) {
					alog("%s: %s!%s@%s enabled kicking for flood on %s with time to ban of %d",
							ci->bi->nick, u->nick, u->username, u->host, ci->name, ci->ttb[TTB_FLOOD]);
					notice_lang(ci->bi->nick, u, BOT_KICK_FLOOD_ON_BAN, ci->floodlines, ci->floodsecs, ci->ttb[TTB_FLOOD]);
				} else {
					alog("%s: %s!%s@%s enabled kicking for flood on %s",
							ci->bi->nick, u->nick, u->username, u->host, ci->name);
					notice_lang(ci->bi->nick, u, BOT_KICK_FLOOD_ON, ci->floodlines, ci->floodsecs);
				}
			} else {
				ci->botflags &= ~BS_KICK_FLOOD;
				alog("%s: %s!%s@%s disabled kicking for flood on %s",
						ci->bi->nick, u->nick, u->username, u->host, ci->name);
				notice_lang(ci->bi->nick, u, BOT_KICK_FLOOD_OFF);
			}
		} else if (!stricmp(option, "REPEAT")) {
			if (!stricmp(value, "ON")) {
				char *times = param1;

				if (ttb) {
					errno = 0;
					ci->ttb[TTB_REPEAT] = strtol(ttb, (char **) NULL, 10);
					if (errno == ERANGE || errno == EINVAL || ci->ttb[TTB_REPEAT] < 0) {
						if (debug) {
							alog("debug: errno is %d ERANGE %d EINVAL %d ttb %d", errno, ERANGE, EINVAL, ci->ttb[TTB_REPEAT]);
						}
						ci->ttb[TTB_REPEAT] = 0;
						notice_lang(ci->bi->nick, u, BOT_KICK_BAD_TTB, ttb);
						return MOD_CONT;
					}
				} else
					ci->ttb[TTB_REPEAT] = 0;

				if (!times)
					ci->repeattimes = 3;
				else
					ci->repeattimes = atol(times);
				if (ci->repeattimes < 2)
					ci->repeattimes = 3;

				ci->botflags |= BS_KICK_REPEAT;
				if (ci->ttb[TTB_REPEAT]) {
					alog("%s: %s!%s@%s enabled kicking for repeating on %s with time to ban of %d",
							ci->bi->nick, u->nick, u->username, u->host, ci->name, ci->ttb[TTB_REPEAT]);
					notice_lang(ci->bi->nick, u, BOT_KICK_REPEAT_ON_BAN, ci->repeattimes, ci->ttb[TTB_REPEAT]);
				} else {
					alog("%s: %s!%s@%s enabled kicking for repeating on %s",
							ci->bi->nick, u->nick, u->username, u->host, ci->name);
					notice_lang(ci->bi->nick, u, BOT_KICK_REPEAT_ON, ci->repeattimes);
				}
			} else {
				ci->botflags &= ~BS_KICK_REPEAT;
				alog("%s: %s!%s@%s disabled kicking for repeating on %s",
						ci->bi->nick, u->nick, u->username, u->host, ci->name);
				notice_lang(ci->bi->nick, u, BOT_KICK_REPEAT_OFF);
			}
		} else if (!stricmp(option, "REVERSES")) {
			if (!stricmp(value, "ON")) {
				if (ttb) {
					errno = 0;
					ci->ttb[TTB_REVERSES] = strtol(ttb, (char **) NULL, 10);
					if (errno == ERANGE || errno == EINVAL || ci->ttb[TTB_REVERSES] < 0) {
						if (debug) {
							alog("debug: errno is %d ERANGE %d EINVAL %d ttb %d", errno, ERANGE, EINVAL, ci->ttb[TTB_REVERSES]);
						}
						ci->ttb[TTB_REVERSES] = 0;
						notice_lang(ci->bi->nick, u, BOT_KICK_BAD_TTB, ttb);
						return MOD_CONT;
					}
				} else
					ci->ttb[TTB_REVERSES] = 0;
				ci->botflags |= BS_KICK_REVERSES;
				if (ci->ttb[TTB_REVERSES]) {
					alog("%s: %s!%s@%s enabled kicking for reversess on %s with time to ban of %d",
							ci->bi->nick, u->nick, u->username, u->host, ci->name, ci->ttb[TTB_REVERSES]);
					notice_lang(ci->bi->nick, u, BOT_KICK_REVERSES_ON_BAN, ci->ttb[TTB_REVERSES]);
				} else {
					alog("%s: %s!%s@%s enabled kicking for reverses on %s",
							ci->bi->nick, u->nick, u->username, u->host, ci->name);
					notice_lang(ci->bi->nick, u, BOT_KICK_REVERSES_ON);
				}
			} else {
				ci->botflags &= ~BS_KICK_REVERSES;
				alog("%s: %s!%s@%s disabled kicking for reverses on %s",
						ci->bi->nick, u->nick, u->username, u->host, ci->name);
				notice_lang(ci->bi->nick, u, BOT_KICK_REVERSES_OFF);
			}
		} else if (!stricmp(option, "UNDERLINES")) {
			if (!stricmp(value, "ON")) {
				if (ttb) {
					errno = 0;
					ci->ttb[TTB_UNDERLINES] = strtol(ttb, (char **) NULL, 10);
					if (errno == ERANGE || errno == EINVAL || ci->ttb[TTB_UNDERLINES] < 0) {
						if (debug) {
							alog("debug: errno is %d ERANGE %d EINVAL %d ttb %d", errno, ERANGE, EINVAL, ci->ttb[TTB_UNDERLINES]);
						}
						ci->ttb[TTB_UNDERLINES] = 0;
						notice_lang(ci->bi->nick, u, BOT_KICK_BAD_TTB, ttb);
						return MOD_CONT;
					}
				} else
					ci->ttb[TTB_UNDERLINES] = 0;
				ci->botflags |= BS_KICK_UNDERLINES;
				if (ci->ttb[TTB_UNDERLINES]) {
					alog("%s: %s!%s@%s enabled kicking for underlines on %s with time to ban of %d",
							ci->bi->nick, u->nick, u->username, u->host, ci->name, ci->ttb[TTB_UNDERLINES]);
					notice_lang(ci->bi->nick, u, BOT_KICK_UNDERLINES_ON_BAN, ci->ttb[TTB_UNDERLINES]);
				} else {
					alog("%s: %s!%s@%s enabled kicking for underlines on %s",
							ci->bi->nick, u->nick, u->username, u->host, ci->name);
					notice_lang(ci->bi->nick, u, BOT_KICK_UNDERLINES_ON);
				}
			} else {
				ci->botflags &= ~BS_KICK_UNDERLINES;
				alog("%s: %s!%s@%s disabled kicking for underlines on %s",
						ci->bi->nick, u->nick, u->username, u->host, ci->name);
				notice_lang(ci->bi->nick, u, BOT_KICK_UNDERLINES_OFF);
			}
		} else
			notice_help(ci->bi->nick, u, BOT_KICK_UNKNOWN, option);
	}
	return MOD_CONT;
}
#endif

/* EOF */
