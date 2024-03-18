/**
 * Provides the info command. - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 24/12/2008
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
 * Last Updated   : 24/12/2008
 *
 **/

#ifdef ENABLE_INFO
#include "info.h"

/**
 * The info command.
 **/
int do_info(User *u, Channel *c, char *param) {
	ChannelInfo *ci = c->ci;
	char buf[BUFSIZE], *end;
	struct tm *tm;
	int need_comma = 0;
	const char *commastr = getstring(u->na, COMMA_SPACE);
	int is_servadmin = is_services_admin(u);
	int show_all = 0;
	time_t expt;

	if (ci->flags & CI_VERBOTEN) {
		if (is_oper(u) && ci->forbidby)
			notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN_OPER, ci->name, ci->forbidby, (ci->forbidreason ? ci->
			forbidreason : getstring(u->na, NO_REASON)));
		else
			notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN, ci->name);
	} else if (!ci->founder) {
		/* Paranoia... this shouldn't be able to happen */
		delchan(ci);
		notice_lang(ci->bi->nick, u, CHAN_X_NOT_REGISTERED, ci->name);
	} else {
		/* Should we show all fields? Only for sadmins and identified users */
		if (param && stricmp(param, "ALL") == 0 && (check_access(u, ci, CA_INFO) || is_servadmin))
			show_all = 1;

		notice_lang(ci->bi->nick, u, CHAN_INFO_HEADER, ci->name);
		notice_lang(ci->bi->nick, u, CHAN_INFO_NO_FOUNDER, ci->founder->display);

		if (show_all && ci->successor)
			notice_lang(ci->bi->nick, u, CHAN_INFO_NO_SUCCESSOR, ci->successor->display);

		notice_lang(ci->bi->nick, u, CHAN_INFO_DESCRIPTION, ci->desc);
		tm = localtime(&ci->time_registered);
		strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
		notice_lang(ci->bi->nick, u, CHAN_INFO_TIME_REGGED, buf);
		tm = localtime(&ci->last_used);
		strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
		notice_lang(ci->bi->nick, u, CHAN_INFO_LAST_USED, buf);
		if (ci->last_topic && (show_all || (!(ci->mlock_on & anope_get_secret_mode()) &&
		(!ci->c || !(ci->c->mode & anope_get_secret_mode()))))) {
			notice_lang(ci->bi->nick, u, CHAN_INFO_LAST_TOPIC, ci->last_topic);
			notice_lang(ci->bi->nick, u, CHAN_INFO_TOPIC_SET_BY, ci->last_topic_setter);
		}

		if (ci->entry_message && show_all)
			notice_lang(ci->bi->nick, u, CHAN_INFO_ENTRYMSG, ci->entry_message);
		if (ci->url)
			notice_lang(ci->bi->nick, u, CHAN_INFO_URL, ci->url);
		if (ci->email)
			notice_lang(ci->bi->nick, u, CHAN_INFO_EMAIL, ci->email);

		if (show_all) {
			notice_lang(ci->bi->nick, u, CHAN_INFO_BANTYPE, ci->bantype);

			end = buf;
			*end = 0;
			if (ci->flags & CI_KEEPTOPIC) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s", getstring(u->na, CHAN_INFO_OPT_KEEPTOPIC));
				need_comma = 1;
			}
			if (ci->flags & CI_OPNOTICE) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_OPNOTICE));
				need_comma = 1;
			}
			if (ci->flags & CI_PEACE) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_PEACE));
				need_comma = 1;
			}
			if (ci->flags & CI_PRIVATE) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_PRIVATE));
				need_comma = 1;
			}
			if (ci->flags & CI_RESTRICTED) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_RESTRICTED));
				need_comma = 1;
			}
			if (ci->flags & CI_SECURE) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_SECURE));
				need_comma = 1;
			}
			if (ci->flags & CI_SECUREOPS) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_SECUREOPS));
				need_comma = 1;
			}
			if (ci->flags & CI_SECUREFOUNDER) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_SECUREFOUNDER));
				need_comma = 1;
			}
			if ((ci->flags & CI_SIGNKICK) || (ci->flags & CI_SIGNKICK_LEVEL)) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_SIGNKICK));
				need_comma = 1;
			}
			if (ci->flags & CI_TOPICLOCK) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_TOPICLOCK));
				need_comma = 1;
			}
			if (ci->flags & CI_XOP) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_XOP));
				need_comma = 1;
			}
			notice_lang(ci->bi->nick, u, CHAN_INFO_OPTIONS, *buf ? buf : getstring(u->na, CHAN_INFO_OPT_NONE));
			notice_lang(ci->bi->nick, u, CHAN_INFO_MODE_LOCK, get_mlock_modes(ci, 1));

		}
		if (show_all) {
			if (ci->flags & CI_NO_EXPIRE) {
				notice_lang(ci->bi->nick, u, CHAN_INFO_NO_EXPIRE);
			} else {
				if (is_servadmin) {
					expt = ci->last_used + CSExpire;
					tm = localtime(&expt);
					strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
					notice_lang(ci->bi->nick, u, CHAN_INFO_EXPIRE, buf);
				}
			}
		}
		if (ci->flags & CI_SUSPENDED) {
			notice_lang(ci->bi->nick, u, CHAN_X_SUSPENDED, ci->forbidby,
			(ci->forbidreason ? ci-> forbidreason : getstring(u->na, NO_REASON)));
		}
	}
	return MOD_CONT;
}
#endif

/* EOF */
