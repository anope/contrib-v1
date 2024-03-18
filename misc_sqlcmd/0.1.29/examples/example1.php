<?php

	/*********************
	 * misc_sqlcmd v2.00 *
	 ********************/

	/* $Id: example1.php 23 2006-11-05 01:00:06Z heinz $ */

	/***********************************************
 	* Example 1: 
	* ==========
	*
	* This script displays a form and prompts the
	* user to enter a nickname, password and email.
	*
	* The information is then sent as a request to
	* Anope for processing, and the status is
	* displayed upon completion.
	*
	************************************************/

	include_once 'sqlcmd.lib.php';
	session_start();

	if (empty($_SESSION['misc_sqlcmd_id']) && (empty($_POST['nickname']) || empty($_POST['password']) || empty($_POST['email']))) {
		display_reg_form();
	}
	elseif (!empty($_SESSION['misc_sqlcmd_id'])) {
		fetch_cmd_status();
	}
	else {		
		$nick = $_POST['nickname'];
		$pass = $_POST['password'];
		$email = $_POST['email'];
		$params = array($nick, $pass, $email);
		if (!($query = sqlcmd_cmd_make('NICK_REG', $params))) {
			die("sqlcmd.lib.php error in sqlcmd_cmd_make!");
		}
		$id = sqlcmd_run($query);
		if (!$id) {
			die("sqlcmd.lib.php error in sqlcmd_run - MySQL Error - ".mysql_error());
		}
		$_SESSION['misc_sqlcmd_id'] = $id;
		header("Location: ".$_SERVER['PHP_SELF']);
		exit;
	}

	function fetch_cmd_status() {
		$id = intval($_SESSION['misc_sqlcmd_id']);
		$status = sqlcmd_status($id);
		if ($status === false) {
			die("sqlcmd.lib.php error in sqlcmd_status");
		}
		if ($status === 0) {
			echo "Command is still processing... Please Wait...";
			echo "<script> setTimeout('recheckStatus()', 2000); function recheckStatus() { location.href = '".$_SERVER['PHP_SELF']."'; } </script>";
		}
		else {
			echo "Your nickname registration has processed.<br/><br/>";
			echo ($status == 1 ? 'The nickname was registered successfully!' : 'There was an error during registration: '.sqlcmd_fetch_error($status));
			unset($_SESSION['misc_sqlcmd_id']);
		}
	}

	function display_reg_form() {
?>
	Please fill in the form below to register a nickname:
	<form method="post">
		Nickname: <input type="text" name="nickname" value="" size="12" /><br/>
		Password: <input type="password" name="password" value="" size="12" /><br/>
		E-Mail Address: <input type="text" name="email" value="" size="12" /><br/><br/>
		<input type="submit" name="register" value="Register Nickname!" />
	</form>
<?php
	}

?>
