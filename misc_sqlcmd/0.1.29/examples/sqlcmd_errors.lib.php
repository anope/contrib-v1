<?php

	/*********************
	 * misc_sqlcmd v2.00 *
	 ********************/

	/* $Id: sqlcmd_errors.lib.php 23 2006-11-05 01:00:06Z heinz $ */

	/***********************************************
 	* Error Code Library
	* ==================
	*
	* This file contains a list of error codes and
	* their meanings.
	*
	* Everytime you install a new version of this
	* module, you should check to ensure you have
	* the latest version of this file in use.
	*
	* To retrieve an error message for a specific
	* error value, simply call:
	*
	* include_once 'sqlcmd_errors.lib.php';
	* $msg = sqlcmd_fetch_error($id, $lang);
	* 
	* (Where $id is the error value, and $lang is
	* the language you require)
	*
	* Currently supported languages:
	* ------------------------------
	*    # English - "en"
	*
	************************************************/
	
	$sqlcmd_errors = array(
	
	        "en" => array(
	                        /* SQLCMD_ERROR_NONE */
	                        1 => "The command has completed successfully",
	                        /* SQLCMD_ERROR_ACCESS_DENIED */
	                        -1 => "Access Denied!",
	                        /* SQLCMD_ERROR_READ_ONLY */
	                        -2 => "Anope is currently running in Read-Only mode",
	                        /* SQLCMD_ERROR_DEFCON */
	                        -3 => "Anope is currently running in a Defcon-restricted mode",
	                        /* SQLCMD_ERROR_SYNTAX_ERROR */
	                        -4 => "There was an error in the Syntax of the command",
	                        /* SQLCMD_ERROR_PERMISSION_DENIED */
	                        -5 => "You do not have sufficient permissions to use this command",
	                        /* SQLCMD_ERROR_MORE_OBSCURE_PASS */
	                        -6 => "Please provide a more obscure password.",
                                /* SQLCMD_ERROR_NICK_NOT_REGISTERED */
                                -10 => "The nickname specified is not registered",
                                /* SQLCMD_ERROR_NICK_ALREADY_REGISTERED */
                                -11 => "The nickname specified is already registered",
                                /* SQLCMD_ERROR_NICK_FORBIDDEN */
                                -12 => "The nickname specified is forbidden",
                                /* SQLCMD_ERROR_NICK_SUSPENDED */
                                -13 => "The nickname specified is suspended",
                                /* SQLCMD_ERROR_NICK_IN_USE */
                                -14 => "The nickname specified is in use on the network",
                                /* SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED */
                                -15 => "The nickname specified cannot be registered",
                                /* SQLCMD_ERROR_EMAIL_INVALID */
                                -16 => "The e-mail address given was invalid",
                                /* SQLCMD_ERROR_NICK_CONF_NOT_FOUND */
                                -17 => "The confirmation for the specified nickname could not be located",
                                /* SQLCMD_ERROR_NICK_CONF_INVALID */
                                -18 => "The confirmation code specified was invalid",
                                /* SQLCMD_ERROR_NICK_REG_FAILED */
                                -19 => "Nickname registration has failed",
                                /* SQLCMD_ERROR_NICK_GROUP_CHANGE_DISABLED */
                                -20 => "Nick Group changing is disabled in Anope",
                                /* SQLCMD_ERROR_NICK_GROUP_SAME */
                                -21 => "You are already grouped to the specified nickname group",
                                /* SQLCMD_ERROR_NICK_GROUP_TOO_MANY */
                                -22 => "You have exceeded the amount of grouped nicknames permitted",
                                /* SQLCMD_ERROR_CHAN_NOT_REGISTERED */
                                -30 => "The channel specified is not registered",
                                /* SQLCMD_ERROR_CHAN_FORBIDDEN */
                                -31 => "The specified channel is forbidden",
                                /* SQLCMD_ERROR_CHAN_SYM_REQ */
                                -32 => "Your channel name should be prefixed with the # symbol",
                                /* SQLCMD_ERROR_CHAN_INVALID */
                                -33 => "The specified channel name is invalid",
                                /* SQLCMD_ERROR_CHAN_MUST_BE_EMPTY */
                                -34 => "The channel you wish to register must be empty",
                                /* SQLCMD_ERROR_CHAN_ALREADY_REG */
                                -35 => "The specified channel is already registered",
                                /* SQLCMD_ERROR_CHAN_MAY_NOT_BE_REG */
                                -36 => "The specified channel may not be registered",
                                /* SQLCMD_ERROR_REACHED_CHAN_LIMIT */
                                -37 => "You have reached the Channel Limit for this nickname",
                                /* SQLCMD_ERROR_CHAN_REG_FAILED */
                                -38 => "Registration of the specified channel has failed",
                                /* SQLCMD_ERROR_CHAN_NOT_XOP */
                                -39 => "The specified channel does not use the XOP system",
                                /* SQLCMD_ERROR_CHAN_XOP_REACHED_LIMIT */
                                -40 => "The specified channel has reached the XOP limit",
                                /* SQLCMD_ERROR_CHAN_XOP_NOT_FOUND */
                                -41 => "The nickname specified was not found on the XOP list",
                                /* SQLCMD_ERROR_NO_MEMOS */
                                -50 => "You have no memos",
                                /* SQLCMD_ERROR_INVALID_MEMO */
                                -51 => "The specified memo does not exist",
                                /* SQLCMD_ERROR_MEMO_DEL_FAILED */
                                -52 => "Deletion of the specified memo has failed",
                                /* SQLCMD_ERROR_MEMO_RECEIVER_INVALID */
                                -53 => "The memo receipiant does not exist",
                                /* SQLCMD_ERROR_BOT_NO_EXIST */
                                -60 => "The specified Botserv bot does not exist",
                                /* SQLCMD_ERROR_BOT_NOT_ASSIGNED */
                                -61 => "There is currently no Botserv bot assigned to the specified channel",
                                /* SQLCMD_ERROR_BOT_ALREADY_ASSIGNED */
                                -62 => "A Botserv bot is already assigned to the specified channel",
                                /* SQLCMD_ERROR_BOT_NOT_ON_CHAN */
                                -63 => "The Botserv bot is not currently on the channel specified",	                        
	                        /* SQLCMD_ERROR_UNSUPPORTED */
	                        -100 => "The IRCd currently running this version of Anope does not support this command",
	                        /* SQLCMD_ERROR_CHECKSUM */
	                        -101 => "There was an error with the computed checksum for this command.",
	                        /* SQLCMD_ERROR_NOT_IMPLEMENTED */
	                        -102 => "This command has not yet been implemented",
	                        /* SQLCMD_ERROR_UNKNOWN_CMD */
	                        -103 => "The specified command could not be found",
	                ),
        );
        
        function sqlcmd_fetch_error($id = 0, $lang = "en") {
                global $sqlcmd_errors;
                
                if (!isset($sqlcmd_errors[$lang]) || !is_array($sqlcmd_errors[$lang]) || empty($sqlcmd_errors[$lang][$id])) {
                        $tmp = "Unknown error code '$id'";
                        return $tmp;
                }
                
                return $sqlcmd_errors[$lang][$id];
        }
        
?>
