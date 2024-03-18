#include "misc_sqlcmd.h"

 /*************************************************************
 *  misc_sqlcmd - Perform commands in Anope using a MySQL DB  *
 *************************************************************/

 /*
  * $Id: misc_sqlcmd_bot.c 18 2006-10-30 18:09:53Z heinz $
  */
 
 /**************************************************************
 * Module Info & Changelog      
 * -----------------------                                     
 * Please see misc_sqlcmd_main.c for the information, changelog
 * and the configuration information.
 *
 **************************************************************/
 
 /**************************************************************
 *                                                             *
 * PLEASE DO NOT EDIT ANYTHING BELOW HERE - MODULE CODE BEGINS *
 *                                                             *
 **************************************************************/

/*
 * BOT_ASSIGN - Assign a BotServ bot to a channel
 *
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - Bot to assign
 */
int sqlcmd_handle_botassign(int ac, char **av)
{

        BotInfo *bi;
        ChannelInfo *ci;

        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!av[2] || !av[4])
                return SQLCMD_ERROR_SYNTAX_ERROR;
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], CA_ASSIGN) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
        else if (!(bi = findbot(av[4])))
                return SQLCMD_ERROR_BOT_NO_EXIST;
        else if (bi->flags & BI_PRIVATE)
                return SQLCMD_ERROR_PERMISSION_DENIED;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if ((ci->bi) && (stricmp(ci->bi->nick, av[4]) == 0))
                return SQLCMD_ERROR_BOT_ALREADY_ASSIGNED;
        else if (ci->botflags & BS_NOBOT)
                return SQLCMD_ERROR_PERMISSION_DENIED;
        else {
                if (ci->bi)
                        sqlcmd_bot_unassign(ci, av[0]);
                ci->bi = bi;
                bi->chancount++;
                if (ci->c && ci->c->usercount >= BSMinUsers)
                        bot_join(ci);
                send_event(EVENT_BOT_ASSIGN, 2, ci->name, bi->nick);
                return SQLCMD_ERROR_NONE;
        }
}

/*
 * BOT_SAY - Make botserv say something to a channel
 *
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - Message to send
 */
int sqlcmd_handle_botsay(int ac, char **av)
{
        ChannelInfo *ci;
        
        if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], CA_SAY) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
                
        if (!av[2] || !av[4])
                return SQLCMD_ERROR_SYNTAX_ERROR;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!ci->bi)
                return SQLCMD_ERROR_BOT_NOT_ASSIGNED;
        else if (!ci->c || ci->c->usercount < BSMinUsers)
                return SQLCMD_ERROR_BOT_NOT_ON_CHAN;
        else {       
                if (av[4][0] != '\001') {
                        anope_cmd_privmsg(ci->bi->nick, ci->name, "%s", av[4]);
                        ci->bi->lastmsg = time(NULL);
                        if (logchan && LogBot)
                                anope_cmd_privmsg(ci->bi->nick, LogChannel, "SAY %s %s %s", av[0], ci->name, av[4]);
                        return SQLCMD_ERROR_NONE;
                } else {
                        return SQLCMD_ERROR_SYNTAX_ERROR;
                }
        }
}

/*
 * BOT_ACT - Make botserv act something in a channel
 *
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - Message to act
 */
int sqlcmd_handle_botact(int ac, char **av)
{
        ChannelInfo *ci;

        if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], CA_SAY) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;

        if (!av[2] || !av[4])
                return SQLCMD_ERROR_SYNTAX_ERROR;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!ci->bi)
                return SQLCMD_ERROR_BOT_NOT_ASSIGNED;
        else if (!ci->c || ci->c->usercount < BSMinUsers)
                return SQLCMD_ERROR_BOT_NOT_ON_CHAN;
        else {
                strnrepl(av[4], sizeof(av[4]), "\001", "");
                anope_cmd_privmsg(ci->bi->nick, ci->name, "%cACTION %s %c", 1, av[4], 1);
                ci->bi->lastmsg = time(NULL);
                if (logchan && LogBot)
                        anope_cmd_privmsg(ci->bi->nick, LogChannel, "ACT %s %s %s", av[0], ci->name, av[4]);
                return SQLCMD_ERROR_NONE;
        }
}

/*
 * BOT_UNASSIGN - Unassign a BotServ bot from a channel
 *
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password
 */
int sqlcmd_handle_botunassign(int ac, char **av)
{
        ChannelInfo *ci;

        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], CA_ASSIGN) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;       
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!ci->bi)
                return SQLCMD_ERROR_BOT_NOT_ASSIGNED;
        else {
                sqlcmd_bot_unassign(ci, av[0]);
                return SQLCMD_ERROR_NONE;    
        }
}


void sqlcmd_bot_unassign(ChannelInfo *ci, char *user)
{
        send_event(EVENT_BOT_UNASSIGN, 2, ci->name, ci->bi->nick);
        if (ci->c && ci->c->usercount >= BSMinUsers)
                anope_cmd_part(ci->bi->nick, ci->name, "UNASSIGN from %s via SQL Command", user);
        ci->bi->chancount--;
        ci->bi = NULL;
}
