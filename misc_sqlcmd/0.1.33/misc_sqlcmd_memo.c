#include "misc_sqlcmd.h"

 /*************************************************************
 *  misc_sqlcmd - Perform commands in Anope using a MySQL DB  *
 *************************************************************/

 /*
  * $Id: misc_sqlcmd_memo.c 18 2006-10-30 18:09:53Z heinz $
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
 * MEMO_SEND - Send a memo
 *
 * av[0] - Sender Nickname
 * av[1] - Sender Password
 * av[2] - Receiver Nickname
 * av[3] - Memo Text
 */
int sqlcmd_handle_memosend(int ac, char **av)
{
        NickAlias *sender = NULL;
        NickAlias *receiver = NULL;
        NickAlias *na = NULL;
        NickCore *nc = NULL;
        Memo *m = NULL;
        MemoInfo *mi = NULL;
        User *u = NULL;
              
        sender = findnick(av[0]);
        receiver = findnick(av[2]);
        if (!sender)
                return SQLCMD_ERROR_NICK_NOT_REGISTERED;
        if (!receiver)
                return SQLCMD_ERROR_MEMO_RECEIVER_INVALID;

        mi = &receiver->nc->memos;

        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (checkDefCon(DEFCON_NO_NEW_MEMOS))
                return SQLCMD_ERROR_DEFCON;
        else  if (!enc_check_password(av[1], sender->nc->pass))
                return SQLCMD_ERROR_ACCESS_DENIED;
        else {
                mi->memocount++;
                mi->memos = srealloc(mi->memos, sizeof(Memo) * mi->memocount);
                m = &mi->memos[mi->memocount - 1];
                strscpy(m->sender, sender->nick, NICKMAX);
                m->moduleData = NULL;
                if (mi->memocount > 1) {
                        m->number = m[-1].number + 1;
                        if (m->number < 1) {
                                int i;
                                for (i = 0; i < mi->memocount; i++) {
                                        mi->memos[i].number = i + 1;
                                }
                        }
                } else {
                        m->number = 1;
                }
                m->time = time(NULL);
                m->text = sstrdup(av[3]);
                m->flags = MF_UNREAD;
                nc = receiver->nc;

                if (MSNotifyAll) {
                        if ((nc->flags & NI_MEMO_RECEIVE) && get_ignore(receiver->nick) == NULL) {
                                int i;
                                for (i = 0; i < nc->aliases.count; i++) {
                                        na = nc->aliases.list[i];
                                        if (na->u && nick_identified(na->u))
                                                notice_lang(s_MemoServ, na->u, MEMO_NEW_MEMO_ARRIVED, sender->nick, s_MemoServ, m->number);
                                }
                        } else {
                                if ((u = finduser(receiver->nick)) && nick_identified(u) && (nc->flags & NI_MEMO_RECEIVE))
                                        notice_lang(s_MemoServ, u, MEMO_NEW_MEMO_ARRIVED, sender->nick, s_MemoServ, m->number);
                        }
                }
                return SQLCMD_ERROR_NONE;
        }
}


/*
 * MEMO_DEL - Delete a memo/set of memos
 *
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Memo/list of memos
 */
int sqlcmd_handle_memodel(int ac, char **av)
{ 
        NickAlias *na;
        MemoInfo *mi;
        char *numstr = av[2];
        int memonum, i;

        na = findnick(av[0]);
        if (!na)
                return SQLCMD_ERROR_NICK_NOT_REGISTERED;

        if (!enc_check_password(av[1], na->nc->pass))
                return SQLCMD_ERROR_ACCESS_DENIED;
      
        mi = &na->nc->memos;
        if (!numstr || !isdigit(*numstr))
                return SQLCMD_ERROR_SYNTAX_ERROR;
        if (!mi || mi->memocount == 0)
                return SQLCMD_ERROR_NO_MEMOS;
         
        memonum = atoi(numstr);       
        alog("debug: [%s] memonum = %d, memocount = %d", MODNAME, memonum, mi->memocount);
        if (memonum > mi->memocount) {
                return SQLCMD_ERROR_INVALID_MEMO;
        }
        if (!delmemo(mi, memonum)) {
                alog("debug: [%s] mi->memos[memonum].number = %d", MODNAME, mi->memos[memonum].number);
                return SQLCMD_ERROR_MEMO_DEL_FAILED;
        }
        for (i = 0; i < mi->memocount; i++)
            mi->memos[i].number = i + 1;

        return SQLCMD_ERROR_NONE;
}


/*
 * MEMO_CLEAR - Delete all memos
 *
 * av[0] - Nickname
 * av[1] - Password
 */
int sqlcmd_handle_memoclear(int ac, char **av)
{ 
        NickAlias *na;
        MemoInfo *mi;
        int i;

        na = findnick(av[0]);
        if (!na)
                return SQLCMD_ERROR_NICK_NOT_REGISTERED;

        if (!enc_check_password(av[1], na->nc->pass))
                return SQLCMD_ERROR_ACCESS_DENIED;
      
        mi = &na->nc->memos;
        if (!mi || mi->memocount == 0)
                return SQLCMD_ERROR_NO_MEMOS;

        /* Delete all memos. */
        for (i = 0; i < mi->memocount; i++) {
                free(mi->memos[i].text);
                moduleCleanStruct(&mi->memos[i].moduleData);
        }
        free(mi->memos);
        mi->memos = NULL;
        mi->memocount = 0;
        return SQLCMD_ERROR_NONE;   
}
