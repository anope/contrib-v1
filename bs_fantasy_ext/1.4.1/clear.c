/**
 * The ChanServ CLEAR command. - Source
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
 * Last Updated   : 03/09/2012
 *
 **/

#ifdef ENABLE_CLEAR
#include "clear.h"

int do_clear(User *u, Channel *c, int type) {
/* 1= Modes
 * 2= Bans
 * 3= Excepts
 * 4= Invites
 * 5= Ops
 * 6= Hops
 * 7= Voices
 * 8= Users */

	/* Clear Channel Modes */
	if (type == 1) {
		char *argv[2];

		notice(c->ci->bi->nick, c->name, "CLEAR MODES command from %s", u->nick);

		if (c->mode) {
			/* Clear modes the bulk of the modes */
			anope_cmd_mode(c->ci->bi->nick, c->name, "%s", ircd->modestoremove);
			argv[0] = ircd->modestoremove;
			chan_set_modes(c->ci->bi->nick, c, 1, argv, 0);

			if (c->key) {
				anope_cmd_mode(c->ci->bi->nick, c->name, "-k %s", c->key);
				argv[0] = "-k";
				argv[1] = c->key;
				chan_set_modes(c->ci->bi->nick, c, 2, argv, 0);
			}
			if (ircd->Lmode && c->redirect) {
				anope_cmd_mode(c->ci->bi->nick, c->name, "-L %s", c->redirect);
				argv[0] = "-L";
				argv[1] = c->redirect;
				chan_set_modes(c->ci->bi->nick, c, 2, argv, 0);
			}
			if (ircd->fmode && c->flood) {
				if (flood_mode_char_remove) {
					anope_cmd_mode(c->ci->bi->nick, c->name, "%s %s", flood_mode_char_remove, c->flood);
					argv[0] = flood_mode_char_remove;
					argv[1] = c->flood;
					chan_set_modes(c->ci->bi->nick, c, 2, argv, 0);
				}
			}
			check_modes(c);
		}

		alog("%s: %s!%s@%s cleared modes on %s",
				c->ci->bi->nick, u->nick, u->username, u->host, c->ci->name);

	/* Clear Channel Bans */
	} else if (type == 2) {
		char *av[2];
		Entry *ban, *next;

		notice(c->ci->bi->nick, c->name, "CLEAR BANS command from %s", u->nick);
		if (c->bans && c->bans->count) {
			av[0] = "-b";
			if (ircd->svsmode_ucmode) {
				anope_cmd_svsmode_chan(c->name, "-b", NULL);
				for (ban = c->bans->entries; ban; ban = next) {
					next = ban->next;
					av[1] = ban->mask;
					chan_set_modes(c->ci->bi->nick, c, 2, av, 0);
				}
			} else {
				for (ban = c->bans->entries; ban; ban = next) {
					next = ban->next;
					av[1] = ban->mask;
					anope_cmd_mode(c->ci->bi->nick, c->name, "%s %s", av[0], av[1]);
					chan_set_modes(c->ci->bi->nick, c, 2, av, 0);
				}
			}
		}

		alog("%s: %s!%s@%s cleared bans on %s",
				c->ci->bi->nick, u->nick, u->username, u->host, c->ci->name);

	/* Clear Channel Excepts */
	} else if (ircd->except && type == 3) {
		char *av[2];
		Entry *except, *next;

		notice(c->ci->bi->nick, c->name, "CLEAR EXCEPTS command from %s", u->nick);
		if (c->excepts && c->excepts->count) {
			av[0] = "-e";
			if (ircd->svsmode_ucmode) {
				anope_cmd_svsmode_chan(c->name, "-e", NULL);
				for (except = c->excepts->entries; except; except = next) {
					next = except->next;
					av[1] = except->mask;
					chan_set_modes(c->ci->bi->nick, c, 2, av, 0);
				}
			} else {
				for (except = c->excepts->entries; except; except = next) {
					next = except->next;
					av[1] = except->mask;
					anope_cmd_mode(c->ci->bi->nick, c->name, "%s %s", av[0], av[1]);
					chan_set_modes(c->ci->bi->nick, c, 2, av, 0);
				}
			}
		}

		alog("%s: %s!%s@%s cleared excepts on %s",
				c->ci->bi->nick, u->nick, u->username, u->host, c->ci->name);

	} else if (!(ircd->except) && type == 3) {
		noticeLang(c->ci->bi->nick, u, LANG_EXCEPTS_UNSUPPORTED);

	/* Clear Channel Invites */
	} else if (ircd->invitemode && type == 4) {
		char *av[2];
		Entry *invite, *next;

		notice(c->ci->bi->nick, c->name, "CLEAR INVITES command from %s", u->nick);
		if (c->invites && c->invites->count) {
			av[0] = "-I";
			if (ircd->svsmode_ucmode) {
				anope_cmd_svsmode_chan(c->name, "-I", NULL);
				for (invite = c->invites->entries; invite; invite = next) {
					next = invite->next;
					av[1] = invite->mask;
					chan_set_modes(c->ci->bi->nick, c, 2, av, 0);
				}
			} else {
				for (invite = c->invites->entries; invite; invite = next) {
					next = invite->next;
					av[1] = invite->mask;
					anope_cmd_mode(c->ci->bi->nick, c->name, "%s %s", av[0], av[1]);
					chan_set_modes(c->ci->bi->nick, c, 2, av, 0);
				}
			}
		}

		alog("%s: %s!%s@%s cleared invites on %s",
				c->ci->bi->nick, u->nick, u->username, u->host, c->ci->name);

	} else if (!(ircd->invitemode) && type == 4) {
		noticeLang(c->ci->bi->nick, u, LANG_INVITEMODE_UNSUPPORTED);

	/* Clear Channel OPs */
	} else if (type == 5) {
		char *av[6];  /* The max we have to hold: chan, ts, modes(max3), nick, nick, nick */
		int ac, isop, isadmin, isown, count, i;
		char buf[BUFSIZE], tmp[BUFSIZE], tmp2[BUFSIZE];
		struct c_userlist *cu;

		notice(c->ci->bi->nick, c->name, "CLEAR OPS command from %s", u->nick);

		if (ircd->svsmode_ucmode) {
			anope_cmd_svsmode_chan(c->name, "-o", NULL);
			if (ircd->owner) {
				anope_cmd_svsmode_chan(c->name, ircd->ownerunset, NULL);
			}
			if (ircd->protect || ircd->admin) {
				anope_cmd_svsmode_chan(c->name, ircd->adminunset, NULL);
			}
		} else {
			for (cu = c->users; cu; cu = cu->next) {
				isop = chan_has_user_status(c, cu->user, CUS_OP);
				isadmin = chan_has_user_status(c, cu->user, CUS_PROTECT);
				isown = chan_has_user_status(c, cu->user, CUS_OWNER);
				count = (isop ? 1 : 0) + (isadmin ? 1 : 0) + (isown ? 1 : 0);

				if (!isop && !isadmin && !isown)
					continue;

				snprintf(tmp, BUFSIZE, "-%s%s%s", (isop ? "o" : ""), (isadmin ? 
						ircd->adminunset+1 : ""), (isown ? ircd->ownerunset+1 : ""));
				/* We need to send the IRCd a nick for every mode.. - Viper */
				snprintf(tmp2, BUFSIZE, "%s %s %s", (isop ? GET_USER(cu->user) : ""),
						(isadmin ? GET_USER(cu->user) : ""), (isown ? GET_USER(cu->user) : ""));

				av[0] = c->name;   /* do_cmode can modify av[0].. */
				if (ircdcap->tsmode) {
					snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
					av[1] = buf;
					av[2] = tmp;
					/* We have to give as much nicks as modes.. - Viper */
					for (i = 0; i < count; i++)
						av[i+3] = GET_USER(cu->user);
					ac = 3 + i;
					anope_cmd_mode(c->ci->bi->nick, av[0], "%s %s", av[2], tmp2);
				} else {
					av[1] = tmp;
					/* We have to give as much nicks as modes.. - Viper */
					for (i = 0; i < count; i++)
						av[i+2] = GET_USER(cu->user);
					ac = 2 + i;
					anope_cmd_mode(c->ci->bi->nick, av[0], "%s %s", av[1], tmp2);
				}
				do_cmode(c->ci->bi->nick, ac, av);
			}
		}

		alog("%s: %s!%s@%s cleared ops on %s",
				c->ci->bi->nick, u->nick, u->username, u->host, c->ci->name);

	/* Clear Channel HOPs */
	} else if (ircd->halfop && type == 6) {
		char *av[4];
		struct c_userlist *cu;
		char buf[BUFSIZE];
		int ac;

		notice(c->ci->bi->nick, c->name, "CLEAR HOPS command from %s", u->nick);

		if (ircd->svsmode_ucmode) {
			anope_cmd_svsmode_chan(c->name, "-h", NULL);

		} else {
			if (ircdcap->tsmode)
				av[2] = "-h";
			else
				av[1] = "-h";

			for (cu = c->users; cu; cu = cu->next) {
				if (!chan_has_user_status(c, cu->user, CUS_HALFOP))
					continue;

				av[0] = c->name;   /* do_cmode can modify av[0].. */
				if (ircdcap->tsmode) {
					snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
					av[1] = buf;
					av[3] = GET_USER(cu->user);
					ac = 4;
				} else {
					av[2] = GET_USER(cu->user);
					ac = 3;
				}

				anope_cmd_mode(c->ci->bi->nick, c->name, "-h %s", cu->user->nick);
				do_cmode(c->ci->bi->nick, ac, av);
			}
		}

		alog("%s: %s!%s@%s cleared halfops on %s",
				c->ci->bi->nick, u->nick, u->username, u->host, c->ci->name);

	} else if (!(ircd->halfop) && type == 6) {
		noticeLang(c->ci->bi->nick, u, LANG_HOPS_UNSUPPORTED);

	/* Clear Channel Voices */
    } else if (type == 7) {
		char *av[4];
		struct c_userlist *cu;
		char buf[BUFSIZE];
		int ac;

		notice(c->ci->bi->nick, c->name, "CLEAR VOICES command from %s", u->nick);

		if (ircd->svsmode_ucmode) {
			anope_cmd_svsmode_chan(c->name, "-v", NULL);

		} else {
			if (ircdcap->tsmode)
				av[2] = "-v";
			else
				av[1] = "-v";

			for (cu = c->users; cu; cu = cu->next) {
				if (!chan_has_user_status(c, cu->user, CUS_VOICE))
					continue;

				av[0] = c->name;   /* do_cmode can modify av[0].. */
				if (ircdcap->tsmode) {
					snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
					av[1] = buf;
					av[3] = GET_USER(cu->user);
					ac = 4;
				} else {
					av[2] = GET_USER(cu->user);
					ac = 3;
				}

				anope_cmd_mode(c->ci->bi->nick, c->name, "-v %s", cu->user->nick);
				do_cmode(c->ci->bi->nick, ac, av);
			}
		}

		alog("%s: %s!%s@%s cleared voices on %s",
				c->ci->bi->nick, u->nick, u->username, u->host, c->ci->name);

	/* Clear Channel Users */
	} else if (type == 8) {
		char *av[3];
		struct c_userlist *cu, *next;
		char buf[256];

		snprintf(buf, sizeof(buf), "CLEAR USERS command from %s", u->nick);
		notice(c->ci->bi->nick, c->name, buf);

		av[0] = c->name;
		av[2] = buf;
		for (cu = c->users; cu; cu = next) {
			next = cu->next;
			av[1] = GET_USER(cu->user);
			anope_cmd_kick(c->ci->bi->nick, av[0], av[1], av[2]);
			do_kick(c->ci->bi->nick, 3, av);
		}

		alog("%s: %s!%s@%s cleared users on %s",
				c->ci->bi->nick, u->nick, u->username, u->host, c->ci->name);
	}

	/* Should NEVER happen, therefor if it does, stop processing */
	else {
		alog("[bs_fantasy_ext] An error has occured while processing CLEAR !!!");
		return MOD_STOP;
	}

	return MOD_CONT;
}
#endif

/* EOF */
