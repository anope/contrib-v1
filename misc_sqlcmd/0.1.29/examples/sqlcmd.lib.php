<?php

	/*********************
	 * misc_sqlcmd v2.00 *
	 ********************/

	/* $Id$ */

	/***********************************************
 	* sqlcmd.lib.php
	* ==============
	*
	* This file provides library functions that make
	* the use of sqlcmd easier on websites and on
	* the command line.
	*
	* See the example*.php files for ideas on how to
	* integrate this with your own site/scripts.
	*
	************************************************/

	/* Configure these settings! */
	$sqlcmd_mysql_host = "localhost";
	$sqlcmd_mysql_user = "anope";
	$sqlcmd_mysql_pass = "anope";
	$sqlcmd_mysql_db = "anope";
	$sqlcmd_checksum_salt = "misc_sqlcmd_module";
	/* Stop configuring here! */

	require_once 'sqlcmd_errors.lib.php';

	$sqlcmd_db = false;
	$sqlcmd_commands = array(
					"NICK_REG" => 3,
					"NICK_CONF" => 3,
					"NICK_GROUP" => 3,
					"NICK_DROP" => 2,
					"CHAN_REG" => 4,
					"CHAN_ADD_SOP" => 5,
					"CHAN_ADD_AOP" => 5,
					"CHAN_ADD_HOP" => 5,
					"CHAN_ADD_VOP" => 5,
					"CHAN_DEL_SOP" => 5,
					"CHAN_DEL_AOP" => 5,
					"CHAN_DEL_HOP" => 5,
					"CHAN_DEL_VOP" => 5,
					"CHAN_ACCESS" => 6,
					"CHAN_TOPIC" => 5,
					"CHAN_DROP" => 4,
					"MEMO_SEND" => 3,
					"MEMO_DEL" => 3,
					"MEMO_CLEAR" => 2,
					"BOT_ASSIGN" => 5,
					"BOT_SAY" => 4,
					"BOT_ACT" => 4,
					"BOT_UNASSIGN" => 4,
					"PING" => 1,
	);
				
	/* 
	 * sqlcmd_cmd_valid
	 * = Parameters:
	 *   - $cmd - Name of the command to check
	 * = Return values:
	 *   - true - The specified command is valid
	 *   - false - The specified command is invalid
	 */
	function sqlcmd_cmd_valid($cmd = 'NICK_REG') {
		global $sqlcmd_commands;

		$valid = false;
		if (isset($sqlcmd_commands[$cmd])) {
			$valid = true;
		}
		return $valid;
	}

	/*
	 * sqlcmd_cmd_make
	 * = Parameters
	 *   - $cmd - The command to perform
	 *   - $params - An array of the parameters to use (see README.txt for list)
	 * = Return values:
	 *   - false  - An error occurred - invalid command, missing params, etc.
	 *   - $query - Returns an SQL query for the command
	 */
	function sqlcmd_cmd_make($cmd = 'NICK_REG', $params = array()) {
		global $sqlcmd_commands;

		if (!sqlcmd_cmd_valid($cmd)) {
			return false;
		}
		if (count($params) !== $sqlcmd_commands[$cmd]) {
			return false;
		}
		if (!sqlcmd_db_connect()) {
			return false;
		}
		$imp = implode(" ", $params);
		$param_str = mysql_real_escape_string($imp);
		$timestamp = time();
		$checksum = sqlcmd_cmd_checksum($cmd, $param_str, $timestamp);
		$query = "INSERT INTO `anope_sqlcmd` (`cmd`, `params`, `timestamp`, `chksum`) VALUES ('$cmd', '$param_str', $timestamp, '$checksum')";
		return $query;					
	}

	/*
	 * sqlcmd_cmd_checksum
	 * = Parameters
	 *   - $cmd - The command to perform
	 *   - $params - A parameter string
	 *   - $ts - A unix timestamp of the command insertion time
	 * = Return values:
	 *   - $chksum - An MD5 hash of the checksum string
	 */
	function sqlcmd_cmd_checksum($cmd = 'NICK_REG', $params = '', $ts = 0) {
		global $sqlcmd_checksum_salt;
		$chksum = md5($cmd.':'.$params.':'.$ts.':'.$sqlcmd_checksum_salt);
		return $chksum;
	}

	/*
	 * sqlcmd_run
	 * = Parameters
	 *   - $query - The sqlcmd_cmd_make query to run
	 * = Return values:
	 *   - false - An error occurred - MySQL down, Schema error, etc.
	 *   - $id   - An integer of the command ID - used for tracking
	 */
	function sqlcmd_run($query = '') {
		if (!sqlcmd_db_connect()) {
			return false;
		}
		$result = mysql_query($query);
		if (!$result || !($id = mysql_insert_id())) {
			return false;
		}
		return $id;
	}

	/*
	 * sqlcmd_status
	 * = Parameters
	 *   - $id - The command id to track
	 * = Return values:
	 *   - false - An error occurred - MySQL down, Schema error, etc.
	 *   - $id   - An integer of the command status - if not 0, pass to sqlcmd_fetch_error
	 */
	function sqlcmd_status($cmd_id = 0) {
		$cmd_id = intval($cmd_id);
		if (!sqlcmd_db_connect()) {
			die("sqlcmd.lib.php error in sqlcmd_status - MySQL Error - ".mysql_error());
		}
		$query = "SELECT `status` FROM anope_sqlcmd WHERE `id` = $cmd_id";
		$result = mysql_query($query);
		if (!$result || mysql_num_rows($result) == 0) {
			return false;
		}
		$array = mysql_fetch_array($result);
		return intval($array[0]);
	}

	/*
	 * sqlcmd_db_connect
	 * = Parameters
	 *   - None
	 * = Return values:
	 *   - false - An error occurred - MySQL down, Schema error, etc.
	 *   - true  - MySQL available and connected
	 */
	function sqlcmd_db_connect() {
		global $sqlcmd_db;
		global $sqlcmd_mysql_host;
		global $sqlcmd_mysql_user;
		global $sqlcmd_mysql_pass;
		global $sqlcmd_mysql_db;
		
		if ($sqlcmd_db && is_resource($sqlcmd_db)) {
			return true;
		}
		$sqlcmd_db = mysql_connect($sqlcmd_mysql_host, $sqlcmd_mysql_user, $sqlcmd_mysql_pass);
		if (!$sqlcmd_db || !mysql_select_db($sqlcmd_mysql_db)) {
			return false;
		}
		return true;
	}

?>
