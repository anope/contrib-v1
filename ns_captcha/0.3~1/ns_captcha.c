#include "module.h"
#include "mysql.h"

#define AUTHOR "Rob"
#define VERSION "0.3"

/**
 * captchaData table definition
 * +-----------+--------------+------+-----+---------+----------------+
 * | Field     | Type         | Null | Key | Default | Extra          |
 * +-----------+--------------+------+-----+---------+----------------+
 * | id        | int(11)      | NO   | PRI | NULL    | auto_increment |
 * | timestamp | int(11)      | YES  |     | NULL    |                |
 * | nick      | varchar(32)  | YES  |     | NULL    |                |
 * | code      | varchar(32)  | YES  |     | NULL    |                |
 * | email     | varchar(100) | YES  |     | NULL    |                |
 * +-----------+--------------+------+-----+---------+----------------+
 *
 **/

NickRequest *makerequest(const char *nick);
int do_register(User * u);
int my_expire(int argc, char **argv);

int loadConfig();
int writeCodeToFile(const unsigned long rtime, const char *nick, const char *code, const char *email);
int mysql_write(const unsigned long rtime, const char *nick, const char *code, const char *email);

char *code_filename = NULL;
char *user_text = NULL;
char *mysql_database = NULL;


#ifdef USE_MYSQL
char *escapeString(MYSQL *mysql, const char *sql); 
#endif

int AnopeInit(int argc, char **argv)
{
	Command *c = NULL;
	EvtHook *hook = NULL;
	
	moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    
    if(loadConfig()) {
		return MOD_STOP;    	
    }
    
    c = createCommand("REGISTER", do_register, NULL, NICK_HELP_REGISTER,-1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_HEAD);
    
    hook = createEventHook(EVENT_DB_EXPIRE, my_expire);
    moduleAddEventHook(hook);
    
    return MOD_CONT;
}

void AnopeFini() { }

/**
 * The /ns register command.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_register(User * u)
{
    NickRequest *nr = NULL, *anr = NULL;
    int ret_code = 0;
    NickCore *nc = NULL;
    int prefixlen = strlen(NSGuestNickPrefix);
    int nicklen = strlen(u->nick);
    char *pass = strtok(NULL, " ");
    char *email = strtok(NULL, " ");
    char passcode[11];
    int idx, min = 1, max = 62, i = 0;
    int chars[] =
        { ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
        'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
        'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    };

    if (readonly) {
        notice_lang(s_NickServ, u, NICK_REGISTRATION_DISABLED);
        return MOD_STOP;
    }

    if (checkDefCon(DEFCON_NO_NEW_NICKS)) {
        notice_lang(s_NickServ, u, OPER_DEFCON_DENIED);
        return MOD_STOP;
    }

    if (!is_oper(u) && NickRegDelay
        && ((time(NULL) - u->my_signon) < NickRegDelay)) {
        notice_lang(s_NickServ, u, NICK_REG_DELAY, NickRegDelay);
        return MOD_STOP;
    }

    if ((anr = findrequestnick(u->nick))) {
        notice_lang(s_NickServ, u, NICK_REQUESTED);
        return MOD_STOP;
    }
    /* Prevent "Guest" nicks from being registered. -TheShadow */

    /* Guest nick can now have a series of between 1 and 7 digits.
     *      --lara
     */
    if (nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 &&
        stristr(u->nick, NSGuestNickPrefix) == u->nick &&
        strspn(u->nick + prefixlen, "1234567890") == nicklen - prefixlen) {
        notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
        return MOD_STOP;
    }

    if (!anope_valid_nick(u->nick)) {
        notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, u->nick);
        return MOD_STOP;
    }

    if (RestrictOperNicks) {
        for (i = 0; i < RootNumber; i++) {
            if (stristr(u->nick, ServicesRoots[i]) && !is_oper(u)) {
                notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED,
                            u->nick);
                return MOD_STOP;
            }
        }
        for (i = 0; i < servadmins.count && (nc = servadmins.list[i]); i++) {
            if (stristr(u->nick, nc->display) && !is_oper(u)) {
                notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED,
                            u->nick);
                return MOD_STOP;
            }
        }
        for (i = 0; i < servopers.count && (nc = servopers.list[i]); i++) {
            if (stristr(u->nick, nc->display) && !is_oper(u)) {
                notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED,
                            u->nick);
                return MOD_STOP;
            }
        }
    }

    if (!pass) {
        if (NSForceEmail) {
            syntax_error(s_NickServ, u, "REGISTER",
                         NICK_REGISTER_SYNTAX_EMAIL);
        } else {
            syntax_error(s_NickServ, u, "REGISTER", NICK_REGISTER_SYNTAX);
        }
    } else if (NSForceEmail && !email) {
        syntax_error(s_NickServ, u, "REGISTER",
                     NICK_REGISTER_SYNTAX_EMAIL);
    } else if (time(NULL) < u->lastnickreg + NSRegDelay) {
        notice_lang(s_NickServ, u, NICK_REG_PLEASE_WAIT, NSRegDelay);
    } else if (u->na) {         /* i.e. there's already such a nick regged */
        if (u->na->status & NS_VERBOTEN) {
            alog("%s: %s@%s tried to register FORBIDden nick %s",
                 s_NickServ, u->username, u->host, u->nick);
            notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
        } else {
            notice_lang(s_NickServ, u, NICK_ALREADY_REGISTERED, u->nick);
        }
    } else if (stricmp(u->nick, pass) == 0
               || (StrictPasswords && strlen(pass) < 5)) {
        notice_lang(s_NickServ, u, MORE_OBSCURE_PASSWORD);
    } else if (enc_encrypt_check_len(strlen(pass), PASSMAX - 1)) {
        notice_lang(s_NickServ, u, PASSWORD_TOO_LONG);
    } else if (email && !MailValidate(email)) {
        notice_lang(s_NickServ, u, MAIL_X_INVALID, email);
    } else {
        for (idx = 0; idx < 9; idx++) {
            passcode[idx] =
                chars[(1 +
                (int) (((float) (max - min)) * getrandom16() /
                (65535 + 1.0)) + min)];
        } passcode[idx] = '\0';
        nr = makerequest(u->nick);
        nr->passcode = sstrdup(passcode);
		enc_encrypt(pass, strlen(pass), nr->password, PASSMAX - 1);

        if (email) {
            nr->email = sstrdup(email);
        }
        nr->requested = time(NULL);

		if(mysql_database) {
			ret_code = mysql_write(nr->requested, nr->nick,nr->passcode,email);
		} else {
			ret_code = writeCodeToFile(nr->requested, nr->nick,nr->passcode,email);
		}
		
		if( ret_code != 0) {
			delnickrequest(nr);
		} else {
			char buffer[2001] = { 0 };
			strncpy(buffer,user_text,2000);
			strnrepl(buffer, 2000, "$nick", u->nick);
			strnrepl(buffer, 2000, "$email", email);
			notice_user(s_NickServ, u, buffer);				
		}
    }
    return MOD_STOP;
}

NickRequest *makerequest(const char *nick)
{
    NickRequest *nr;

    nr = scalloc(1, sizeof(NickRequest));
    nr->nick = sstrdup(nick);
    insert_requestnick(nr);
    alog("%s: Nick %s has been requested", s_NickServ, nr->nick);
    return nr;
}

int loadConfig() {
	
	int i;
	int ret = 1;
    char *tmp = NULL;
    char *tmp2 = NULL;
    char *tmp3 = NULL;

    Directive confvalues[][1] = {
        {{"CaptchaCodeFile", {{PARAM_STRING, PARAM_RELOAD, &tmp}}}},
        {{"CaptchaUserText", {{PARAM_STRING, PARAM_RELOAD, &tmp2}}}},
        {{"CaptchaMysqlDatabase", {{PARAM_STRING, PARAM_RELOAD, &tmp3}}}}
    };

    for (i = 0; i < 3; i++)
        moduleGetConfigDirective(confvalues[i]);

	if(user_text) {
		free(user_text);
		user_text = NULL;
	}
	
	if(tmp) {
		if(code_filename) {
			free(code_filename);
		}
		code_filename = tmp;	
	} 
	if(tmp2) {
		if(user_text) {
			free(user_text);
		}
		user_text = tmp2;	
	}
	if(tmp3) {
		if(mysql_database) {
			free(mysql_database);
		}	
		mysql_database = tmp3;
	}
	
	if(code_filename && user_text) {
		alog("ns_captcha: Directive CaptchaCodeFile loaded (%s)...", code_filename);
    	alog("ns_captcha: Directive CaptchaUserText loaded (%s)...", user_text);
		ret = 0;
	} else {
		alog("ns_captcha: ERROR config items not set either (CaptchaCodeFile or CaptchaUserText)");
	}

    return ret;
}

int writeCodeToFile(const unsigned long rtime, const char *nick, const char *code, const char *email) {
	FILE *out = NULL;
	int ret = -1;
	
	if ((out = fopen(code_filename, "a")) == NULL ) {
		alog("ns_captcha: ERROR - unable to open [%s]",code_filename);
		anope_cmd_global(s_NickServ,"ns_captcha: ERROR - unable to open [%s]",code_filename);
		ret = -2;
	} else {
		if(email) {
			fprintf(out, "%ld %s %s %s\n", rtime, nick,code,email);
		} else {
			fprintf(out, "%ld %s %s\n", rtime, nick,code);
		}
		fclose(out);
		ret = 0;
	}
    return ret;	
}

int my_expire(int argc, char **argv) {
	FILE *out = NULL;
	NickRequest *nr = NULL;
	int i;
	
	if(!mysql_database) {
		if ((out = fopen(code_filename, "w")) == NULL ) {
			alog("ns_captcha: ERROR - unable to open [%s] for expiry",code_filename);
			anope_cmd_global(s_NickServ,"ns_captcha: ERROR - unable to open [%s] for expiry",code_filename);
		} else {
			for (i = 0; i < 1024; i++) {
				for (nr = nrlists[i]; nr; nr = nr->next) {
					if(nr->email) {
						fprintf(out, "%ld %s %s %s\n", nr->requested, nr->nick,nr->passcode, nr->email);
					} else {
						fprintf(out, "%ld %s %s\n",nr->requested, nr->nick,nr->passcode);
					}
	    		}
			}
		}
	}
	return MOD_CONT;
}

int mysql_write(const unsigned long rtime, const char *nick, const char *code, const char *email) {
	int ret = -1;
#ifdef USE_MYSQL

	char *quotedNick = NULL;
	char *quotedEmail = NULL;
	char *quotedCode = NULL;	

	char buffer[1000] = { 0 };
	MYSQL mysql;
	
	if(mysql_init(&mysql)==NULL) {
		alog("ns_captcha: unable to init mysql");
	} else {
		if (!mysql_real_connect(&mysql,MysqlHost,MysqlUser,MysqlPass,NULL,0,NULL,0)) {
    		alog("ns_captcha: unable to connect to mysql");
		} else {
			if(mysql_select_db(&mysql,mysql_database)!=0) {
				alog("ns_captcha: mysql database does not exist");
			} else {				
				quotedNick  = escapeString(&mysql,nick);
				quotedEmail = escapeString(&mysql,email);
				if(email) quotedCode = escapeString(&mysql,code); 
				snprintf(buffer,1000,"INSERT INTO captchaData (timestamp,nick,code,email) VALUES (%ld,'%s','%s','%s');",rtime,quotedNick,quotedCode,email ? quotedEmail : "");
			    alog("String used: [%s]",buffer);
				if (mysql_query (&mysql, buffer) != 0) {
					alog("ns_captcha: mysql insert failed (%s)",mysql_error(&mysql));					
				} else {
					ret = 0;	
				}
				free(quotedNick);
				free(quotedCode);
				if(email) free(quotedEmail);
			}
		}		
	}
	mysql_close(&mysql);
#endif
	return ret;	
}

#ifdef USE_MYSQL
char *escapeString(MYSQL *mysql, const char *sql) {
	
	int slen;
    char *quoted;
    
    if (!sql)
        return sstrdup("");

    slen = strlen(sql);
    quoted = malloc((1 + (slen * 2)) * sizeof(char));

    mysql_real_escape_string(mysql, quoted, sql, slen);

    return quoted;	
}
#endif

/* EOF */

