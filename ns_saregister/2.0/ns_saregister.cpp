/* Original Module Author: Adam
 * Updated for 1.9.4 (and beyond?): Laura
 * Data of Original: 14/02/10
 * Date of This Version: 26/12/10
 **********************************************
 * Module used in: Nickserv
 * Syntax: /ns saregister 'nick' 'pass' 'email'
 **********************************************
 * Changes: Updated to work with 1.9.4
*/
#include "module.h"

#define AUTHOR "Laura"
#define VERSION "2.0"
#define INVALID_NICKNAME "This nickname is invalid for use with standard RFC 2812"

class CommandNSSaRegister : public Command
{
    public:
        CommandNSSaRegister() : Command("SAREGISTER", 3, 3, "nickserv/saregister")
        {
        }
        CommandReturn Execute(CommandSource &source, const std::vector<Anope::string> &params)
        {
            User *u = source.u;
            const Anope::string &nick = params[0];
            const Anope::string &password = params[1];
            const Anope::string &email = params[2];

            NickRequest *nr = NULL;
            NickAlias *na = findnick(nick);

            //Debug Stuff Below
            //source.Reply(nick.c_str());
            //source.Reply(password.c_str());
            //source.Reply(email.c_str());
            //return MOD_CONT;

            size_t prefixlen = Config->NSGuestNickPrefix.length();
            std::list<std::pair<Anope::string, Anope::string> >::iterator it, it_end;
            
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
            if (findbot(nick)) //works
            {
                source.Reply(BOT_BOT_ALREADY_EXISTS, nick.c_str());     //Copied from Botserv Add.. Find a better one if needed.
                return MOD_STOP;
            }
            if (findnick(nick)) //works
            {
                source.Reply(NICK_ALREADY_REGISTERED, nick.c_str());
                return MOD_STOP;
            }
            //Is this even required? --Covered by registered nick? yes
            /*if (na && na->HasFlag(NS_FORBIDDEN))
            {
                source.Reply(NICK_X_FORBIDDEN, nick.c_str());
                return MOD_STOP;
            }
            //Is this even required? --Covered by registered nick? yes
            if (na && na->nc->HasFlag(NI_SUSPENDED))
            {
                source.Reply(NICK_X_SUSPENDED, nick.c_str());
                return MOD_STOP;
            }
            */
            //Allowed to create nicks at this time?
            if (readonly)
            {
                source.Reply(NICK_REGISTRATION_DISABLED);
                return MOD_STOP;
            }
            //Check for valid nick -- nothing too long, no Guest####### nicks, and nothing banned in ircd.conf
            if (nick.length() > Config->NickLen) //works
            {
                source.Reply(INVALID_NICKNAME);
                return MOD_STOP;
            }
            if (nick.length() <= prefixlen + 7 && nick.length() >= prefixlen + 1 && !nick.find_ci(Config->NSGuestNickPrefix) && nick.substr(prefixlen).find_first_not_of("1234567890") == Anope::string::npos)
            {
                source.Reply(NICK_CANNOT_BE_REGISTERED, nick.c_str());
                return MOD_STOP;
            }
            if (!ircdproto->IsNickValid(nick))
            {
                source.Reply(NICK_X_FORBIDDEN, nick.c_str());
                return MOD_STOP;
            }
            if (Config->RestrictOperNicks)
            {
                for (it = Config->Opers.begin(), it_end = Config->Opers.end(); it != it_end; ++it)
                {
                    Anope::string temp_nick = it->first;

                    if (nick.find_ci(temp_nick) != Anope::string::npos)
                    {
                        source.Reply(NICK_CANNOT_BE_REGISTERED, nick.c_str());
                        return MOD_STOP;
                    }
                }
            }
            //Prolly don't need this, but just in case someone wants to try to crash things
            if (isdigit(nick[0]) || nick[0] == '-')
            {
                source.Reply(INVALID_NICKNAME);
                return MOD_STOP;
            }
            for (unsigned i =0, end = nick.length(); i <end && i <Config->NickLen; ++i)
                if (!isvalidnick(nick[i]))
                {
                    source.Reply(INVALID_NICKNAME);
                    return MOD_STOP;
                }
            //Check for a valid and secure pass
            if (password.equals_ci(nick) || (Config->StrictPasswords && password.length() <5))
            {
                source.Reply(MORE_OBSCURE_PASSWORD);
                return MOD_STOP;
            }
            else if (password.length() > Config->PassLen)
            {
                source.Reply(PASSWORD_TOO_LONG);
                return MOD_STOP;
            }
            //Check for valid email
            if (!email.empty() && !MailValidate(email))
            {
                source.Reply(MAIL_X_INVALID, email.c_str());
                return MOD_STOP;
            }
            else  //else just for debug, remove later.
            {
                //Stuff to be done when we're sure nick is allowed.
                //Mostly copied from ns_register.cpp (perhaps a better way, can we just call the ns function?)
                //passcode may not be needed, but it's there to make it happy just in case :)
                Anope::string passcode = "";
                for (idx =0; idx <9; ++idx)
                    passcode += chars[1 +static_cast<int>((static_cast<float>(max - min)) * getrandom16() / 65536.0) + min];
		nr = new NickRequest(nick);
		nr->passcode = passcode;
		enc_encrypt(password, nr->password);
                nr->email = email;
                nr->requested = Anope::CurTime;

                na = new NickAlias(nr->nick, new NickCore(nr->nick));
                na->nc->pass = nr->password;
                na->nc->memos.memomax = Config->MSMaxMemos;
                //taken from force because we're forcing the nick creation
                na->last_usermask = "*@*";
                na->last_realname = "unknown";
                na->time_registered = na->last_seen = Anope::CurTime;
                na->nc->language = Config->NSDefLanguage;
                na->nc->email = email;

                Log(LOG_ADMIN, u, this) << "to register " << nick << " (email: " << email << ")";
                source.Reply(NICK_FORCE_REG, nick.c_str());     //Used the "Nickname X confirmed" message to verify that nick was created
                User *user_ = finduser(nr->nick);
                delete nr;
                if (user_) validate_user(user_);
                return MOD_CONT;
            }
//            return MOD_CONT;
        }

        bool OnHelp(CommandSource &source, const Anope::string &subcommand)
        {
            source.Reply(" ");
            source.Reply("/msg NickServ \2SAREGISTER \37nick\37 \37password\37 \37email\37");
            source.Reply(" ");
            source.Reply("Allows services admins to register other nicks.");
            return true;
        }

        void OnSyntaxError(CommandSource &source, const Anope::string &subcommand)
        {
            source.Reply("Syntax: \2SAREGISTER \37nick\37 \37password\37 \37email\37");
        }
        /* Outdated?
        void OnNickServHelp(CommandSource &source)
        {
            source.Reply("    SAREGISTER Register a nickname");
        }
        */
        void OnServHelp(CommandSource &source)
        {
            source.Reply("    SAREGISTER Register a nickname");
        }
};

class NSSaRegister : public Module
{
    CommandNSSaRegister commandnssaregister;

    public:
        NSSaRegister(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
        {
            this->SetAuthor(AUTHOR);
            this->SetVersion(VERSION);
            this->SetType(THIRD);

            this->AddCommand(NickServ, &commandnssaregister);
        }
};

MODULE_INIT(NSSaRegister)

