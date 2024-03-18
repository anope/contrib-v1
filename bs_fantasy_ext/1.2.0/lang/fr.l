##############################################################################
##### French language file.                                              #####
##############################################################################
# Module Name    : bs_fantasy_ext
# Author         : Viper <Viper@Anope.org>
# Creation Date  : 14/09/2008
# Last Updated   : 17/06/2007
#
# More info on http://modules.anope.org and http://forum.anope.org
#
##############################################################################
#
# French language file provided by Seb Inconnu.
#
# ! This file is currently outdated so i am looking for someone to 
# ! update this file to the new layout used in en_us.l and update language strings.
# ! Until updated this language is disabled.
# ! Contact me @ viper@anope.org.
#
##############################################################################
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
##############################################################################
#
# When translating this file to another language, keep in mind that the
# order of parameters for sprintf() is fixed in the source code, so your
# messages need to take the same parameters in the same order as the
# English messages do. If you can't get a natural translation
# of a message without changing the order of the parameters, let me know
# (viper@anope.org) which message is causing a problem and I'll see
# what I can do.
#
#
# Also in help messages, please try to limit line lengths to 90 characters
# of text (not including the leading tab).  This length was chosen because
# it does not cause line wrap under default settings on most current IRC
# clients.  Remember that format characters (control-B, control-_) are not
# included in that 90-character limit (since they don't show on the user's
# screen).  Also remember that format specifiers (%s, etc.) will expand
# when displayed, so remember to take this into account; you can assume
# that the length of a pseudoclient name (ex. %NS replacement) will be eight
# characters, so reduce the maximum line length by 6 for every %xS on a
# line.
#
# To translators or those brave enough to edit this (and risk crashing):
# The following specifiers are available:
#   - %R    Will be translated to "/" or "/msg", depending on UseStrictPrivMsg.
#   - %F    Will be translated to the BSFantasyCharacter.
#   - %NS   Will be translated to the NickServ nick.
#   - %CS   Will be translated to the ChanServ nick.
#   - %OS   Will be translated to the OperServ nick.
#   - %BS   Will be translated to the BotServ nick.
#   - %MS   Will be translated to the MemoServ nick.
#   - %HS   Will be translated to the HostServ nick.
#
# Finally, remember to put a tab at the beginning of every line of text
# (including empty lines).  This has to be a tab, not spaces.
#
##############################################################################

LANG_NAME
	Français

LANG_HELP
	SYNTAX: %shelp
	Voici l'aide pour les commandes fantasy botserv.
LANG_CLEAR_SYNTAX
	SYNTAX: %sclear [modes|bans|excepts|invites|ops|hops|voices|users]
LANG_EXCEPTS_UNSUPPORTED
	Les Chans Excepts ne sont pas supportés sur ce réseau.
LANG_INVITEMODE_UNSUPPORTED
	Les Chans Invites ne sont pas supportés sur ce réseau.
LANG_HOPS_UNSUPPORTED
	Le Grade Halfops n'est pas supporté par ce réseau.
LANG_SOP_SYNTAX
	SYNTAX: %ssop {add|del|list|clear} [pseudo|entry-list]
LANG_SOP_SYNTAX_EXT
	SYNTAX: %ssop add [pseudo]
	        %ssop del [pseudo|num de list|list]
	        %ssop list [mask|list]
	        %ssop clear"
	
	Maintains the SOP (SuperOP) list for the channel.
	The SOP list gives users all rights given by the AOP list, and adds
	those needed to use the AutoKick and the BadWords lists,
	to send and read channel memos, and so on.
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.
LANG_AOP_SYNTAX
	SYNTAX: %saop {add|del|list|clear} [pseudo|entry-list]
LANG_AOP_SYNTAX_EXT
	SYNTAX: %saop add [pseudo]
	        %saop del [pseudo|num de list|list]
	        %saop list [mask|list]
	        %saop clear"
	
	Maintains the AOP (AutoOP) list for the channel.
	The AOP list gives users the right to be auto-opped on your channel,
	to unban or invite themselves if needed, to have their
	greet message showed on join, and so on.
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.
LANG_HOP_SYNTAX
	SYNTAX: %shop {add|del|list|clear} [pseudo|entry-list]
LANG_HOP_SYNTAX_EXT
	SYNTAX: %shop add [pseudo]
	       %shop del [pseudo|num de list|list]
	       %shop list [mask|list]
	       %shop clear"
	
	Maintains the HOP (HalfOP) list for the channel. The HOP
	list gives users the right to be auto-halfopped on your channel
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.
LANG_VOP_SYNTAX
	SYNTAX: %svop {add|del|list|clear} [pseudo|entry-list]
LANG_VOP_SYNTAX_EXT
	SYNTAX: %svop add [pseudo]
	       %svop del [pseudo|num de list|list]
	       %svop list [mask|list]
	       %svop clear"
	
	Maintains the VOP (VOicePeople) list for a channel.
	The VOP list allows users to be auto-voiced and to voice
	themselves if they aren't.
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.
LANG_ACCESS_SYNTAX
	SYNTAX: %saccess {add|del|list|clear} [pseudo [level]|entry-list]
LANG_ACCESS_SYNTAX_EXT
	SYNTAX: %saccess add [pseudo] [level]
	       %saccess del [pseudo|num de list|list]
	       %saccess list [mask|list]"
	       %saccess clear"
	
	Maintains the access list for a channel.  The access
	list specifies which users are allowed chanop status or
	access to commands on the channel.
	Different user levels allow for access to different subsets of
	privileges; cf. HELP ACCESS LEVELS for more specific information.
	Any nick not on the access list has a user level of 0.
LANG_LEVELS_SYNTAX
	SYNTAX: %slevels {set|dis[able]|list|reset} [item [level]]
LANG_LEVELS_SYNTAX_EXT
	SYNTAX: %slevels set [type] [level]
	       %slevels {dis|disable} [type]
	       %slevels list
	       %slevels reset"
	
	The LEVELS command allows fine control over the meaning of
	the numeric access levels used for the channel.  With this command,
	you can define the access level required for most administrative functions.
	(The SET FOUNDER and SET PASSWORD commands, as well as this command, 
	are always restricted to the channel founder.) 
	
	LEVELS SET allows the access level for a function or group of functions to be changed.
	LEVELS DISABLE (or DIS for short) disables an automatic
	feature or disallows access to a function by anyone other than the channel founder.
	LEVELS LIST shows the current levels for each function or group of functions.
	LEVELS RESET resets the levels to the default levels of a newly-created channel.
	(See HELP ACCESS LEVELS for more info.)
	
	For a list of the features and functions whose levels can be
	set, see HELP LEVELS DESC.
LANG_LEVELS_ERR_NO_VAL
	Erreur: Pas de valeur donnée au paramètre SET.
LANG_LEVELS_ERR_VAL
	Erreur: Valeur donnée non-numérique.
LANG_ONLY_FOUNDERS
	Accès refusé : Seuls les Owners peuvent utiliser cette commande.
LANG_GOLIST_OPER_ONLY
	IRCop
LANG_GOLIST_OPER_AND_SO
	IRCop + Services
LANG_GOLIST_OPER_AND_SA
	IRCop + Services Administrateur
LANG_GOLIST_OPER_AND_SRA
	IRCOperateur + Services Root Administrateur
LANG_GOLIST_NONE
	Pas d'IRCops disponiblent pour l'instant
LANG_GOLIST_HEADER
	-----------------------------------------------------------------------
	Les IRCop suivant sont en ligne:
	-----------------------------------------------------------------------
LANG_GOLIST_FOOTER
	-----------------------------------------------------------------------
	Total d'IRCops disponiblent : %d 
	-----------------------------------------------------------------------
	-> Toujours essayer de contacter un IRCop sur un chan avant de le PV
	-> Eviter de PV un IRCop sans avoir sa permission avant.
	-> faire un /whois sur IRCop avant de vouloir lui parler.
	-> Lire (et respecter) un message d'away présent dans le Whois.
	-----------------------------------------------------------------------
LANG_AKICK_SYNTAX
	SYNTAX: %sakick {add|stick|unstick|del|list|view|enforce|clear} [pseudo-or-usermask] [raison]
LANG_AKICK_SYNTAX_EXT
	SYNTAX: %sakick add [mask | user] [raison]
	       %sakick {stick|unstick|del} mask
	       %sakick {list|view} [mask]
	       %sakick {enforce|clear}
LANG_BADWORDS_SYNTAX
	SYNTAX: %sbadwords {add|del|list|clear} [mot|entry-list] [single|start|end|any]
LANG_BADWORDS_SYNTAX_EXT
	SYNTAX: %sbadwords add [mot] {single|start|end|any}
	SYNTAX: %sbadwords del [mot|num de list|list]
	SYNTAX: %sbadwords list [mask|list]
	SYNTAX: %sbadwords clear
LANG_BADWORDS_ERR_STYLE
	Erreur: Style invalide. Utiliser uniquement START, SINGLE, ANY ou END.
LANG_CMD_NOT_AV
	Cette commande n'est pas disponible. Contacter le staff du réseau pour plus d'informations.
LANG_SET_SYNTAX
	SYNTAX: %sset [option] [parametres]
	Available options: founder, successor, desc, email, url, entrymsg, bantype, mlock,
	       keeptopic, topiclock, opnotice, peace, private, restricted, secure, secureops,
	       securefounder, signkick, xop, noexpire (limited to SA)
	       dontkickops, dontkickvoices, greet, fantasy, symbiosis, nobot (limited to SA)
LANG_SET_SYNTAX_EXT
	SYNTAX: %sset [option] [parameters]
	Available options:
	       founder       -  Désigne le nouvel Owner du Chan
	       successor     -  Désigne le successeur de l'Owner pour un Chan
	       desc          -  Règle la description du Chan
	       url           -  Associe une URL au Chan
	       email         -  Associe un E-mail au Chan
	       entrymsg      -  Règle le message envoyé aux utilisateurs quand ils entrent dans un Chan
	       bantype       -  Regle le type de ban que les services appliquent
	       mlock         -  Verrouille les modes de chan choisis
	       keeptopic     -  Maintient le Topic quand le Chan n'est pas utilisé
	       topiclock     -  Le Topic peut seulement être modifié avec TOPIC
	       opnotice      -  Envoie une notice quand la commande OP/DEOP est utilisée
	       peace         -  Régule l'usage des commandes de sanction
	       private       -  Cache le chan dans la commande LIST
	       restricted    -  Restreint l'accès du Chan
	       secure        -  Active les options de sécurité des services
	       secureops     -  Contrôle strict du statut d'op
	       securefounder -  Contrôle strict du statut d'owner
	       signkick      -  Signature des kicks qui ne sont pas faits par la commande KICK
	       xop           -  Met le système d'accès en mode XOP
	       noexpire      -  Règle l'expiration du Chan. (limité au Service Admin)
	
	       dontkickops   -  Protège les ops des kicks par les services.
	       dontkickvoices - Protège les voices des kicks par les services.
	       greet         -  Autorise les messages greet quand on rejoint un Chan.
	       fantasy       -  Autorise les commandes fantasy sur le Chan. (Lire l'aide avant !)
	       symbiosis     -  Autorise le bot à agir comme un vrai bot.
	       nobot         -  Empêche l'assignation d'un bot sur le Chan. (limité au Service Admin)
LANG_SET_FOUNDER_SYNTAX
	SYNTAX: %sset founder [pseudo]
LANG_SET_FOUNDER_SYNTAX_EXT
	SYNTAX: %sset founder [pseudo]
	Change le propriétaire du Chan. Le nouveau propriétaire doit être un pseudo enregistré.
LANG_SET_SUCCESSOR_SYNTAX
	SYNTAX: %sset successor [pseudo]
LANG_SET_SUCCESSOR_SYNTAX_EXT
	SYNTAX: %sset successor [pseudo]
	Change le successeur du propriétaire. Si le pseudo du propriétaire expire ou est droppé
	Tant que le Chan est enregistré le successeur peut devenir le propriétaire du Chan.
	Le pseudo du Successeur doit être enregistré.
LANG_SET_DESC_SYNTAX
	SYNTAX: %sset desc [description]
LANG_SET_DESC_SYNTAX_EXT
	SYNTAX: %sset desc [description]
	Règle la description du Chan, elle se voit sur la commande LIST et INFO.
LANG_SET_URL_SYNTAX
	SYNTAX: %sset url [url]
LANG_SET_URL_SYNTAX_EXT
	SYNTAX: %sset url [url]
	Associe L'URL donnée au Chan.
	Cette URL sera visible sur la commande INFO du Chan.
	Si aucune URL n'est precisée, cela effacera toute URL associée au Chan.
LANG_SET_EMAIL_SYNTAX
	SYNTAX: %sset email [email]
LANG_SET_EMAIL_SYNTAX_EXT
	SYNTAX: %sset email [email]
	Associe L'E-mail donné au Chan.
	Cet E-mail sera visible sur la commande INFO du Chan.
	Si aucun E-mail n'est precisé, cela effacera tout E-mail associé au Chan.
LANG_SET_ENTRYMSG_SYNTAX
	SYNTAX: %sset entrymsg [Message d'entré]\n
LANG_SET_ENTRYMSG_SYNTAX_EXT
	SYNTAX: %sset entrymsg [Message d'entré]
	Règle le message qui sera envoyé par /notice aux utilisateurs qui rejoindront le chan.
	Si aucun message n'est precisé, il n'y aura pas de notice envoyée.
LANG_SET_BANTYPE_SYNTAX
	SYNTAX: %sset bantype [0|1|2|3]
LANG_SET_BANTYPE_SYNTAX_EXT
	SYNTAX: %sset bantype [0|1|2|3]
	Règle le type de ban utilisé par les services quand ils auront besoin de bannir un utilisateur du Chan.
	
	Le type de ban sera un nombre entre 0 et 3 qui signifie :
	0: ban de cette forme *!user@host
	1: ban de cette forme *!*user@host
	2: ban de cette forme *!*@host
	3: ban de cette forme *!*user@*.domain
LANG_SET_MLOCK_SYNTAX
	SYNTAX: %sset mlock [modes]
LANG_SET_MLOCK_SYNTAX_EXT
	SYNTAX: %sset mlock [modes]
	Règle le mode-lock pour le Chan.
	Ceci vous permet de définir les modes de Chan qui seront maintenus.\n
LANG_SET_KEEPTOPIC_SYNTAX
	SYNTAX: %sset keeptopic [on|off]
LANG_SET_KEEPTOPIC_SYNTAX_EXT
	SYNTAX: %sset keeptopic [on|off]
	Met ou Enlève le maintien du Topic d'un Chan. Quand l'option KEEPTOPIC est activée,
	le topic du Chan sera gardé après que le dernier utilisateur quitte le Chan,
	et sera restauré à la prochaine création du Chan.
LANG_SET_TOPICLOCK_SYNTAX
	SYNTAX: %sset topiclock [on|off]
LANG_SET_TOPICLOCK_SYNTAX_EXT
	SYNTAX: %sset topiclock [on|off]
	Met ou enlève le verrouillage du topic pour un Chan. Quand l'option TOPICLOCK est activée,
	les services ne permettront pas la modification du Topic sur le Chan.
LANG_SET_OPNOTICE_SYNTAX
	SYNTAX: %sset opnotice [on|off]
LANG_SET_OPNOTICE_SYNTAX_EXT
	SYNTAX: %sset opnotice [on|off]
	Met ou enlève la notice d'Op pour un Chan. Quand l'option OPNOTICE est activée,
	les services enverront une notice au Chan pour prévenir de l utilisation de la commande OP/DEOP.
LANG_SET_PEACE_SYNTAX
	SYNTAX: %sset peace [on|off]
LANG_SET_PEACE_SYNTAX_EXT
	SYNTAX: %sset peace [on|off]
	Met ou enlève l'option peace pour le Chan. Quand l'option PEACE est activée,
	un utilisateur ne pourra pas kicker, bannir ou enlever le statut d'un utilisateur qui aura
	un niveau supérieur ou égal au sien.
LANG_SET_PRIVATE_SYNTAX
	SYNTAX: %sset private [on|off]
LANG_SET_PRIVATE_SYNTAX_EXT
	SYNTAX: %sset private [on|off]
	Met ou enlève l'option private pour le Chan. Quand l'option PRIVATE est activée,
	la commande /%s LIST ne montrera le Chan dans aucune liste.
LANG_SET_RESTRICTED_SYNTAX
	SYNTAX: %sset restricted [on|off]
LANG_SET_RESTRICTED_SYNTAX_EXT
	SYNTAX: %sset restricted [on|off]
	Met ou enlève l'option de restriction pour un Chan. Quand l'option RESTRICTED est activée,
	les utilisateurs qui n'ont pas d'accès Opérateur
	(utilisateurs avec accès négatif et, si secure ops est actif, utilisateurs sans accès)
	seront immediatement kickés et bannis du Chan.
LANG_SET_SECURE_SYNTAX
	SYNTAX: %sset secure [on|off]
LANG_SET_SECURE_SYNTAX_EXT
	SYNTAX: %sset secure [on|off]
	Met ou enlève l'option de sécurité ChanServ pour un Chan. Quand l'option SECURE est activée,
	Seuls les utilisateurs qui ont un pseudo enregistré et identifié aux services, 
	auront un accès au Chan sous contrôle de la liste d'accès.
LANG_SET_SECUREOPS_SYNTAX
	SYNTAX: %sset secureops [on|off]
LANG_SET_SECUREOPS_SYNTAX_EXT
	SYNTAX: %sset secureops [on|off]
	Met ou enlève l'option secure ops option pour un Chan. Quand l'option SECUREOPS est activée,
	les utilisateurs qui ne sont pas dans la liste d'accès,
	ne pourront pas avoir le statut d'Op.
LANG_SET_SECUREFOUNDER_SYNTAX
	SYNTAX: %sset securefounder [on|off]
LANG_SET_SECUREFOUNDER_SYNTAX_EXT
	SYNTAX: %sset securefounder [on|off]
	Met ou enlève l'option secure founder pour un Chan. Quand l'option SECURE FOUNDER est activée,
	seul le propriétaire et son successeur pourront dropper le Chan, changer son password,
	pas les propriétaires identifiés auprés des services.
LANG_SET_SIGNKICK_SYNTAX
	SYNTAX: %sset signkick [on|off]
LANG_SET_SIGNKICK_SYNTAX_EXT
	SYNTAX: %sset signkick [on|off]
	Met ou enlève la signature des kicks pour un Chan. Quand l'option SIGNKICK est activée, les kicks mis
	par les services afficheront le pseudo et la raison de celui qui utilise la commande de kick.
	Si vous utilisez les LEVELS, ceux qui auront un level supérieur ou égal au level SIGNKICK
	n'auront pas de kick signé sur le Chan. Voir '%shelp levels' pour plus d'information.
LANG_SET_XOP_SYNTAX
	SYNTAX: %sset xop [on|off]
LANG_SET_XOP_SYNTAX_EXT
	SYNTAX: %sset xop [on|off]
	Met ou enlève le système xOP pour un Chan. Quand l'option XOP est activée, vous devez utiliser les commandes
	AOP/SOP/VOP pour donner les accès aux utilisateurs, autrement utiliser la commande ACCESS.\n
LANG_SET_NOEXPIRE_SYNTAX
	SYNTAX: %sset noexpire [on|off]
LANG_SET_NOEXPIRE_SYNTAX_EXT
	SYNTAX: %sset noexpire [on|off]
	Règle le paramètre d'expiration du Chan. Quand l'option NOEXPIRE est activée le Chan n'expirera pas.
	
	Cette commande est réservée aux Administrateurs de Services.
LANG_TOPIC_SYNTAX
	SYNTAX: %stopic [topic]
	SYNTAX: %sctopic [topic]
LANG_TOPIC_SYNTAX_EXT
	SYNTAX: %stopic [topic]
	SYNTAX: %sctopic
	Sets the channel topic. If no topic is given, the current topic is displayed in the channel.
	The CTOPIC command is provided to clear the topic.
	Cette commande s'utilise avec SET TOPICLOCK
LANG_INVITE_SYNTAX
	SYNTAX: %sinvite [pseudo]
LANG_INVITE_SYNTAX_EXT
	SYNTAX: %sinvite [pseudo]
	Invite 'Pseudo' sur le Chan si celui ci est présent sur le réseau
LANG_INVITE_NO_USER
	Cet utilisateur n'est pas connecté.
LANG_INVITE_IS_ON
	Cet utilisateur est déjà sur le Chan !
LANG_INVITE_YOURSELF
	Vous ne pouvez pas vous inviter vous même !
LANG_CMD_LIST_HEADER
	Commandes Fantasy disponiblent :
LANG_CMD_LIST_FOOTER
	Toutes les fonctions de services ne sont pas détaillées ici...
LANG_CMD_LIST_GENERAL
	Commandes général: %s
LANG_CMD_LIST_CHAN_AXX
	Commandes de l'accès au Chan: %s
LANG_CMD_LIST_CHG_SET
	Commandes de réglage de Chan: %s
LANG_CMD_LIST_ACTIONS
	Commandes d'actions sur le Chan: %s
LANG_CMD_LIST_IRCOP
	Commandes IRC Operateur: %s
LANG_UNKNOWN_CMD
	Commandes inconnues: %s
LANG_HELP_HEADER
	----- Aide pour les commandes Fantasy -----
LANG_HELP_FOOTER
	----- Fin de la liste des commandes -----
LANG_APPENDTOPIC_SYNTAX
	SYNTAX: %sappendtopic ou %stappend [texte à ajouter au topic]
LANG_APPEND_SYNTAX_EXT
	SYNTAX: %sappendtopic ou %stappend [texte à ajouter au topic]
	Ajoute le texte donné au topic actuel du Chan.
	Cette commande s'utilise avec SET TOPICLOCK
LANG_STAFF
	SYNTAX: %sstaff ou %sircops
	Cette commande donne la liste des IRC Operateurs disponibles.
LANG_CMDLIST
	SYNTAX: %scmdlist
	Montre la liste des commandes Fantasy
LANG_MINFO
	SYNTAX: %sminfo (ou %smodinfo)
	Montre les informations du module des commandes Fantasy.
LANG_HELP_UNKNOWN_CMD
	Pas d'aide disponible pour cette commande.
LANG_BAN_SYNTAX
	SYNTAX: %sban [pseudo|mask]
LANG_BAN_SYNTAX_EXT
	SYNTAX: %sban [pseudo|mask]
	Banni le pseudo ou le mask donné du Chan.
LANG_MUTE_SYNTAX
	SYNTAX: %smute [pseudo|mask] (dépend de l'IRCD)
LANG_MUTE_SYNTAX_EXT
	SYNTAX: %smute [pseudo|mask]
	Banni ou rend muet le pseudo ou le mask donné pour le Chan. (dépend de L'IRCD)
LANG_UNMUTE_SYNTAX
	SYNTAX: %sunmute [pseudo|mask] (dépend de l'IRCD)
LANG_UNMUTE_SYNTAX_EXT
	SYNTAX: %sunmute [pseudo|mask]
	Debanni ou enlève Le mode muet sur le pseudo ou le mask donné pour le Chan. (dépend de l'IRCD)
LANG_SVERSION
	SYNTAX: %ssversion
	Montre les informations sur la version et le build des Services Anope.
LANG_KILL_SYNTAX
	SYNTAX: %skill [pseudo] [raison]
LANG_KILL_SYNTAX_EXT
	SYNTAX: %skill [pseudo] [raison]
	Cette commande force l'utilisateur à se deconnecter du réseau (identique à /kill).
	IRC Opérateurs seulement. Cette commande peut être désactivée
LANG_MODE_SYNTAX
	SYNTAX: %smode [mode(s)]
LANG_MODE_SYNTAX_EXT
	SYNTAX: %smode [mode(s)]
	Cette commande modifie les modes de Chan.
	IRC Opérateurs seulement. Cette commande peut être désactivée
LANG_AKILL_SYNTAX
	SYNTAX: %sakill AKILL {add|del|list|view|clear} [[+expiry] {mask|entry-list} [raison]]
LANG_AKILL_SYNTAX_EXT
	SYNTAX: %sakill add [+expiry] {mask} raison
	SYNTAX: %sakill del {mask|num de list|list}
	SYNTAX: %sakill [list|view] [mask|list]
	SYNTAX: %sakill clear
	IRC Opérateurs seulement. Cette commande peut être désactivée
LANG_IGNORE_SYNTAX
	Syntax: %signore {add|del|list|clear} [time] [pseudo]
LANG_IGNORE_SYNTAX_EXT
	SYNTAX: %signore {add|del} [expiry] [pseudo]
	SYNTAX: %signore {list|clear}
	Autorise les Services Admins à rendre un pseudo ignoré par les services pour
	un certain temps ou jusqu au redémarrage. Le temps par défaut
	à un format en secondes. Les combinaisons d'unité de temps ne sont pas permises.
	Pour que les services ignorent indéfiniment, utiliser 0 pour le temps.
LANG_INFO_SYNTAX
	Syntax: %sinfo [all]
LANG_INFO_SYNTAX_EXT
	SYNTAX: %sinfo [all]
	Donne la liste des informations pour le Chan.
	L'option 'all' est limitée au propriétaire et aux Services Admins.
LANG_CMD_DISABLED
	Cette commande a été désactivée, Merci de contacter Le staff du réseau pour plus d'informations.
LANG_HELP_CMDS_GEN
	Commandes Générales :
	  %shelp      -   Aide de botserv pour les commandes
	  %scmdlist   -   Montre la liste de toutes les commandes FANTASY
	  %sircops    -   Montre la liste de tous les IRCops disponibles
	  %sstaff     -   Montre la liste de tous les IRCops disponibles
	  %sminfo     -   Montre les informations à propos du module
	  %ssversion  -   Montre les informations sur Anope en général
	  %sinfo      -   Montre les informations à propos du Chan
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint\n
LANG_HELP_CMDS_ACC
	Commandes de Contrôle d'Accès:\n
	  %ssop       -   Permet de gérer la liste des SOP (SuperOP)
	  %saop       -   Permet de gérer la liste des AOP (AutoOP)
	  %shop       -   Permet de gérer la liste des HOP (HalfOP)(peut ne pas être géré par le réseau)
	  %svop       -   Permet de gérer la liste des VOP (VOicePeople)
	  %saccess    -   Permet de gérer la liste des AXX (quand XOP est désactivé)
	  %slevels    -   Permet de gérer les levels du Chan
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint\n
LANG_HELP_CMDS_SETTINGS
	Commands de Règlage du chan: 
	  %sakick     -   Gère la liste de akick 
	  %sbadwords  -   Gère la liste des mots interdits 
	  %sset       -   Change les réglages du Chan. (cf. '%shelp set')
	  %sbkick     -   Change le réglage des kicks Botserv.(cf. '%shelp bkick')
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint\n
LANG_HELP_CMDS_ACTIONS
	Commandes d'Action du Chan:
	  %sclear     -   Efface l'option spécifiée sur le chan.
	  %stopic     -   Règle le topic du chan
	  %stappend   -   Ajoute le texte au topic
	  %sinvite    -   Invite un utilisateur sur le chan
	  %sup        -   Attribue les grades auxquels l'utilisateur a accès
	  %sdown      -   Enlève tous les grades d'un utilisateur
	  %skick      -   Kick un utilisateur sur le Chan
	  %skb        -   Kick et ban un ou des utilisateur(s) sur le Chan
	  %stkb       -   Kick et temporaire ban un ou des utilisateur(s) sur le Chan
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint\n
LANG_HELP_CMDS_PUTMODES
	Commands de Mode du Chan:
	  %sban       -   Bannit l'utilisateur sur le Chan sans le kicker.
	  %stban      -   Bannit l'utilisateur temporaire sur le Chan sans le kicker.
	  %sunban     -   Débannit l'utilisateur sur le Chan
	  %smute      -   Bannit l'utilisateur ou le rend muet (dépend de L'IRCD)
	  %sunmute    -   Débannit l'utilisateur ou lève le mode muet (dépend de L'IRCD)
	  %sop        -   +o: Op vous-même ou un autre utilisateur
	  %sdeop      -   -o: Deop vous-même ou un autre utilisateur
	  %svoice     -   +v: Voice vous-même ou un autre utilisateur
	  %sdevoice   -   -v: Devoice vous-même ou un autre utilisateur
	  %sprotect   -   +a: Protect vous-même ou un autre utilisateur (Peut ne pas être disponible)
	  %sdeprotect -   -a: Deprotect vous-même ou un autre utilisateur ((Peut ne pas être disponible)
	  %sowner     -   +q: Attribue le grade owner (Peut ne pas être disponible)
	  %sdeowner   -   -q: Enlève votre grade d'owner (Peut ne pas être disponible)
	  %shalfop    -   +h: Halfop vous-même ou un autre utilisateur (Peut ne pas être disponible)
	  %sdehalfop  -   -h: Dehalfop vous-même ou un autre utilisateur (Peut ne pas être disponible)
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint\n
LANG_HELP_CMDS_IRCOP
	Commandes IRC Operateur :
	  %sakill     -   Force un ban sur le mask pour tout le serveur.
	  %skill      -   Deconnecte l'utilisateur
	  %smode      -   Règle les modes de Chan
	  %signore    -   Règle les services pour ignorer un utilisateur
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint\n
LANG_HELP_MORE_INFO
	Pour avoir de l'aide sur une commande %shelp <commande>
LANG_HELP_LIST
	Pour avoir une liste des commandes possibles pour ce que vous voulez faire, tapez: 
	
	 %shelp general  - Montre les commandes générales disponibles. Aucun accès requis.
	 %shelp chaccess - Montre les commandes pour modifier la liste d'accès ou des levels d'un chan.
	 %shelp config   - Configure les réglages d'un chan.
	 %shelp act      - Liste des commandes d'action possibles par les services.
	 %shelp ircop    - Commandes vablables seulement pour les IRC Operateurs.
	
	Pour avoir la liste de toutes les commandes disponibles, tape '%shelp all'
LANG_SET_DONTKICKOPS_SYNTAX
	SYNTAX: %sset dontkickops {on | off}
LANG_SET_DONTKICKOPS_SYNTAX_EXT
	SYNTAX: %sset dontkickops {on | off}
	Active ou désactive la protection des ops sur un chan. Quand l'option DONTKICKOPS est activée,
	cette option empêche le bot de kicker les ops même s'ils ne sont pas concernés
	par le level NOKICK.
LANG_SET_DONTKICKVOICES_SYNTAX
	SYNTAX: %sset dontkickvoices {on | off}
LANG_SET_DONTKICKVOICES_SYNTAX_EXT
	SYNTAX: %sset dontkickvoices {on | off}
	Active ou désactive la protection des voices sur un chan. Quand l'option DONTKICKVOICES est activée,
	cette option empêche le bot de kicker les voices même s'ils ne sont pas concernés 
	par le level NOKICK.
LANG_SET_GREET_SYNTAX
	SYNTAX: %sset greet [on|off]
LANG_SET_GREET_SYNTAX_EXT
	SYNTAX: %sset greet {on | off}
	Active ou désactive le  Message d'Accueil sur un chan. Quand l'option GREET est activée,
	elle fait afficher au bot le message d'accueil de chaque utilisateur qui a un accès suffisant au chan.
LANG_SET_FANTASY_SYNTAX
	SYNTAX: %sset fantasy [on|off]
LANG_SET_FANTASY_SYNTAX_EXT
	SYNTAX: %sset fantasy {on | off}
	Active ou désactive le mode FANTASY sur un chan. Quand l'option FANTASY est activée, cette option
	permet aux utilisateurs d'utiliser les commandes !op, !deop, !voice, !devoice, !kick, !kb, !unban.
	Noter que les utilisateurs qui veulent utliliser les commandes FANTASY doivent avoir 
	suffisamment d'accès pour les levels FANTASIA et le level qui dépend de la commande
	
	IMPORTANT: Désactiver cette option désactivera les commandes FANTASY
LANG_SET_SYMBIOSIS_SYNTAX
	SYNTAX: %sset symbiosis [on|off]
LANG_SET_SYMBIOSIS_SYNTAX_EXT
	SYNTAX: %sset symbiosis {on | off}
	Active ou désactive la symbiose sur un chan. Quand l'option SYMBIOSIS est activée,
	cette option fait faire au bot tout ce qui est normalement effectué par BotServ sur les chans,
	tels que les MODEs,les KICKs, et même le message d'entrée. 
	Note: Quand l'option est désactivée, la plupart des commandes fantasy seront possibles avec le bot.
LANG_SET_NOBOT_SYNTAX
	SYNTAX: %sset nobot [on|off]
LANG_SET_NOBOT_SYNTAX_EXT
	SYNTAX: %sset nobot {on | off}
	Cette option fait qu'un bot ne peut pas être assigné sur le Chan. Si un bot est 
	quand même assigné, il sera désassigné automatiquement si cette option est activée.
LANG_BKICK_SYNTAX
	SYNTAX: %sbkick [option] {on | off} [ttb [params]]
	Options disponibles: bolds, badwords, caps, colors, flood, repeat, reverses, underlines.
LANG_BKICK_SYNTAX_EXT
	SYNTAX: %sbkick [option] {on | off} [ttb [params]]
	Available options:
	       bolds       -  Règle le bot pour kicker les Gras.
	       badwords    -  Règle le bot pour kicker les Mots Interdits.
	       caps        -  Règle le bot pour kicker les Majuscules.
	       colors      -  Règle le bot pour kicker les Couleurs.
	       flood       -  Règle le bot pour kicker le flood.
	       repeat      -  Règle le bot pour kicker les Répétitions.
	       reverses    -  Règle le bot pour kicker les inversés.
	       underlines  -  Règle le bot pour kicker les Soulignés.
	
	ttb est le nombre de fois qu'un utilisateur peut être kické 
	avant d'être banni. Laissez ttb vierge pour désactiver
LANG_BKICK_BOLDS_SYNTAX
	SYNTAX: %sbkick bolds {on | off} [ttb]
LANG_BKICK_BOLDS_SYNTAX_EXT
	SYNTAX: %sbkick bolds {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes 
	qui écrivent en gras. les services enveront une notice quand la commande OP DEOP sera utilisée.
	ttb est le nombre de fois qu'un utilisateur peut être kicker avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_BADWORDS_SYNTAX
	SYNTAX: %sbkick badwords {on | off} [ttb]
LANG_BKICK_BADWORDS_SYNTAX_EXT
	SYNTAX: %sbkick badwords {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui utilisent 
	des mots interdits. Vous pouvez definir la liste de mots interdits en tapant %sbadwords command.
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_CAPS_SYNTAX
	SYNTAX: %sbkick caps {on | off} [ttb [min [percent]]]
LANG_BKICK_CAPS_SYNTAX_EXT
	SYNTAX: %sbkick caps {on | off} [ttb [min [percent]]]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui 
	écrivent en majuscule. Le bot kicke si il y a minimum min majuscule caps 
	et qu'elles constituent au moins percent% de la ligne de texte entière 
        "(si ces paramètres ne sont pas donnés, c'est par défaut 10 caractères et 25%).
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_COLORS_SYNTAX
	SYNTAX: %sbkick colors {on | off} [ttb]
LANG_BKICK_COLORS_SYNTAX_EXT
	SYNTAX: %sbkick colors {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui ecrivent en couleur.
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_FLOOD_SYNTAX
	SYNTAX: %sbkick flood {on | off} [ttb [ln [secs]]]
LANG_BKICK_FLOOD_SYNTAX_EXT
	SYNTAX: %sbkick flood {on | off} [ttb [ln [secs]]]
	Quand l'option est selectionnée, le bot pourra kicker les personnes 
	qui flood sur le chan en ecrivant ln lignes en secs secondes. 
	(si vous ne paramétrez pas, c'est 6 lignes en 10 secondes par défaut).
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_REPEAT_SYNTAX
	SYNTAX: %sbkick repeat {on | off} [ttb [num]]
LANG_BKICK_REPEAT_SYNTAX_EXT
	SYNTAX: %sbkick repeat {on | off} [ttb [num]]
	Quand l'option est selectionnée, le bot pourra kicker les personnes 
	qui se répètent num fois (si num n'est pas designé, c'est 3 par défaut)."
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban.
LANG_BKICK_REVERSES_SYNTAX
	SYNTAX: %sbkick reverses {on | off} [ttb]
LANG_BKICK_REVERSES_SYNTAX_EXT
	SYNTAX: %sbkick reverses {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui ecrivent en inversé.
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban.
LANG_BKICK_UNDERLINES_SYNTAX
	SYNTAX: %sbkick underlines {on | off} [ttb]
LANG_BKICK_UNDERLINES_SYNTAX_EXT
	SYNTAX: %sbkick underlines {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui ecrivent en souligné.  
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban.
LANG_BKICK_UNKNOWN_OPTION
	Option BKICK inconnue: %s.
LANG_ADLIST_SA
	Services Administrateur
LANG_ADLIST_SRA
	Services Root Administrateur
LANG_ADLIST_NONE
	Aucun administrateur disponible actuellement
LANG_ADLIST_HEADER
	-----------------------------------------------------------------------
	Les Administrateurs suivant sont en ligne: 
	-----------------------------------------------------------------------
LANG_ADLIST_FOOTER
	-----------------------------------------------------------------------
	Total d'Admins disponibles : %d 
	-----------------------------------------------------------------------
	-> Toujours essayer de contacter un Admin sur un chan avant de le PV 
	-> Eviter de PV un Admin sans avoir sa permission avant. 
	-> faire un /whois sur Admin avant de vouloir lui parler. 
	-> Lire (et respecter) un message d'away present dans le Whois. 
	-----------------------------------------------------------------------
LANG_KICKBAN_SYNTAX
	SYNTAX: %skickban or %skb [pseudo | [pseudo!]ident@host] [raison]
LANG_KICKBAN_SYNTAX_EXT
	SYNTAX: %skickban or %skb [pseudo | [pseudo!]ident@host] [raison]
	Ceci kickera et bannira la personne désignée ou toutes les personnes ayant 
	le même mask sur le Chan. Si un mask est donné et que quelqu'un ne devrait   
	pas être kické' à cause des réglages du chan, la commande est anulée. 
	Personne n'est kické ou banni.
LANG_UP_SYNTAX
	SYNTAX: %sup [pseudo]
LANG_UP_SYNTAX_EXT
	SYNTAX: %sup [pseudo]
	Donne à l'utilisateur les modes auquel il a droit. Ceci ne donnera pas de grade si
	l'utilisateur n'a pas d'accès.
	
	Note: Un pseudo peut seulement etre specifié par le owner du Chan ou un SRA.
LANG_DOWN_SYNTAX
	SYNTAX: %sdown [pseudo]
LANG_DOWN_SYNTAX_EXT
	SYNTAX: %sdown [pseudo]
	Enleve tout les modes au pseudo. Si le Chan est modéré, cela enlevera les voices,
	jusqu'a ce qu'un utilisateur soit de nouveaux designé par l'owner.
	
	Note: Un pseudo peut seulement etre specifié par le owner du Chan ou un SRA.
LANG_UPDOWN_SYNTAX
	SYNTAX: %sup|%sdown [pseudo]
LANG_TBAN_SYNTAX	*/
	SYNTAX: %stb or %stban [pseudo | [pseudo!]ident@host] [time]
LANG_TBAN_SYNTAX_EXT
	SYNTAX: %stb or %stban [pseudo | [pseudo!]ident@host] [time]
	Ceci bannira le pseudo ou le mask specifié dans le Chan pour
	la durée précisée. si le temp n'est pas donné, il sera d'une heure par defaut.
	le temp devra etre donné dans le format suivant <nombre><(s)/m/h/d>.
	Exemple: %stb Viper 10m ceci me bannira pour 10 minutes.
	
	Note: Si les services sont redemarrés avant l'expiration du ban,
	il ne sera pas automatiquement enlevé
LANG_TBAN_RESPONSE
	Le ban temporaire %s de %s, expirera dans %s.
LANG_TKBAN_SYNTAX
	SYNTAX: %stkb [pseudo | [pseudo!]ident@host] [temp] [raison]
LANG_TKBAN_SYNTAX_EXT
	SYNTAX: %stkb or %stkban [pseudo | [pseudo!]ident@host] [temp] [raison]
	Ceci Kickera et bannira l'utilisateur spécifié ou tout ceux qui correspondront
	au mask donné dans le Chan pour le temps donné. 
	Si un mask est donné et qu'un utilisateur qui correspond aux
	reglages de Chan ne doit pas etre banni, La commande sera annulé avant que le premier kick sois fait.
	Si le temps n'est pas precisé ou donné dans un mauvais format, il sera d'une heure par defaut.
	le temp devra etre donné dans le format suivant <nombre><(s)/m/h/d>.
	Exemple: %stb Viper 10m ceci me kickera et bannira pour 10 minutes.
	
	Note: Si les services sont redemarrés avant l'expiration du ban
	il ne sera pas automatiquement enlevé
LANG_INVALID_TIME
	%s et un format de temp invalide.
LANG_REQ_NICK_OR_MASK
	Cette commande requierre un pseudo ou un mask user@host valide.
LANG_SUSPEND_SYNTAX
	SYNTAX: %ssuspend [reason]
LANG_SUSPEND_SYNTAX_EXT
	SYNTAX: %ssuspend [reason]
	Disallows anyone from using the channel but preserves the channel
	data and settings untill the channel in unsuspended.
LANG_SUSPEND_NO_REASON
	A reason is mandatory to SUSPEND a channel on this network.
LANG_KB_KICK_DENIED
	Permission to kick %s is denied.
LANG_CLEAR_SYNTAX_EXT
	SYNTAX: %sclear [param]
	
	Following parameters are accepted:
	MODES   - Resets all modes on the channel (i.e. clears modes i,k,l,m,n,p,s,t).
	BANS    - Clears all bans on the channel.
	EXCEPTS - Clears all excepts on the channel.
	INVITES - Clears all invites on the channel.
	OPS     - Removes channel-operator status (mode +o) from all channel operators.
	HOPS    - Removes channel-halfoperator status (mode +h) from all channel halfoperators.
	VOICES  - Removes \"voice\" status (mode +v) from anyone with that mode set.
	USERS   - Removes (kicks) all users from the channel.
	
	Note: Not all parameters may be supported, depending on the IRCd.
LANG_ACCESS_LEVELS
	User access levels:
	
	By default, the following access levels are defined:
	*** Founder *** 
	Full access to administrative functions and opping upon entering the channel.
	Note that only one person may have founder status (it cannot be given using the
	ACCESS command {This is not true if cs_fop is loaded!}).
	***      10  *** 
	Access to AKICK command and automatic opping.
	***       5  *** 
	Automatic opping upon entering the channel.
	***       4  ***
	Automatic halfopping (+h) upon entering the channel. (If supported by IRCd.)
	***       3  ***
	Automatic voicing (+v) upon entering the channel.
	***       0  *** 
	No special privileges; Can be opped by other ops (unless secure-ops is set).
	***      <0  ***
	May not be opped.
	
	These levels may be changed, or new ones added, using the
	LEVELS command. For more information, cf. HELP LEVELS.
LANG_ADMINS
	SYNTAX: %sadmins
	This will return list of all available Services and Network Administrators..
LANG_SYNC_SYNTAX
	SYNTAX: %ssync
LANG_SYNC_SYNTAX_EXT
	SYNTAX: %ssync
	
	This command will give all users currently in the channel the level
	they are granted by the channels' access list. Users who have a level
	greater then the one they are supposed to have will be demoted.
	
	The use ofthis command is restricted to the Channel Founder.
LANG_SYNC_DONE
	Synchronized userlist with accesslist.
LANG_SHUN_SYNTAX
	SYNTAX: %sshun [add | del] [nick | user@host] [+expiry] [reason]
LANG_SHUN_SYNTAX_EXT
	SYNTAX: %sshun [add] [nick | user@host] [+expiry] [reason]
	        %sshun del [nick | user@host]
	
	This command adds or deletes a shun on the given nick or username@usermask
	If 'ADD' or 'DEL' is not specified, ADD is assumed.
	The time given is considered to be in days, unless another interval is specified.
	Possible alternatives: secondes (s), minutes, (m), hours, (h) and days (d).
	Note that these may not be combined! Only the first part will be read.
	If no reason is specified, a default reason will be used.
	If the reason starts with a '+', an expiry must be given!
	
	Note that SHUN DEL is only guaranteed to work for shuns added by the SHUN command
	and only if the user is still online. In any other case, use name@mask.
	When shunning the target 'add' or 'del', 'ADD' must be given.
LANG_TSHUN_SYNTAX
	SYNTAX: %stshun [add | del] [nick] [reason]
LANG_TSHUN_SYNTAX_EXT
	SYNTAX: %stshun [add] [nick] [reason]
	        %stshun del [nick]
	
	This command adds or deletes a temporary shun on the given user.
	If add or del is not specified, add is assumed.
	This command cannot be used on IRC operators.
	Note that this shun will be automatically removed when the user disconnects
	and that it will only affect one session.
LANG_SHUN_ADDED
	SHUN added for %s@%s.
LANG_SHUN_DELETION
	SHUN on %s@%s has been deleted, if it existed.
LANG_TSHUN_ADDED
	Temporary SHUN placed on user %s.
LANG_TSHUN_DELETION
	Removed temporary SHUN on user %s.
LANG_TSHUN_TARGET_NEXIST
	User %s does not currently exist.
LANG_TSHUN_TARGET_OPER
	Command cannot be used on IRC operators.
