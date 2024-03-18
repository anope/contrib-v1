/* Original Module Author: Adam
 * Updated for 1.9.2 (and beyond?): Laura
 * Data of Original: 14/02/10
 * Date of This Version: 26/12/10
 **********************************************
 * Module used in: Nickserv
 * Syntax: /ns saregister 'nick' 'pass' 'email'
 **********************************************
 * Changes: Updated to work with 1.9.2-p2
*/
#include "module.h"

#define AUTHOR "Laura"
#define VERSION "1.9"
#define INVALID_NICKNAME "This nickname is invalid for use with standard RFC 2812"

class CommandNSSaRegister : public Command
{
    public:
        CommandNSSaRegister() : Command("SAREGISTER", 3, 3, "nickserv/saregister")
        {
        }
        CommandReturn Execute(User *u, const std::vector<ci::string> &params)
        {
            const char *nick = params[0].c_str();
            const char *password = params[1].c_str();
            const char *email = params[2].c_str();
            int nicklen = strlen(nick);
            int prefixlen = strlen(Config.NSGuestNickPrefix);

            NickRequest *nr = NULL;
            NickAlias *na = findnick(nick);

            //Debug Stuff Below
            //notice_lang(Config.s_NickServ, u, nick);
            //notice_lang(Config.s_NickServ, u, password);
            //notice_lang(Config.s_NickServ, u, email);
            //return MOD_CONT;

            std::list<std::pair<std::string, std::string> >::iterator it;
            
            //The tidbits below are used to create a passcode.. remove these if it's not needed.. here for safety.
            int idx, min = 1, max = 62;
	    int chars[] = {
		' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
		'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
		'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
		'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
		'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
	    };
            
            //Run checks to make sure we're not trying to register a nickname that is already registered, forbidden, used in botserv, or suspended
            if (findbot(nick))
            {
                notice_lang(Config.s_NickServ, u, BOT_BOT_ALREADY_EXISTS, nick);     //Copied from Botserv Add.. Find a better one if needed.
                return MOD_STOP;
            }
            if (findnick(nick))
            {
                notice_lang(Config.s_NickServ, u, NICK_ALREADY_REGISTERED, nick);
                return MOD_STOP;
            }
            //Allowed to create nicks at this time?
            if (readonly)
            {
                notice_lang(Config.s_NickServ, u, NICK_REGISTRATION_DISABLED);
                return MOD_STOP;
            }
            //Check for valid nick -- nothing too long, no Guest####### nicks, and nothing banned in ircd.conf
            if (strlen(nick) > Config.NickLen)
            {
                m_privmsg("NickServ", u->nick.c_str(), INVALID_NICKNAME);
                return MOD_STOP;
            }
            if (nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 && stristr(nick, Config.NSGuestNickPrefix) == nick && strspn(nick + prefixlen, "1234567890") == nicklen - prefixlen)
            {
                notice_lang(Config.s_NickServ, u, NICK_CANNOT_BE_REGISTERED, nick);
                return MOD_STOP;
            }
            if (!ircdproto->IsNickValid(nick))
            {
                notice_lang(Config.s_NickServ, u, NICK_X_FORBIDDEN, nick);
                return MOD_STOP;
            }
            if (Config.RestrictOperNicks)
            {
                for (it = Config.Opers.begin(); it != Config.Opers.end(); ++it)
                {
                    std::string temp_nick = it->first;

                    if (stristr(u->nick.c_str(), nick))
                    {
                        notice_lang(Config.s_NickServ, u, NICK_CANNOT_BE_REGISTERED, nick);
                        return MOD_STOP;
                    }
                }
            }
            //Prolly don't need this, but just in case someone wants to try to crash things
            if (isdigit(nick[0]) || nick[0] == '-')
            {
                m_privmsg("NickServ", u->nick.c_str(), INVALID_NICKNAME);
                return MOD_STOP;
            }
            for (unsigned i =0, end = nicklen; i <end && i <Config.NickLen; ++i)
                if (!isvalidnick(nick[i]))
                {
                    m_privmsg("NickServ", u->nick.c_str(), INVALID_NICKNAME);
                    return MOD_STOP;
                }
            //Check for a valid and secure pass
            if (!stricmp(nick, password) || (Config.StrictPasswords && strlen(password) <5))
            {
                notice_lang(Config.s_NickServ, u, MORE_OBSCURE_PASSWORD);
                return MOD_STOP;
            }
            else if (strlen(password) > Config.PassLen)
            {
                notice_lang(Config.s_NickServ, u, PASSWORD_TOO_LONG);
                return MOD_STOP;
            }
            //Check for valid email
            if (email && !MailValidate(email))
            {
                notice_lang(Config.s_NickServ, u, MAIL_X_INVALID, email);
                return MOD_STOP;
            }
            else  //else just for debug, remove later.
            {
                //Stuff to be done when we're sure nick is allowed.
                //Mostly copied from ns_register.cpp (perhaps a better way, can we just call the ns function?)
                //passcode may not be needed, but it's there to make it happy just in case :)
                char passcode[11];
                for (idx =0; idx <9; ++idx)
                    passcode[idx] = chars[1 +static_cast<int>((static_cast<float>(max - min)) * getrandom16() / 65536.0) + min];
                passcode[idx] = '\0';
		nr = new NickRequest(nick);
		nr->passcode = passcode;
                nr->password = password;
		enc_encrypt_in_place(nr->password);
                nr->email = sstrdup(email);
                nr->requested = time(NULL);

                na = new NickAlias(nr->nick, new NickCore(nr->nick));
                na->nc->pass = nr->password;
                na->nc->memos.memomax = Config.MSMaxMemos;
                //taken from force because we're forcing the nick creation
                na->last_usermask = sstrdup("*@*");
                na->last_realname = sstrdup("unknown");
                na->time_registered = na->last_seen = time(NULL);
                na->nc->language = Config.NSDefLanguage;
                na->nc->email = sstrdup(email);

                Alog() << Config.s_NickServ << ": '" << nick << "' registered by " << u->nick << " (email: " << email << nr->email << ")";
                notice_lang(Config.s_NickServ, u, NICK_FORCE_REG, nr->nick);     //Used the "Nickname X confirmed" message to verify that nick was created
                User *user_ = finduser(nr->nick);
                delete nr;
                if (user_) validate_user(user_);
                return MOD_CONT;
            }
//            return MOD_CONT;
        }

        bool OnHelp(User *u, const ci::string &subcommand)
        {
            m_privmsg("NickServ", u->nick.c_str(), " ");
            m_privmsg("NickServ", u->nick.c_str(), "/msg NickServ \2SAREGISTER \37nick\37 \37password\37 \37email\37");
            m_privmsg("NickServ", u->nick.c_str(), " ");
            m_privmsg("NickServ", u->nick.c_str(), "Allows services admins to register other nicks.");
            return true;
        }

        void OnSyntaxError(User *u, const ci::string &subcommand)
        {
            m_privmsg("NickServ", u->nick.c_str(), "Syntax: \2SAREGISTER \37nick\37 \37password\37 \37email\37");
        }
};

class NSSaRegister : public Module
{
    public:
        NSSaRegister(const std::string &modname, const std::string &creator) : Module(modname, creator)
        {
            this->SetAuthor(AUTHOR);
            this->SetVersion(VERSION);
            this->SetType(THIRD);

            this->AddCommand(NICKSERV, new CommandNSSaRegister());
        }
        void OnNickServHelp(User *u)
        {
            m_privmsg("NickServ", u->nick.c_str(), "    SAREGISTER Register a nickname");
        }
};

MODULE_INIT(NSSaRegister)

