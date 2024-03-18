#include "gseen.h"

CommandReturn CommandSEEN::Execute(User *u, std::vector<ci::string> &params)
{
	mysqlpp::StoreQueryResult res;
	Channel *c = findchan(params[0].c_str());
	Channel *targetchan;
	ci::string target = params[1];
	std::string onlinestatus, timebuf;
	struct tm *lastbuf;
	int type, clang;
	long int last;
	size_t tbuf, days, hours, minutes, seconds = 0;
	char timebuf2[BUFSIZE];
	User *u2;

	if (!c)
		return MOD_CONT; // invalid channel (should never happen)
	if (c && !c->ci)
		return MOD_CONT;  // channel is not registered 

	if (target.empty())
		return MOD_CONT; // print some error message?

	if (target.size() > NICKMAX)
	{
		lng.BotSpeakLang(c->ci, "NICKTOOLONG", NICKMAX);
		return MOD_CONT;
	}

	if (target.compare(c->ci->bi->nick) == 0)
	{
		lng.BotSpeakLang(c->ci, "GS_BOT", u->nick);
		return MOD_CONT;
	}

	clang = lng.GetLang(c); // now get the Channel Language. All messages are generated in this language now.

	if ((u2 = finduser(target.c_str())))
		onlinestatus = lng.GetLangString(clang, "GS_ONLINE");
	else
		onlinestatus = lng.GetLangString(clang, "GS_OFFLINE", target.c_str());

	query.reset();
	query << "SELECT * FROM " << DBNAME << " WHERE nick=%0q;";
	query.parse();
	if (debug)
		alog("debug: %s", query.str(target.c_str()).c_str());
	if ((res = query.store(target.c_str())) && (res.num_rows() > 0))
	{
		type = atoi(res[0]["type"].c_str());
		// generate the string for last seen time 
		last = atol(res[0]["last"].c_str());
		tbuf = time(NULL) - last;
		days = (tbuf / 86400);
		hours = (tbuf / 3600) % 24;
		minutes = (tbuf / 60) % 60;
		seconds = (tbuf) % 60;
		lastbuf = localtime(&last);
		strftime_lang(timebuf2, BUFSIZE, u, STRFTIME_DATE_TIME_FORMAT, lastbuf);
		if (days)
			timebuf = lng.GetLangString(clang, "GS_TIME_FULL", days, hours, minutes);
		else if (hours)
			timebuf = lng.GetLangString(clang, "GS_TIME_HOURS", hours, minutes);
		else if (minutes)
			timebuf = lng.GetLangString(clang, "GS_TIME_MINUTES", minutes);
		else 
			timebuf = lng.GetLangString(clang, "GS_TIME_SECONDS", seconds);
		// now generate the output messages
		if (type == 1)  // target was last seen connecting to the network
		{
			lng.BotSpeakLang(c->ci, "GS_CONNECT", target.c_str(), res[0]["vhost"].c_str(), 
						timebuf.c_str(), timebuf2, onlinestatus.c_str());
		}
		else if (type == 2) // target was last seen changing his nick to newnick
		{
			// now lets see if newnick is still online
			if ((u2 = finduser(res[0]["newnick"].c_str())))
				onlinestatus = lng.GetLangString(clang, "GS_ONLINE_AS", u2->nick);
			else
				onlinestatus = lng.GetLangString(clang, "GS_OFFLINE", res[0]["newnick"].c_str());
			lng.BotSpeakLang(c->ci, "GS_NICKCHANGE_TO", target.c_str(), res[0]["vhost"].c_str(),
					res[0]["newnick"].c_str(), timebuf.c_str(), onlinestatus.c_str());
		}
		else if (type == 3) // target was last seen changing nick from newnick
		{
			lng.BotSpeakLang(c->ci, "GS_NICKCHANGE_FROM", target.c_str(), res[0]["vhost"].c_str(),
					res[0]["newnick"].c_str(), target.c_str(), timebuf.c_str(), onlinestatus.c_str());
		}
		else if (type == 4) // target was last seen joining a channel
		{
			targetchan = findchan(res[0]["chan"].c_str());
			if (targetchan && (c != targetchan) && (targetchan->mode & anope_get_secret_mode()))
			{
				lng.BotSpeakLang(c->ci, "GS_JOIN_SECRET", target.c_str(), res[0]["vhost"].c_str(),
					timebuf.c_str(), onlinestatus.c_str());
			}
			else
			{
				lng.BotSpeakLang(c->ci, "GS_JOIN", target.c_str(), res[0]["vhost"].c_str(),
					res[0]["chan"].c_str(), timebuf.c_str(), onlinestatus.c_str());
			}
		}
		else if (type == 5) // target was last seen parting a channel
		{
			targetchan = findchan(res[0]["chan"].c_str());
			if (targetchan && (c != targetchan) && (targetchan->mode & anope_get_secret_mode()))
			{
				lng.BotSpeakLang(c->ci, "GS_PART_SECRET", target.c_str(), res[0]["vhost"].c_str(),
					timebuf.c_str(), onlinestatus.c_str());
			}
			else
			{
				lng.BotSpeakLang(c->ci, "GS_PART", target.c_str(), res[0]["vhost"].c_str(),
					res[0]["chan"].c_str(), res[0]["msg"].c_str(), timebuf.c_str(), onlinestatus.c_str());
			}
		}
		else if (type == 6) // target was last seen quitting the network
		{
			lng.BotSpeakLang(c->ci, "GS_QUIT", target.c_str(), res[0]["vhost"].c_str(), 
					res[0]["msg"].c_str(), timebuf.c_str(), onlinestatus.c_str());
		}
		else if (type == 7) // target was last seen when kicked out of a chan
		{
			targetchan = findchan(res[0]["chan"].c_str());
			if (targetchan && (c != targetchan) && (targetchan->mode & anope_get_secret_mode()))
			{
				lng.BotSpeakLang(c->ci, "GS_KICK_SECRET", target.c_str(), res[0]["vhost"].c_str(),
					timebuf.c_str(), onlinestatus.c_str());
			}
			else
			{
				lng.BotSpeakLang(c->ci, "GS_KICK", target.c_str(), res[0]["vhost"].c_str(),
					res[0]["chan"].c_str(), res[0]["msg"].c_str(), timebuf.c_str(), onlinestatus.c_str());
			}
		}
		else
		{
			lng.BotSpeakLang(c->ci, "GS_FAILED");
		}
	}
	else
	{	// target not found
		lng.BotSpeakLang(c->ci, "GS_NOT_SEEN", target.c_str());
	}
	return MOD_CONT;
}


