 /*************************************************************
 *  misc_sqlcmd - Perform commands in Anope using a MySQL DB  *
 *************************************************************/

 /*
  * $Id: misc_sqlcmd_errors.h 21 2006-11-04 23:53:04Z heinz $
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

/* No error occurred! */
#define SQLCMD_ERROR_NONE                        1

/* Common error return values */
#define SQLCMD_ERROR_ACCESS_DENIED              -1
#define SQLCMD_ERROR_READ_ONLY                  -2
#define SQLCMD_ERROR_DEFCON                     -3
#define SQLCMD_ERROR_SYNTAX_ERROR               -4
#define SQLCMD_ERROR_PERMISSION_DENIED          -5
#define SQLCMD_ERROR_MORE_OBSCURE_PASS          -6
#define SQLCMD_ERROR_UNSUPPORTED                -100
#define SQLCMD_ERROR_CHECKSUM                   -101
#define SQLCMD_ERROR_NOT_IMPLEMENTED            -102
#define SQLCMD_ERROR_UNKNOWN_CMD                -103

/* Nickname related errors */
#define SQLCMD_ERROR_NICK_NOT_REGISTERED        -10
#define SQLCMD_ERROR_NICK_ALREADY_REGISTERED    -11
#define SQLCMD_ERROR_NICK_FORBIDDEN             -12
#define SQLCMD_ERROR_NICK_SUSPENDED             -13
#define SQLCMD_ERROR_NICK_IN_USE                -14
#define SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED  -15
#define SQLCMD_ERROR_EMAIL_INVALID              -16
#define SQLCMD_ERROR_NICK_CONF_NOT_FOUND        -17
#define SQLCMD_ERROR_NICK_CONF_INVALID          -18
#define SQLCMD_ERROR_NICK_REG_FAILED            -19
#define SQLCMD_ERROR_NICK_GROUP_CHANGE_DISABLED -20
#define SQLCMD_ERROR_NICK_GROUP_SAME            -21
#define SQLCMD_ERROR_NICK_GROUP_TOO_MANY        -22

/* Channel related errors */
#define SQLCMD_ERROR_CHAN_NOT_REGISTERED        -30
#define SQLCMD_ERROR_CHAN_FORBIDDEN             -31
#define SQLCMD_ERROR_CHAN_SYM_REQ               -32
#define SQLCMD_ERROR_CHAN_INVALID               -33
#define SQLCMD_ERROR_CHAN_MUST_BE_EMPTY         -34
#define SQLCMD_ERROR_CHAN_ALREADY_REG           -35
#define SQLCMD_ERROR_CHAN_MAY_NOT_BE_REG        -36
#define SQLCMD_ERROR_REACHED_CHAN_LIMIT         -37
#define SQLCMD_ERROR_CHAN_REG_FAILED            -38
#define SQLCMD_ERROR_CHAN_NOT_XOP               -39
#define SQLCMD_ERROR_CHAN_XOP_REACHED_LIMIT     -40
#define SQLCMD_ERROR_CHAN_XOP_NOT_FOUND         -41

/* Memo related errors */
#define SQLCMD_ERROR_NO_MEMOS                   -50
#define SQLCMD_ERROR_INVALID_MEMO               -51
#define SQLCMD_ERROR_MEMO_DEL_FAILED            -52
#define SQLCMD_ERROR_MEMO_RECEIVER_INVALID      -53

/* Bot related errors */
#define SQLCMD_ERROR_BOT_NO_EXIST               -60
#define SQLCMD_ERROR_BOT_NOT_ASSIGNED           -61
#define SQLCMD_ERROR_BOT_ALREADY_ASSIGNED       -62
#define SQLCMD_ERROR_BOT_NOT_ON_CHAN            -63
