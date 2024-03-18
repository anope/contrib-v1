##############################################################################
##### Archivo de idioma español.                                         #####
##############################################################################
# Nombre módulo        : bs_fantasy_ext
# Autor                : Viper <Viper@Anope.org>
# Fecha de Creación    : 14/09/2008
# Última Actualización : 04/09/2012
#
# Más información en http://modules.anope.org y http://forum.anope.org
#
##############################################################################
#
# Archivo de idioma Español. proporcionada por DoctorYerva <admin@cannabisdreams.es>.
#
# ! This file is currently outdated and missing some translations.
# ! I am looking for someone to update the translations and check them against en_us.
# ! Contact me @ viper@anope.org.
#
##############################################################################
#
# Este programa es software libre, puede redistribuirlo y / o modificar
# bajo los términos de la GNU General Public License publicada por
# la Free Software Foundation, bien de la versión 2 de la Licencia, o
# (según su elección) cualquier versión posterior.
#
##############################################################################
#
# Cuando la traducción de este fichero a otro idioma, tener en cuenta que la
# orden de los parámetros de sprintf () se fija en el código fuente, de modo que su
# mensajes necesidad de tener los mismos parámetros en el mismo orden que el
# Inglés mensajes hacer. Si no puede obtener una traducción naturales
# de un mensaje sin cambiar el orden de los parámetros, que me haga saber
# (viper@anope.org) mensaje que está causando un problema y voy a ver
# lo que puedo hacer.
#
#
# También en mensajes de ayuda, por favor, intente limitar las longitudes de línea de 90 caracteres
# de texto (no incluye la principal ficha). Esta longitud fue elegida porque
# no causa en virtud de la línea de recapitulación configuración predeterminada en la mayoría actual de IRC
# clientes. Recuerde que los caracteres de formato (de control y B, de control -_) no se
# incluido en ese 90-límite de caracteres (ya que no se muestran en el equipo del usuario
# pantalla). Asimismo, recuerda que el formato especificadores (%s, etc) se ampliará
# cuando aparece, así que recuerda tener en cuenta este hecho, se puede asumir
# que la duración de un pseudoclient nombre (por ejemplo, la sustitución %NS) será de ocho
# caracteres, de manera de reducir la longitud máxima de línea en un 6 por cada uno de %xS
# linea.
#
# A los traductores o los que sean suficientemente valientes como para editar este (y el riesgo de borrar):
# Los siguientes especificadores están disponibles:
#   - %R    Será traducido a "/" o "/msg", dependiendo de UseStrictPrivMsg.
#   - %F    Será traducido a la BSFantasyCharacter.
#   - %NS   Será traducido a la NickServ nick.
#   - %CS   Será traducido a la ChanServ nick.
#   - %OS   Será traducido a la OperServ nick.
#   - %BS   Será traducido a la BotServ nick.
#   - %MS   Será traducido a la MemoServ nick.
#   - %HS   Será traducido a la HostServ nick.
#
# Finally, remember to put a tab at the beginning of every line of text
# (including empty lines).  This has to be a tab, not spaces.
#
##############################################################################

LANG_NAME
	Spanish

#
#
# Generic Messages
#
#

#
# Error Messages
#

LANG_ONLY_FOUNDERS
	Acceso denegado: Sólo fundadores puede utilizar este comando.

LANG_EXCEPTS_UNSUPPORTED
	Excepto canal no se admiten en la red.
LANG_INVITEMODE_UNSUPPORTED
	Invitar a canal no se admiten en la red.
LANG_HOPS_UNSUPPORTED
	Halfops no se admiten en la red.

LANG_CMD_NOT_AV
	Este comando no está disponible actualmente. Por favor, póngase en contacto con el personal de la red en #Help para más información.

LANG_CMD_DISABLED
	Este comando ha sido desactivado. Por favor póngase en contacto con el Personal de la Red para más información.

LANG_UNKNOWN_CMD
	Comando desconocido: %s

LANG_INVALID_TIME
	%s no es válido timestamp.

LANG_REQ_NICK_OR_MASK
	Este comando requiere un alias o un usuario válido user@host mascara.


# Human readable expiration not covered by the core
EXPIRES_S
	expira en %d segundos
EXPIRES_1S
	expira en %d segundo


# Levels
LANG_LEVELS_ERR_NO_VAL
	Error: no hay un valor con el parámetro SET .
LANG_LEVELS_ERR_VAL
	Error: El valor no es numérico.

# Badwords
LANG_BADWORDS_ERR_STYLE
	Error: Error: Invalido estilo especificado. Por favor especifique START, SINGLE, ANY o END solo.

# Invite
LANG_INVITE_NO_USER
	No existe el usuario o no está en línea.
LANG_INVITE_IS_ON
	El usuario ya está en el canal!
LANG_INVITE_YOURSELF
	No se puede invitar a usted mismo!


#
# Operlist
#
LANG_GOLIST_OPER_ONLY
	Oper
LANG_GOLIST_OPER_AND_SO
	Oper + Servicios Oper
LANG_GOLIST_OPER_AND_SA
	Oper + Servicios Administrador
LANG_GOLIST_OPER_AND_SRA
	Oper + Servicios Root Administrador
LANG_GOLIST_NONE
	Opers no están disponibles actualmente
LANG_GOLIST_HEADER
	-----------------------------------------------------------------------
	Los siguientes son los operadores de IRC NetWork en línea:
	-----------------------------------------------------------------------
LANG_GOLIST_FOOTER
	-----------------------------------------------------------------------
	Total de Opers disponibles: %d 
	-----------------------------------------------------------------------
	 -> Siempre trate de contactar con un IRCop en el canal #Help antes de privado.
	 -> Trate de evitar hacer privado a un IRCop sin pedirle permiso primero.
	 -> Por favor, /whois a un IRCop antes de intentar comunicarse con ellos.
	 -> Leer (y respeto) de distancia de cualquier línea de la información.
	-----------------------------------------------------------------------

#
# Adminlist
#
LANG_ADLIST_SA
	Servicios Administrador
LANG_ADLIST_SRA
	Servicios Root Administrador
LANG_ADLIST_NONE
	Los administradores no están disponibles actualmente
LANG_ADLIST_HEADER
	-----------------------------------------------------------------------
	Los siguientes son los administradores en línea:
	-----------------------------------------------------------------------
LANG_ADLIST_FOOTER
	-----------------------------------------------------------------------
	Total Administradores disponibles: %d 
	-----------------------------------------------------------------------
	 -> Siempre trate de contactar a un Administrador en el canal #Help antes de privado.
	 -> Trate de evitar hacer privado a un administrador sin permiso primero.
	 -> Por favor, /whois a un Administrador antes de intentar comunicarse con ellos.
	 -> Leer (y respeto) de distancia de cualquier línea de la información.
	-----------------------------------------------------------------------

# TB
LANG_TBAN_RESPONSE
	Temporalmente baneado %s desde %s, %s.

#TSHUN
LANG_TSHUN_ADDED
	SHUN temporal en los usuarios %s.
LANG_TSHUN_DELETION
	SHUN eliminado temporal del usuario %s.
LANG_TSHUN_TARGET_NEXIST
	Usuario %s no existe actualmente.
LANG_TSHUN_TARGET_OPER
	Comando no se puede utilizar, solo los operadores en el IRC.

# SHUN
LANG_SHUN_ADDED
	SHUN añadido por %s@%s.
LANG_SHUN_DELETION
	SHUN en %s@%s fue bien eliminado y ya no existe.

# Suspend
LANG_SUSPEND_NO_REASON
	Una razón es obligatoria para suspender a un canal en la red.

# Sync
LANG_SYNC_DONE
	Sincronizado de usuarios con accesslist.

# Unban
LANG_UNBAN_NOTE
	Tratando de eliminar la prohibición %s.
	Sin embargo, esto no garantiza una completa limpieza unban porque
	con el fin de salvaguardar la privacidad de los usuarios se pongan en venta algunas prohibiciones
	los usuarios de propiedad intelectual o de acogida puede no haber sido eliminado.



#
#
# Help messages
#
#

LANG_HELP
	Sintaxis: %Fhelp
	Esta es la ayuda para el %BS fantasy comandos.
LANG_CMDLIST
	Sintaxis: %Fcmdlist
	Muestra los comandos disponibles fantasía


LANG_CMD_LIST_HEADER
	Comandos disponibles fantasía:
LANG_CMD_LIST_FOOTER
	Tenga en cuenta que no todas las posibles funciones de los servicios están cubiertos aquí ...
LANG_CMD_LIST_GENERAL
	General de comandos: %s
LANG_CMD_LIST_CHAN_AXX
	Canal de acceso a los comandos: %s
LANG_CMD_LIST_CHG_SET
	Canal configuracion: %s
LANG_CMD_LIST_ACTIONS
	Channel Actions: %s
LANG_CMD_LIST_IRCOP
	Comandos IRC Operador: %s
LANG_CMD_LIST_EXTMODS
	Other modules: %s

LANG_HELP_UNKNOWN_CMD
	No hay ayuda disponible para este comando.

LANG_HELP_CMDS_GEN
	General Comandos:
	   %Fhelp      -   Ayuda para %BS comandos de la fantasía
	   %Fcmdlist   -   Una lista de todos los comandos de fantasía
	   %Ftopic     -   Show the channel's topic
	   %Fircops    -   Muestra una lista de todos los IRCops Locales
	   %Fstaff     -   Muestra una lista de todos los IRCops Globales
	   %Fminfo     -   Muestra información acerca de este módulo
	   %Fsversion  -   Anope muestra información en general
	   %Finfo      -   Muestra información sobre el canal
	
	Nota: Estos comandos pueden no estar disponibles, los apagados o el acceso a los mismos puede ser restringido,
LANG_HELP_CMDS_ACC
	Comandos de Control de Acceso:
	   %Fsop       -   Proporciona acceso a la SOP (SuperOP) list
	   %Faop       -   Proporciona acceso a la AOP (AutoOP) list
	   %Fhop       -   Proporciona acceso a la HOP (HalfOP) list (No puede ser soportado en la red)
	   %Fvop       -   Proporciona acceso a la VOP (VOicePeople) list
	   %Faccess    -   Proporciona acceso a la access list (cuando la opcion XOP esta OFF)
	   %Flevels    -   Proporciona el acceso a la lista de control de los niveles de los derechos de cada nivel de acceso
	
	Nota: Estos comandos pueden no estar disponibles, los discapacitados o el acceso a los mismos puede ser limitada,
LANG_HELP_CMDS_SETTINGS
	Comandos de Ajuste de canal: 
	   %Fakick     -   Mantiene la akick list
	   %Fbadwords  -   Mantiene la badwords list
	   %Fset       -   Cambios en la configuración del canal. (cf. '%Fhelp set')
	   %Fbkick     -   Cambio la configuracion de %BS kick comandos.(cf. '%Fhelp bkick')
	
	Notea: Estos comandos pueden no estar disponibles, los discapacitados o el acceso a los mismos puede ser limitada,
LANG_HELP_CMDS_ACTIONS
	Comandos de canal de acción:
	   %Fclear     -   Borra la opción especificada en el canal.
	   %Ftopic     -   Establece el topic del canal
	   %Fctopic    -   Clear the channel's topic
	   %Ftappend   -   Añade el texto al topic
	   %Frappend   -   Replaces the text last appended to the topic
	   %Ftprepend  -   Prepends text to the topic
	   %Frprepend  -   Replaces the text last prepended to the topic
	   %Finvite    -   Invita a un usuario en el canal
	   %Fup        -   Ofrece al usuario los modos que tiene acceso a
	   %Fdown      -   Elimina todos los modos de los usuarios
	   %Fkick      -   Kick a user or all users matching a mask from the channel
	   %Fkb        -   Patea y Prohibe la entrada a un usuario en el canal
	   %Ftkb       -   Patea y expulsa temporalmente a un usuario en el canal
	
	Nota: Estos comandos pueden no estar disponibles, los discapacitados o el acceso a los mismos puede ser limitada,
LANG_HELP_CMDS_PUTMODES
	Modos de canal Comandos:
	   %Fban       -   Prohibiciones (a) usuario (s) / máscara de la canal sin patadas
	   %Ftban      -   Temporalmente la prohibición (a) usuario (s) / máscara de la canal sin patadas
	   %Funban     -   Removes the ban(s) on a user/mask.
	   %Fmute      -   Silencia / Prohibición el usuario (ircd Dependiente)
	   %Funmute    -   Unmutes / unbans el usuario (ircd Dependiente)
	   %Fop        -   Op usted o a otro usuario
	   %Fdeop      -   Deop usted o a otro usuario
	   %Fvoice     -   Voice usted o a otro usuario
	   %Fdevoice   -   Devoice usted o a otro usuario
	   %Fprotect   -   Protect usted o a otro usuario (No puede ser soportado en la red)
	   %Fdeprotect -   Deprotect usted o a otro usuario (No puede ser soportado en la red)
	   %Fowner     -   Dese a usted el modo owner (No puede ser soportado en la red)
	   %Fdeowner   -   Quitese a usted el modo owner (No puede ser soportado en la red)
	   %Fhalfop    -   Halfop usted o a otro usuario (No puede ser soportado en la red)
	   %Fdehalfop  -   Dehalfop usted o a otro usuario (No puede ser soportado en la red)
	
	Nota: Estos comandos pueden no estar disponibles, los discapacitados o el acceso a los mismos puede ser restringido,
LANG_HELP_CMDS_IRCOP
	Comandos IRC Operador:
	   %Fakill     -   Se fuerza la prohibición de una máscara, para toda la red.
	   %Fkill      -   'Kills' a el usuario
	   %Fmode      -   Cambio en los modos de canales
	   %Fignore    -   Hace ignore de los servicios a un usuario
	
	Nota: Estos comandos pueden no estar disponibles, los discapacitados o el acceso a los mismos puede ser restringido,

LANG_HELP_MORE_INFO
	Para obtener más información sobre un tipo %Fhelp <command>
LANG_HELP_LIST
	Para obtener una lista de comandos disponibles para lo que quieres hacer, escriba lo siguiente: 
	
	  %Fhelp general  - Muestra general de los comandos disponibles. No hay acceso necesario.
	  %Fhelp chaccess - Muestra los comandos para manipular los canales de acceso y niveles de lista (s).
	  %Fhelp config   - Configurar los canales y la configuración de listas.
	  %Fhelp act      - Comandos que harán que los servicios de realizar determinadas acciones.
	  %Fhelp ircop    - Comandos disponibles únicamente para los operadores de IRC.
	
	Para obtener una lista de todos los comandos existentes, escriba '%Fhelp all'

# Access levels
LANG_ACCESS_LEVELS
	Niveles de acceso de usuario:
	
	Por defecto, los siguientes niveles de acceso se definen:
	 *** Founder *** 
	 Pleno acceso a las funciones administrativas y opping al entrar en el canal.
	 Tenga en cuenta que sólo una persona puede tener fundador de estado (que no puede darse mediante el
	 ACCESS comando {Esto no es cierto si se ha cargado cs_fop!}).
	 ***      10  *** 
	 El acceso a AKICK mando y opping automáticamente.
	 ***       5  *** 
	 Opping automáticamente al entrar en el canal.
	 ***       4  ***
	 Halfopping automático (+h) al entrar en el canal. (Si por ircd.)
	 ***       3  ***
	 Expresando automática (+v) a la entrada del canal.
	 ***       0  *** 
	 No hay privilegios; opped puede ser por otras operaciones (a menos que secure-ops se establece).
	 ***      <0  ***
	 No puede ser opped.
	
	Estos niveles podrán ser modificados, o nuevos añadido, usando el
	LEVELS comandos. Para mas informacion, use HELP LEVELS.

# Admins
LANG_ADMINS_SYNTAX
	Sintaxis: %Fadmins
	Esto volverá a la lista de todos los servicios y administradores de la red ..

# Akick
LANG_AKICK_SYNTAX
	Sintaxis: %Fakick {add | stick | unstick | del | list | view | enforce | clear} [nick-or-usermask] [reason]
LANG_AKICK_SYNTAX_EXT
	Sintaxis: %Fakick add {mask | user} [reason]
	        %Fakick {stick | unstick | del} mask
	        %Fakick {list | view} [mask]
	        %Fakick {enforce | clear}

# Akill
LANG_AKILL_SYNTAX
	Sintaxis: %Fakill {add | del | list | view | clear} [+expiry] [mask | entry-list] [reason]
LANG_AKILL_SYNTAX_EXT
	Sintaxis: %Fakill add [+expiry] mask reason
	        %Fakill del {mask | entry-num | list}
	        %Fakill {list | view} [mask | list]
	        %Fakill clear
	Solo para operadores IRC. Este comando puede estar desactivado.

# Append
LANG_APPENDT_SYNTAX
	Sintaxis: {%Fappendtopic | %Ftappend} texto que se adjunta al topic
LANG_APPENDT_SYNTAX_EXT
	Sintaxis: {%Fappendtopic | %Ftappend} texto que se adjunta al topic
	Añade el texto indicado en el canal actual del tema.
	Este comando es muy útil en conjunción con SET TOPICLOCK

# Badwords
LANG_BADWORDS_SYNTAX
	Sintaxis: %Fbadwords {add | del | list | clear} [word | entry-list] [single | start | end | any]
LANG_BADWORDS_SYNTAX_EXT
	Sintaxis: %Fbadwords add word [single | start | end | any]
	        %Fbadwords del {word | entry-num | list}
	        %Fbadwords list [mask | list]
	        %Fbadwords clear

# Ban
LANG_BAN_SYNTAX
	Sintaxis: %Fban [nick | mask]
LANG_BAN_SYNTAX_EXT
	Sintaxis: %Fban [nick | mask]
	Prohíbe a un nick o usermask la entrada a el canal.

#
# BKick
#
LANG_BKICK_SYNTAX
	Sintaxis: %Fbkick option {on | off} [ttb [params]]
	Opciones permitidas: bolds, badwords, caps, colors, flood, repeat, reverses, underlines.
LANG_BKICK_SYNTAX_EXT
	Sintaxis: %Fbkick option {on | off} [ttb [params]]
	Available options:
	        bolds       -  Setea si el bot kickeara negrita.
	        badwords    -  Setea si el bot kickeara las palabras que esten agregadas al badworlist.
	        caps        -  Setea si el bot kickeara por MAYUSCULAS.
	        colors      -  Setea si el bot kickeara por colores.
	        flood       -  Setea si el bot kickeara por floods.
	        repeat      -  Setea si el bot kickeara por repeaticiones.
	        reverses    -  Setea si el bot kickeara por reverses.
	        underlines  -  Setea si el bot kickeara por underlines.
	
	TTB es el número de veces que un usuario puede ser expulsado antes de que se prohibiba su entrada permanentemente.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_BOLDS_SYNTAX
	Sintaxis: %Fbkick bolds {on | off} [ttb]
LANG_BKICK_BOLDS_SYNTAX_EXT
	Sintaxis: %Fbkick bolds {on | off} [ttb]
	Cuando está activada, esta opción le dice al bot que expulse a los usuarios que utilizan el bolds.
	servicios de enviar un aviso al canal cada vez que el OP o DEOP se utilizan los comandos.
	TTB es el número de veces que un usuario puede ser expulsado antes de que sea prohibido.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_BADWORDS_SYNTAX
	Sintaxis: %Fbkick badwords {on | off} [ttb]
LANG_BKICK_BADWORDS_SYNTAX_EXT
	Sintaxis: %Fbkick badwords {on | off} [ttb]
	Cuando está activada, esta opción le dice al robot para poner los usuarios que decir algunas palabras sobre el canal.
	Puede definir malas palabras para su canal mediante el comando %Fbadwords .
	TTB es el número de veces que un usuario puede ser expulsado antes de que sea prohibido.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_CAPS_SYNTAX
	Sintaxis: %Fbkick caps {on | off} [ttb [min [percent]]]
LANG_BKICK_CAPS_SYNTAX_EXT
	Sintaxis: %Fbkick caps {on | off} [ttb [min [percent]]]
	Cuando está activada, esta opción le dice al robot para poner los usuarios que están hablando en MAYUSCULAS.
	El robot se inicia sólo si hay al menos min MAYUSCULAS y constituyen por lo menos
	porcentaje% del total de línea de texto (si no está dado, que por defecto a 10 caracteres y 25%).
	TTB es el número de veces que un usuario puede ser expulsado antes de que sea prohibido.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_COLORS_SYNTAX
	Sintaxis: %Fbkick colors {on | off} [ttb]
LANG_BKICK_COLORS_SYNTAX_EXT
	Sintaxis: %Fbkick colors {on | off} [ttb]
	Cuando está activada, esta opción le dice al robot haga kick los usuarios que utilizan colors.
	TTB es el número de veces que un usuario puede ser expulsado antes de que sea prohibido.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_FLOOD_SYNTAX
	Sintaxis: %Fbkick flood {on | off} [ttb [ln [secs]]]
LANG_BKICK_FLOOD_SYNTAX_EXT
	Sintaxis: %Fbkick flood {on | off} [ttb [ln [secs]]]
	Cuando está activada, esta opción le dice al robot haga kick users who are flooding the channel
	using at least ln lines in secs seconds. 
	(if not given, it defaults to 6 lines in 10 seconds).
	TTB es el número de veces que un usuario puede ser expulsado antes de que sea prohibido.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_REPEAT_SYNTAX
	Sintaxis: %Fbkick repeat {on | off} [ttb [num]]
LANG_BKICK_REPEAT_SYNTAX_EXT
	Sintaxis: %Fbkick repeat {on | off} [ttb [num]]
	Cuando está activada, esta opción le dice al robot haga kick a los usuarios que repiten
	si num veces (num si no se da, por defecto sera 3)."
	TTB es el número de veces que un usuario puede ser expulsado antes de que sea prohibido.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_REVERSES_SYNTAX
	Sintaxis: %Fbkick reverses {on | off} [ttb]
LANG_BKICK_REVERSES_SYNTAX_EXT
	Sintaxis: %Fbkick reverses {on | off} [ttb]
	Cuando está activada, esta opción le dice al robot haga kick los usuarios que utilizan reverses.
	TTB es el número de veces que un usuario puede ser expulsado antes de que sea prohibido.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_UNDERLINES_SYNTAX
	Sintaxis: %Fbkick underlines {on | off} [ttb]
LANG_BKICK_UNDERLINES_SYNTAX_EXT
	Sintaxis: %Fbkick underlines {on | off} [ttb]
	Cuando está activada, esta opción le dice al robot haga kick los usuarios que utilizan underlines.
	TTB es el número de veces que un usuario puede ser expulsado antes de que sea prohibido.
	No le dé TTB para deshabilitar el sistema de prohibición, una vez activado.
LANG_BKICK_UNKNOWN_OPTION
	Desconocido BKICK opción: %s.


# Clear
LANG_CLEAR_SYNTAX
	Sintaxis: %Fclear {modes | bans | excepts | invites | ops | hops | voices | users}
LANG_CLEAR_SYNTAX_EXT
	Sintaxis: %Fclear [param]
	
	Se aceptan los siguientes parámetros:
	MODES   - Restablece todos los modos en el canal (i.e. clears modes i,k,l,m,n,p,s,t).
	BANS    - Borra todas las prohibiciones sobre el canal.
	EXCEPTS - Borra todos excepts en el canal.
	INVITES - Borra invita a todos en el canal.
	OPS     - Elimina-operador estado canal (modo +o) de todos los operadores de canal.
	HOPS    - Elimina el estado halfoperator canal (modo +h) de todos los canales halfoperators.
	VOICES  - Elimina \"voice\" estado (modo +v) de modo que cualquier persona con conjunto.
	USERS   - Elimina (kicks) a todos los usuarios del canal.
	
	Nota: No todos los parámetros pueden ser apoyadas, en función de la ircd.

# CTopic
LANG_CTOPIC_SYNTAX
	Syntax: %Fctopic
LANG_CTOPIC_SYNTAX_EXT
	Syntax: %Fctopic
	
	Clears the channels topic.
	This command is most useful in conjunction with SET TOPICLOCK

# Down
LANG_DOWN_SYNTAX
	Sintaxis: %Fdown [nick] [level]
LANG_DOWN_SYNTAX_EXT
	Sintaxis: %Fdown [nick] [level]
	
	Unsets todos los modos. Si el canal está moderado, que se restablecerá la voz,
	a menos que el otro usuario expedido por una de las fundadoras. También es posible
	para especificar un nivel de objetivo. El objetivo puede ser cualquiera de los siguientes:
	 - q - Fundador del canal (modos +qo) (Depende del IRCd)
	 - a - Operador Protegido / Administrador (modos +ao) (Depende del IRCd)
	 - o - Operador del canal (modo +o)
	 - h - Operador Medio del canala (mode +h) (Dpende del IRCd)
	 - v - \"Voice\" (modo +v)
	Si un objetivo es especificado, los usuarios actuales de privilegios será sustituido por el objetivo de una.
	
	Nota: Un nick sólo puede ser especificado por el fundador del canal o un SRA.

# Ignore
LANG_IGNORE_SYNTAX
	Sintaxis: %Fignore {add | del | list | clear} [time] [nick | mask]
LANG_IGNORE_SYNTAX_EXT
	Sintaxis: %Fignore {add | del} [expiry] [nick | mask]
	        %Fignore {list | clear}
	
	Permite a los Administradores de Servicios de Servicios de hacer caso omiso de un nick para un
	cierto tiempo o hasta el próximo reinicio. El tiempo predeterminado
	formato segundos. Combinaciones de unidades de tiempo no se permite.
	Servicios para hacer caso omiso de ignorar al usuario hasta el próximo reinicio de tipo 0 con el tiempo.

# Info
LANG_INFO_SYNTAX
	Sintaxis: %Finfo [all]
LANG_INFO_SYNTAX_EXT
	Sintaxis: %Finfo [all]
	Listas de información sobre el canal.
	El "all" opción se limita a los fundadores y administradores de servicios.

# Invite
LANG_INVITE_SYNTAX
	Sintaxis: %Finvite [nick]
LANG_INVITE_SYNTAX_EXT
	Sintaxis: %Finvite [nick]
	Invita a los 'nick' en el canal si el usuario está presente en la red

# KB
LANG_KICKBAN_SYNTAX
	Sintaxis: {%Fkickban | %Fkb} [nick | [nick!]ident@host] [reason]
LANG_KICKBAN_SYNTAX_EXT
	Sintaxis: {%Fkickban | %Fkb} [nick!]ident@host] [reason]
	
	Esta prohibición se iniciará y el usuario especificado o todos los usuarios se pongan en venta a la 
	máscara de la canal. Si se da una máscara y alguien que no debe ser expulsado 
	de acuerdo con los canales que coincide con la configuración, el comando se aborta antes de que
	nadie sea pateado o prohibido.

# KICK
LANG_KICK_SYNTAX
	Syntax: {%Fkick | %Fk} [nick | nick[!ident@host]] [reason]
LANG_KICK_SYNTAX_EXT
	Syntax: {%Fkick | %Fk} nick[!ident@host]] [reason]
	
	This will kick the specified user or all users matching the given
	mask from the channel.

# Kill
LANG_KILL_SYNTAX
	Sintaxis: %Fkill nick reason
LANG_KILL_SYNTAX_EXT
	Sintaxis: %Fkill nick reason
	
	Esta fuerza se desconecta el usuario de la red (mismo que /kill).
	Solo Operadores del IRC. Este comando puede ser deshabilitado

# Module Info
LANG_MINFO
	Sintaxis: {%Fminfo | %Fmodinfo}
	Muestra información sobre el módulo de suministro de la mayor parte de los comandos de fantasía.

# Mute / Unmute
LANG_MUTE_SYNTAX
	Sintaxis: %Fmute [nick | mask] (!IRCd Dependant!)
LANG_MUTE_SYNTAX_EXT
	Sintaxis: %Fmute [nick | mask]
	
	Prohibiciones o silencia dado el sobrenombre o usermask del canal. (Ircd Dependiente)

LANG_UNMUTE_SYNTAX
	Sintaxis: %Funmute [nick | mask] (!IRCd Dependant!)
LANG_UNMUTE_SYNTAX_EXT
	Sintaxis: %Funmute [nick | mask]
	
	Unbans o unmutes dado el sobrenombre o usermask del canal. (Ircd Dependiente)

# Mode
LANG_MODE_SYNTAX
	Sintaxis: %Fmode mode(s)
LANG_MODE_SYNTAX_EXT
	Sintaxis: %Fmode mode(s)
	
	De esta forma, establecerá las modalidades especificadas en el canal
	Solo Operadores del IRC. Este comando puede ser deshabilitado

# Prepend
LANG_PREPENDT_SYNTAX
	Syntax: {%Fprependtopic | %Ftprepend} text to be prepended to topic
LANG_PREPENDT_SYNTAX_EXT
	Syntax: {%Fprependtopic | %Ftprepend} text to be prepended to topic
	
	Prepends the given text to the channel's current topic.
	This command is most useful in conjunction with SET TOPICLOCK

# Rappend
LANG_RAPPEND_SYNTAX
	Syntax: {%Frappend | %Ftrapp} text to replace last append to topic
LANG_RAPPEND_SYNTAX_EXT
	Syntax: {%Frappend | %Ftrapp} text to replace last append to topic
	
	Replaces the last append to the topic with the given text.
	If no replacement text is given, the last append is removed.
	This command is most useful in conjunction with SET TOPICLOCK

# Rprepend
LANG_RPREPEND_SYNTAX
	Syntax: {%Frprepend | %Ftrprep} text to replace last prepend to topic
LANG_RPREPEND_SYNTAX_EXT
	Syntax: {%Frprepend | %Ftrprep} text to replace last prepend to topic
	
	Replaces the last prepend to the topic with the given text.
	If no replacement text is given, the last prepend is removed.
	This command is most useful in conjunction with SET TOPICLOCK

#
# SET
#
LANG_SET_SYNTAX
	Sintaxis: %Fset option parameters
	Las opciones disponibles: founder, successor, desc, email, url, entrymsg, bantype, mlock,
	        keeptopic, topiclock, opnotice, peace, private, restricted, secure, secureops,
	        securefounder, signkick, xop, noexpire (limited to SA)
	        dontkickops, dontkickvoices, greet, fantasy, symbiosis, nobot (limited to SA)
LANG_SET_SYNTAX_EXT
	Sintaxis: %Fset option parameters
	Las opciones disponibles:
	        founder       -  Ajuste el fundador de un canal
	        successor     -  Ajuste el sucesor de un canal
	        desc          -  Ajuste la descripción del canal 
	        url           -  Asociar una URL con el canal
	        email         -  Asociar una dirección de correo electrónico con el canal
	        entrymsg      -  Establezca un mensaje que se enviará a los usuarios cuando entran en el canal
	        bantype       -  Servicios Establece cómo hacer las prohibiciones sobre el canal
	        mlock         -  Modos de bloqueo de canales activado o desactivado
	        keeptopic     -  Conservar topic, cuando el canal no está en uso
	        topiclock     -  Topic sólo se pueden cambiar con el comando TOPIC
	        opnotice      -  Enviar un aviso cuando OP/DEOP comandos se utilizan
	        peace         -  Regular el uso de comandos críticos
	        private       -  Ocultar canal de comando LISTA
	        restricted    -  restringir el acceso al canal
	        secure        -  activar los servicios de seguridad
	        secureops     -  un control más estricto de la condición chanop
	        securefounder -  un control más estricto de canal fundador estado
	        signkick      -  Suscribirse patadas que se realizan con KICK comando
	        xop           -  Activa o desactiva el sistema de privilegios de usuario
	        noexpire      -  Establece si el canal dado expirará. (limitado a Administradores de Servicios)	
	
	        dontkickops   -  Proteger contra el canal ops patadas por parte de los servicios bot.
	        dontkickvoices - Proteger a las voces en contra de patadas por parte de los servicios bot.
	        greet         -  Habilitar mensajes de saludo al entrar a su canal.
	        fantasy       -  Habilitar comandos de fantasía en el canal. (leer ayudar primero !)
	        symbiosis     -  Deje que el robot para que actúe como un verdadero robot.
	        nobot         -  Evita que un bot de ser asignado a este canal. (limitado a Administradores de Servicios)

# Set subcommands
## Chanserv options
LANG_SET_FOUNDER_SYNTAX
	Sintaxis: %Fset founder nick
LANG_SET_FOUNDER_SYNTAX_EXT
	Sintaxis: %Fset founder nick
	Cambia el fundador de un canal. El nuevo nombre debe ser registrado en la Red.
LANG_SET_SUCCESSOR_SYNTAX
	Sintaxis: %Fset successor nick
LANG_SET_SUCCESSOR_SYNTAX_EXT
	Sintaxis: %Fset successor nick
	Cambia el sucesor de un canal. Si el apodo del fundador expira o se ha caído
	mientras que el canal está todavía registrado, el sucesor será el nuevo fundador del canal.
	El nuevo nombre debe ser registrado en la Red.
LANG_SET_DESC_SYNTAX
	Sintaxis: %Fset desc description
LANG_SET_DESC_SYNTAX_EXT
	Sintaxis: %Fset desc description
	Establece la descripción de la canal, que muestra con el comando LIST y INFO.
LANG_SET_URL_SYNTAX
	Sintaxis: %Fset url [url]
LANG_SET_URL_SYNTAX_EXT
	Sintaxis: %Fset url [url]
	Asociados a la URL con el canal.
	Esta URL se mostrará cada vez que alguien solicita información sobre el canal con el comando INFO.
	Si no se da el parámetro, se elimina cualquier URL actual para el canal.
LANG_SET_EMAIL_SYNTAX
	Sintaxis: %Fset email [email]
LANG_SET_EMAIL_SYNTAX_EXT
	Sintaxis: %Fset email [email]
	Asociados a la dirección de correo electrónico con el canal.
	Esta dirección se mostrará cada vez que alguien solicita información sobre el canal con el comando INFO.
	Si no se da el parámetro, se elimina cualquier actual Dirección de correo electrónico para el canal.
LANG_SET_ENTRYMSG_SYNTAX
	Sintaxis: %Fset entrymsg [entry-message],
LANG_SET_ENTRYMSG_SYNTAX_EXT
	Sintaxis: %Fset entrymsg [entry-message]
	Establece el mensaje que se enviará a través del /notice a userswhen aviso de que entren en el canal.
	Si no se da el parámetro, no causa ningún mensaje que se enviará al entrar en el canal.
LANG_SET_BANTYPE_SYNTAX
	Sintaxis: %Fset bantype {0 | 1 | 2 | 3}
LANG_SET_BANTYPE_SYNTAX_EXT
	Sintaxis: %Fset bantype {0 | 1 | 2 | 3}
	Establece la prohibición de tipo que será utilizada por los servicios que necesitan cada vez de prohibir a alguien de tu canal.
	
	bantype es un número entre 0 y 3 que significa:
	0: ban in the form *!user@host
	1: ban in the form *!*user@host
	2: ban in the form *!*@host
	3: ban in the form *!*user@*.domain
LANG_SET_MLOCK_SYNTAX
	Sintaxis: %Fset mlock [modes]
LANG_SET_MLOCK_SYNTAX_EXT
	Sintaxis: %Fset mlock [modes]
	Establece el modo de bloqueo de parámetro para el canal.
	Esto le permite definir ciertos modos de canal para estar siempre puestos, o fuera de la libertad de estar encendido o apagado.,
LANG_SET_KEEPTOPIC_SYNTAX
	Sintaxis: %Fset keeptopic {on | off}
LANG_SET_KEEPTOPIC_SYNTAX_EXT
	Sintaxis: %Fset keeptopic {on | off}
	Activa o desactiva la opción de retención de tema para un canal. Al tema de retención se fija,
	el tema del canal será recordado incluso después de que el último usuario deja el canal,
	and will be restored the next time the channel is created.
LANG_SET_TOPICLOCK_SYNTAX
	Sintaxis: %Fset topiclock {on | off}
LANG_SET_TOPICLOCK_SYNTAX_EXT
	Sintaxis: %Fset topiclock {on | off}
	Activa o desactiva la opción de bloqueo tema para un canal. Al tema de bloqueo se fija,
	Los servicios de no permitir que el canal topic que se ha cambiado excepto a través del comando TOPIC.
LANG_SET_OPNOTICE_SYNTAX
	Sintaxis: %Fset opnotice {on | off}
LANG_SET_OPNOTICE_SYNTAX_EXT
	Sintaxis: %Fset opnotice {on | off}
	Activa o desactiva la opción Aviso opción para un canal. Cuando op-aviso está definido,
	servicios de enviar un aviso al canal cada vez que el OP o DEOP se utilizan los comandos.
LANG_SET_PEACE_SYNTAX
	Sintaxis: %Fset peace {on | off}
LANG_SET_PEACE_SYNTAX_EXT
	Sintaxis: %Fset peace {on | off}
	Activa o desactiva la opción de la paz de un canal. Cuando la paz se establece,
	un usuario no será capaz de patear, prohibir o eliminar una condición de la canal de un usuario que ha
	un nivel superior o igual a través de sus servicios.
LANG_SET_PRIVATE_SYNTAX
	Sintaxis: %Fset private {on | off}
LANG_SET_PRIVATE_SYNTAX_EXT
	Sintaxis: %Fset private {on | off}
	Activa o desactiva la opción privado para un canal.
	Cuando se fija privada, un %R%CS LIST no incluirá el canal en ninguna lista.
LANG_SET_RESTRICTED_SYNTAX
	Sintaxis: %Fset restricted {on | off}
LANG_SET_RESTRICTED_SYNTAX_EXT
	Sintaxis: %Fset restricted {on | off}
	Activa o desactiva la opción de acceso restringido para un canal.
	Activa o desactiva la opción de acceso restringido para un canal
	privilegios (los usuarios con niveles de acceso negativo y, si es seguro ops conjunto, los usuarios no figuran en la lista de acceso)
	en lugar de ser expulsado y prohibido de la canal.
LANG_SET_SECURE_SYNTAX
	Sintaxis: %Fset secure {on | off}
LANG_SET_SECURE_SYNTAX_EXT
	Sintaxis: %Fset secure {on | off}
	Activa o desactiva %CS's características de seguridad de un canal. SECURE Cuando se establece, sólo los usuarios que han
	registrado sus Nicks con los servicios y IDENTIFY con su contraseña tendrá acceso
	al canal como el control de la lista de acceso.
LANG_SET_SECUREOPS_SYNTAX
	Sintaxis: %Fset secureops {on | off}
LANG_SET_SECUREOPS_SYNTAX_EXT
	Sintaxis: %Fset secureops {on | off}
	Activa o desactiva la opción de garantizar operaciones de un canal.
	Ops seguro cuando está activado, los usuarios que utilizan no en el de usuarios no se permitirá chanop estado.
LANG_SET_SECUREFOUNDER_SYNTAX
	Sintaxis: %Fset securefounder {on | off}
LANG_SET_SECUREFOUNDER_SYNTAX_EXT
	Sintaxis: %Fset securefounder {on | off}
	Activa o desactiva la opción segura fundador de un canal. Cuando seguro es fundador conjunto,
	el verdadero fundador sólo será capaz de abandonar el canal, cambiar su contraseña, su fundador y su
	sucesor, y no aquellos que son IDENTIFY con servicios.
LANG_SET_SIGNKICK_SYNTAX
	Sintaxis: %Fset signkick {on | level | off}
LANG_SET_SIGNKICK_SYNTAX_EXT
	Sintaxis: %Fset signkick {on | level | off}
	Activa o desactiva firmado patadas a un canal. Cuando se establece SIGNKICK, patadas expedido con
	los servicios de KICK comando tienen el nick que utiliza el comando en su razón.
	Si utiliza LEVEL, los que tienen un nivel que es superior o igual a la SIGNKICK levelon la
	canal no han firmado sus patadas. Ver '%Fhelp levels' para mas informacion.
LANG_SET_XOP_SYNTAX
	Sintaxis: %Fset xop {on | off}
LANG_SET_XOP_SYNTAX_EXT
	Sintaxis: %Fset xop {on | off}
	Activa o desactiva el sistema de listas XOP un canal. Cuando XOP está definido, usted tiene que utilizar el AOP/SOP/VOP
	comandos con el fin de dar privilegios de canal a los usuarios, cosa que usted tiene que usar el comando ACCESS.,
LANG_SET_NOEXPIRE_SYNTAX
	Sintaxis: %Fset noexpire {on | off}
LANG_SET_NOEXPIRE_SYNTAX_EXT
	Sintaxis: %Fset noexpire {on | off}
	Establece si el canal dado expirará. La fijación de este a ON impide que el canal de terminaría.
	
	Este comando se limita a los Administradores de Servicios.

## Botserv options
LANG_SET_DONTKICKOPS_SYNTAX
	Sintaxis: %Fset dontkickops {on | off}
LANG_SET_DONTKICKOPS_SYNTAX_EXT
	Sintaxis: %Fset dontkickops {on | off}
	Activa o desactiva la protección de OP en el canal. Cuando está activado,
	ops no será pateado por el robot de servicio, aun cuando no coinciden con la NOKICK level.
LANG_SET_DONTKICKVOICES_SYNTAX
	Sintaxis: %Fset dontkickvoices {on | off}
LANG_SET_DONTKICKVOICES_SYNTAX_EXT
	Sintaxis: %Fset dontkickvoices {on | off}
	Activa o desactiva la protección de VOZ sobre el canal. Cuando está activado,
	voces no será pateado por el robot de servicio, aun cuando no coinciden con la NOKICK level.
LANG_SET_GREET_SYNTAX
	Sintaxis: %Fset greet {on | off}
LANG_SET_GREET_SYNTAX_EXT
	Sintaxis: %Fset greet {on | off}
	Activa o desactiva el modo de saludar a un canal. Cuando está activado, el bot mostrará saludar
	mensajes de los usuarios que une el canal, siempre que tengan suficiente acceso al canal.
LANG_SET_FANTASY_SYNTAX
	Sintaxis: %Fset fantasy {on | off}
LANG_SET_FANTASY_SYNTAX_EXT
	Sintaxis: %Fset fantasy {on | off}
	Activa o desactiva el modo de la fantasía en un canal. Cuando está activada, los usuarios podrán utilizar
	comandos %Fop, %Fcmdlist y %Fhelp. Tenga en cuenta que los usuarios que deseen utilizar los comandos MUST tener fantasilist
	nivel suficiente para el nivel de FANTASIA y otro dependiendo del comando si es necesario

	IMPORTANTE: Al desactivar esta deshabilitará todos los comandos de fantasía en este canal a fin de volver a permitir
	Habrá que hacerse a través de %BS.
LANG_SET_SYMBIOSIS_SYNTAX
	Sintaxis: %Fset symbiosis {on | off}
LANG_SET_SYMBIOSIS_SYNTAX_EXT
	Sintaxis: %Fset symbiosis {on | off}
	Activa o desactiva el modo de simbiosis en un canal. Cuando está activado, el robot hará todo lo posible
	normalmente realizada por %CS en los canales, tales como los MODES, KICKS, e incluso GREET de mensajes.
	Nota: Cuando está desactivada, la mayoría de los comandos de fantasía seguirá siendo manejado por el robot.
LANG_SET_NOBOT_SYNTAX
	Sintaxis: %Fset nobot {on | off}
LANG_SET_NOBOT_SYNTAX_EXT
	Sintaxis: %Fset nobot {on | off}
	Esta opción hace que un canal se unassignable. Si un robot ya se ha asignado al canal,
	es sin asignar automáticamente cuando se active la opción.

LANG_SET_UNKNOWN_OPTION
	Desconocido SET opción: %s.

# Shun
LANG_SHUN_SYNTAX
	Sintaxis: %Fshun [add | del] [+expiry] {nick | user@host} [reason]
LANG_SHUN_SYNTAX_EXT
	Sintaxis: %Fshun [add] [+expiry] {nick | user@host} [reason]
	        %Fshun del {nick | user@host}
	
	Este comando añade o elimina un rehuir dado en el nick o username@usermask
	Si 'ADD' o 'DEL' no se especifica, se supone Añadir.
	El tiempo dado que se considera en días, a menos que se especifique otra intervalo.
	Posibles alternativas: segundos (s), minutos (m), horas (h) y días (d).
	Tenga en cuenta que estos no pueden ser combinados! Sólo la primera parte se lee.
	Si no se especifica la razón, una razón por defecto se utilizará.
	Si la máscara se inicia con una '+', porque el tiempo de vencimiento se debe dar!
	
	Tenga en cuenta que SHUN DEL sólo se garantiza que funcione para rehuye añadido por el comando SHUN
	y sólo si el usuario se encuentra conectado. En cualquier otro caso, el uso name@mask.
	Al rechazar el objetivo 'add' o 'del', 'ADD' se debe dar.

# Staff
LANG_STAFF
	Sintaxis: {%Fstaff | %Fircops}
	Esto mostrara la lista de IRCops Globales disponibles.

# Status mode commands (!op/!deop/...)
LANG_STATUS_MODE_SYNTAX
	Syntax: %F%s [[nick | mask] [[nick | mask] [...]]]
LANG_STATUS_MODE_SYNTAX_EXT
	Syntax: %F%s [[nick | mask] [[nick | mask] [...]]]
	
	Changes status modes on the channel for the given user(s) or all users matching given mask(s).
	This command accepts a space separated list of nicks, masks or a combination thereof.
	Depending on channel settings, this command will not take modes from users with a
	higher access level than the issuer.

# Suspend
LANG_SUSPEND_SYNTAX
	Sintaxis: %Fsuspend [reason]
LANG_SUSPEND_SYNTAX_EXT
	Sintaxis: %Fsuspend [reason]
	
	Impide el uso de cualquier persona de la canal, pero conserva el canal
	datos y la configuración hasta el canal en unsuspended.

# Sversion
LANG_SVERSION
	Sintaxis: %Fsversion
	Muestra información sobre el Centro de Servicios Anope versión y construir.

# Sync
LANG_SYNC_SYNTAX
	Sintaxis: %Fsync
LANG_SYNC_SYNTAX_EXT
	Sintaxis: %Fsync
	
	Este comando le dará todos los usuarios actualmente en el nivel del canal
	es concedida por los canales lista de acceso. Los usuarios que tengan un nivel
	mayor que la que se supone que se han degradado.
	
	El uso de este comando se limita a el Fundador del canal.

# TB
LANG_TBAN_SYNTAX
	Sintaxis: {%Ftb | %Ftban} [nick | [nick!]ident@host] [time]
LANG_TBAN_SYNTAX_EXT
	Sintaxis: {%Ftb | %Ftban} [nick | [nick!]ident@host] [time]
	
	Esta prohibición se da el nick o hostmask del canal con el objetivo específico
	cantidad de tiempo. Si el tiempo no se da, será por defecto a 1 hora.
	El tiempo que debe darse en el formato <digit><(s)/m/h/d>.
	Ejemplo: %Ftb Viper 10m que tendra prohibición durante 10 minutos.
	
	Nota: si los servicios se reinicia expira antes de la prohibición, la prohibición
	no se eliminará automáticamente

# TKB
LANG_TKBAN_SYNTAX
	Sintaxis: %Ftkb [nick | [nick!]ident@host] [time] [reason]
LANG_TKBAN_SYNTAX_EXT
	Sintaxis: %Ftkb or %Ftkban [nick | [nick!]ident@host] [time] [reason]
	
	Este se iniciará el período de prohibición temporal y el usuario especificado o todos los usuarios se pongan en venta
	habida cuenta de la máscara de la canal de la cantidad especificada de tiempo. 
	Si se da una máscara y alguien que no debería ser prohibido de acuerdo con la
	canales de la configuración de los partidos, el comando se aborta antes de que alguien es pateado.
	También ser conscientes de que este comando no puede trabajar con direcciones IP por motivos de privacidad."
	Si el tiempo no se da o no es válida, que por defecto a 1 hora.
	El tiempo que debe darse en el formato <digit><(s)/m/h/d>.
	Ejemplo: %Ftkb Viper 10m Bye que tendra kickban durante 10 minutos.
	
	Nota: si los servicios se reinicia expira antes de la prohibición, la prohibición
	no se eliminará automáticamente.

# Topic
LANG_TOPIC_SYNTAX
	Sintaxis: %Ftopic [topic]
	Sintaxis: %Fctopic
LANG_TOPIC_SYNTAX_EXT
	Sintaxis: %Ftopic [topic]
	Sintaxis: %Fctopic
	
	Establece el topic de canal. Si no hay ningún topic que se trata, el topic actual se muestra en el canal.
	El comando se CTOPIC para borrar el topic.
	Este comando es muy útil en conjunción con SET TOPICLOCK

# TSHUN
LANG_TSHUN_SYNTAX
	Sintaxis: %Ftshun [add | del] nick [reason]
LANG_TSHUN_SYNTAX_EXT
	Sintaxis: %Ftshun [add] [nick] [reason]
	        %Ftshun del [nick]
	
	Este comando añade o elimina un temporal en el rehuir usuario.
	Si añade o del no se especifica, se supone añadir.
	Este comando no puede ser usado por Operadores del IRC.
	Tenga en cuenta que este rehuir se eliminará automáticamente cuando el usuario se desconecta
	y que sólo afectará a un período de sesiones.

# Unban
LANG_UNBAN_SYNTAX
	Syntax: %Funban [nick | mask]
LANG_UNBAN_SYNTAX_EXT
	Syntax: %Funban [nick | mask]
	
	Removes bans on the given mask from the channel or removes all bans matching given user.
	
	Note: checking against a users IP address is done for opers or when unbanning yourself.

# UP
LANG_UPDOWN_SYNTAX
	Sintaxis: {%Fup | %Fdown} [nick]
LANG_UP_SYNTAX
	Sintaxis: %Fup [nick]
LANG_UP_SYNTAX_EXT
	Sintaxis: %Fup [nick]
	
	Ofrece al usuario los modos que tiene derecho a. Esto unset modos, si tiene alguna
	el usuario no tiene acceso a el canal.
	
	Nota: Un nick sólo puede ser especificado por el fundador de la canal o un Administrador de Servicios.


#
# XOP & ACCESS, LEVELS
#
LANG_SOP_SYNTAX
	Sintaxis: %Fsop {add | del | list | clear} [nick | entry-list]
LANG_SOP_SYNTAX_EXT
	Sintaxis: %Fsop add nick
	        %Fsop del {nick | entry-num | list}
	        %Fsop list [mask | list]
	        %Fsop clear
	
	Mantiene la SOP (SuperOP) lista para el canal.
	La lista SOP proporciona a los usuarios todos los derechos otorgados por la lista AOP, y agrega
	los necesarios para utilizar el AutoKick y el BadWords lists,
	para enviar y leer memos de canal, y así sucesivamente.
	
	Este comando puede haber sido inhabilitada para su canal, y
	en ese caso deberá utilizar la lista de acceso. Ver HELP ACCESS
	para obtener información sobre la lista de acceso, y HELP SET XOP para saber como
	para alternar entre la lista de acceso y sistemas de listas XOP.

LANG_AOP_SYNTAX
	Sintaxis: %Faop {add | del | list | clear} [nick | entry-list]
LANG_AOP_SYNTAX_EXT
	Sintaxis: %Faop add nick
	        %Faop del {nick | entry-num | list}
	        %Faop list [mask | list]
	        %Faop clear
	
	Mantiene la AOP (AutoOP) lista para el canal.
	La lista AOP ofrece a los usuarios el derecho a ser auto-opped en su canal,
	a eliminar la prohibición o invitar a sí mismos si fuera necesario, para que sus
	mensaje de saludo que se mostrara al entrar, y así sucesivamente.
	
	Este comando puede haber sido inhabilitada para su canal, y
	en ese caso deberá utilizar la lista de acceso. Ver HELP ACCESS
	para obtener información sobre la lista de acceso, y HELP SET XOP para saber como
	para alternar entre la lista de acceso y sistemas de listas XOP.

LANG_HOP_SYNTAX
	Sintaxis: %Fhop {add | del | list | clear} [nick | entry-list]
LANG_HOP_SYNTAX_EXT
	Sintaxis: %Fhop add nick
	        %Fhop del {nick | entry-num | list}
	        %Fhop list [mask | list]
	        %Fhop clear
	
	Mantiene la HOP (HalfOP) lista para el canal. La HOP
	lista proporciona a los usuarios el derecho a ser auto-halfopped en su canal
	
	Este comando puede haber sido inhabilitada para su canal, y
	en ese caso deberá utilizar la lista de acceso. Ver HELP ACCESS
	para obtener información sobre la lista de acceso, y HELP SET XOP para saber como
	para alternar entre la lista de acceso y sistemas de listas XOP.

LANG_VOP_SYNTAX
	Sintaxis: %Fvop {add | del | list | clear} [nick | entry-list]
LANG_VOP_SYNTAX_EXT
	Sintaxis: %Fvop add nick
	        %Fvop del {nick | entry-num | list}
	        %Fvop list [mask | list]
	        %Fvop clear
	
	Mantiene la VOP (VOicePeople) list for a channel.
	The VOP list allows users to be auto-voiced and to voice
	themselves if they aren't.
	
	This command may have been disabled for your channel, and
	en ese caso deberá utilizar la lista de acceso. Ver HELP ACCESS
	para obtener información sobre la lista de acceso, y HELP SET XOP para saber como
	para alternar entre la lista de acceso y sistemas de listas XOP.

LANG_ACCESS_SYNTAX
	Sintaxis: %Faccess {add | del | list | clear} [nick [level] | mask | list]
LANG_ACCESS_SYNTAX_EXT
	Sintaxis: %Faccess add nick level
	        %Faccess del {nick | entry-num | list}
	        %Faccess list [mask | list]
	        %Faccess clear
	
	Mantiene la lista de acceso para un canal. El acceso
	lista especifica que los usuarios se les permite chanop o
	el acceso a los comandos en el canal.
	Diferentes niveles de usuario para permitir el acceso a los distintos subgrupos de
	privilegios; cf. %R%CS HELP ACCESS LEVELS para obtener información más específica.
	Cualquier nick no aparece en la lista de acceso de usuario tiene un nivel de 0.

LANG_LEVELS_SYNTAX
	Sintaxis: %Flevels {set |dis[able] | list | reset} [item [level]]
LANG_LEVELS_SYNTAX_EXT
	Sintaxis: %Flevels set type level
	        %Flevels {dis | disable} type
	        %Flevels list
	        %Flevels reset
	
	El LEVELS permite un control preciso sobre el significado de
	los niveles de acceso numérico utilizado para el canal. Con este comando,
	puede definir el nivel de acceso requerido para la mayoría de las funciones administrativas.
	(SET FOUNDER y SET PASSWORD, así como este comando, 
	siempre se limita a la canal fundador.) 
	
	LEVELS SET permite que el nivel de acceso para una función o grupo de funciones que cambiar.
	LEVELS DISABLE (o DIS para abreviar) se desactiva automáticamente
	característica o impida el acceso a una función por cualquiera que no sea el canal fundador.
	LEVELS LIST muestra los niveles actuales para cada función o grupo de funciones.
	LEVELS RESET restablece los niveles a los niveles predeterminados de un recién creado canal.
	(Vease %R%CS HELP ACCESS LEVELS para más información.)
	
	Para obtener una lista de las características y funciones cuyos niveles pueden ser
	conjunto, véase HELP LEVELS DESC.

