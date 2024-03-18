-- MySQL dump 10.10
--
-- Host: localhost    Database: anope
-- ------------------------------------------------------
-- Server version	5.0.24a-Debian_9-log
--
-- Table structure for table `anope_sqlcmd`
--

DROP TABLE IF EXISTS `anope_sqlcmd`;
CREATE TABLE `anope_sqlcmd` (
  `id` int(10) NOT NULL auto_increment,
  `cmd` varchar(30) NOT NULL,
  `params` varchar(255) NOT NULL,
  `timestamp` int(20) NOT NULL,
  `chksum` varchar(32) NOT NULL,
  `status` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) AUTO_INCREMENT=1;
