#include "language.h"

const boolean coloredText_iSupport[16]  = {1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0};
const cText_t coloredText[16][6]        = {
    /* english */
    {{"[\037New\037]", "[\037Updated\037]", "[\037Activated\037]", "[\037Rejected\037]", "Host Request for %s activated by %s (%s)", "Host Request for %s rejected by %s (%s)"},
    {"\00312[\037New\037]\003", "\00311[\037Updated\037]\003", "\00309[\037Activated\037]\003", "\00304[\037Rejected\037]\003", "\00309Host Request for %s activated by %s (%s)\003", "\00304Host Request for %s rejected by %s (%s)\003"}},
    /* japanese (jis encoding) */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* japanese (euc encoding) */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* japanese (sjis encoding)  */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* spanish */
    {{"[\037Nuevo\037]", "[\037Actualizado\037]", "[\037Activado\037]", "[\037Rechazado\037]", "Petici�n de Host para %s activado por %s (%s)", "Petici�n de Host de %s rechazado por %s (%s)"},
    {"\00312[\037Nuevo\037]\003", "\00311[\037Actualizado\037]\003", "\00309[\037Activado\037]\003", "\00304[\037Rechazado\037]\003", "\00309Petici�n de Host para %s activado por %s (%s)\003", "\00304Petici�n de Host para %s rechazado por %s (%s)\003"}},
    /* portuguese */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* french */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* turkish */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* italian */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* german */
    {{"[\037Neu\037]", "[\037Ver�ndert\037]", "[\037Aktiviert\037]", "[\037Abgelehnt\037]", "vHost f�r %s wurde gesetzt von %s (%s)", "vHost f�r %s wurde abgelehnt von %s (%s)"},
    {"\00312[\037Neu\037]\003", "\00311[\037Ver�ndert\037]\003", "\00309[\037Aktiviert\037]\003", "\00304[\037Abgelehnt\037]\003", "\00309vHost f�r %s wurde gesetzt von %s (%s)\003", "\00304vHost f�r %s wurde abgelehnt von %s (%s)\003"}},
    /* catalan */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* greek */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* dutch */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* russian */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* hungarian */
    {{"[\037�j\037]", "[\037Friss�tett\037]", "[\037Aktiv�lt\037]", "[\037Elutas�tott\037]", "A host k�relem a(z) %s nick-hez aktiv�l�sra ker�lt %s �ltal (%s)", "A host k�relem a(z) %s nick-hez elutas�t�sra ker�lt %s �ltal (%s)"},
    {"\00312[\037�j\037]\003", "\00311[\037Friss�tett\037]\003", "\00309[\037Aktiv�lt\037]\003", "\00304[\037Elutas�tott\037]\003", "\00309A host k�relem a(z) %s nick-hez elutas�t�sra ker�lt %s (%s)\003 �ltal", "\00304A host k�relem a(z) %s nick-hez elutas�t�sra ker�lt %s �ltal (%s)\003"}},
    /* polish */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
};

int my__def_language(void)
{
    if (coloredText_iSupport[NSDefLanguage])
        return NSDefLanguage;
    else
        return LANG_EN_US;
}

void my__add_languages(void)
{
/* English translation */
char *langtable_en_us[] = {
    /* LNG_REQUEST_SYNTAX */
    "Syntax: \002REQUEST \037vHost\037\002",
    /* LNG_REQUESTED */
    "Your vHost has been requested. Please wait for an operator to activate it.",
    /* LNG_REQUEST_WAIT */
    "Please wait %d seconds before requesting a new vHost.",
    /* LNG_REQUEST_MEMO */
    "[auto memo] vHost \002%s\002 has been requested.",
    /* LNG_ACTIVATE_SYNTAX */
    "Syntax: \002{ACTIVATE|A} \037nick\037 [\037reason\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Note: Wildcard nick matching is enabled. Supported wildcard characters are '?' and '*'.\n"
#else
    "  Note: Wildcard nick matching is disabled.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Note: Regex nick matching is enabled. Prefix match string with '!'.",
#else
    "  Note: Regex nick matching is disabled.",
#endif
    /* LNG_ACTALL_SYNTAX */
    "Syntax: \002ACTALL [\037reason\037]\002",
    /* LNG_ACTIVATED */
    "vHost for '%s' has been activated.",
    /* LNG_ACTIVATE_MEMO */
    "[auto memo] Your requested vHost has been approved and set.",
    /* LNG_ACTIVATE_MEMO_REASON */
    "[auto memo] Your requested vHost has been approved and set. Reason: %s",
    /* LNG_REJECT_SYNTAX */
    "Syntax: \002{REJECT|R} \037nick\037 [\037reason\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Note: Wildcard nick matching is enabled. Supported wildcard characters are '?' and '*'.\n"
#else
    "  Note: Wildcard nick matching is disabled.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Note: Regex nick matching is enabled. Prefix match string with '!'.",
#else
    "  Note: Regex nick matching is disabled.",
#endif
    /* LNG_REJALL_SYNTAX */
    "Syntax: \002REJALL [\037reason\037]\002",
    /* LNG_REJECTED */
    "vHost for '%s' has been rejected.",
    /* LNG_REJECT_MEMO */
    "[auto memo] Your requested vHost has been rejected.",
    /* LNG_REJECT_MEMO_REASON */
    "[auto memo] Your requested vHost has been rejected. Reason: %s",
    /* LNG_NO_REQUEST */
    "No request for nick '%s' found.",
    /* LNG_HELP */
    "    REQUEST     Request a vHost for your nick",
    /* LNG_HELP_SETTER */
    "    ACTIVATE    Approve the requested vHost of a user\n"
    "                Shortcut \002A\002 is available\n"
    "    ACTALL      Approve all requested vHosts\n"
    "    REJECT      Reject the requested vHost of a user\n"
    "                Shortcut \002R\002 is available\n"
    "    REJALL      Reject all requested vHosts\n"
    "    WAITING     Convenience command for LIST +req\n"
    "                Shortcut \002W\002 is available\n"
    "    CONFIG      Access various settings",
    /* LNG_HELP_REQUEST */
    "Request the given vHost to be activated for your nick by the\n"
    "network administrators. Please be patient while your request\n"
    "is being considered.",
    /* LNG_HELP_ACTIVATE */
    "Activate the requested vHost for the given nick.",
    /* LNG_HELP_ACTALL */
    "Activate all requested vHosts.",
    /* LNG_HELP_ACTIVATE_MEMO */
    "A memo informing the user will also be sent.",
    /* LNG_HELP_REJECT */
    "Reject the requested vHost for the given nick.",
    /* LNG_HELP_REJECT_MEMO */
    "A memo informing the user will also be sent.",
    /* LNG_HELP_REJALL */
    "Reject all requested vHosts.",
    /* LNG_HELP_REJALL_MEMO */
    "A memo informing the users will also be sent.",
    /* LNG_WAITING_SYNTAX */
    "Syntax: \002{WAITING|W} [{\037nick\037|\037num\037}]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Note: Wildcard nick matching is enabled. Supported wildcard characters are '?' and '*'.\n"
#else
    "  Note: Wildcard nick matching is disabled.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Note: Regex nick matching is enabled. Prefix match string with '!'.",
#else
    "  Note: Regex nick matching is disabled.",
#endif
    /* LNG_HELP_WAITING */
    "This command is provided for convenience. It is essentially\n"
    "the same as performing a  LIST +req  .\n"
    " \n"
    "Output can be limited either by specifying a nick, in\n"
    "which case only the request belonging to that nick will\n"
    "be displayed, or by specifying a number, in which case\n"
    "the number of entries displayed will not exceed this number.\n"
    "Numbers 0 and smaller will display the total number of open\n"
    "vHost requests.",
    /* LNG_REQUEST_FORBIDDEN */
    "The vHost you requested is forbidden.",
    /* LNG_REQUEST_FORBIDDEN_REASON */
    "The vHost you requested is forbidden. Reason: %s",
    /* LNG_CONFIG_SYNTAX */
    "Syntax: \002CONFIG \037option\037 [\037parameters\037]\002",
    /* LNG_HELP_CONFIG */
    "Allows access to various settings. \037option\037 can be one of:\n"
    " \n"
    "    EXCEPTION   Modify list of exceptions\n"
    "    REGEX       Modify list of regexes\n"
    "    SAVE_DB     Manually dump request and regex databases to files\n"
    "    LIST        List config values\n"
    "    SET         Adjust runtime variables\n"
    " \n"
    "Details for: \002CONFIG EXCEPTION\002\n"
    "Syntax: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#num\037|\037nick\037}|LIST}\002\n"
    " \n"
    "Allows modifying the exception list.\n"
    " \n"
    "    LIST        Display exception list, includes exception number and set-by nick for use in \002DEL\002\n"
    "    ADD         Add an exception. All group members will be exempt aswell.\n"
    "    DEL         Delete an exception by reference, see \002LIST\002\n"
    " \n"
    "Details for: \002CONFIG REGEX\002\n"
    "Syntax: \002CONFIG REGEX {ADD \037reason\037 \037regex\037|DEL {\037#num\037|\037nick\037}|LIST}\002\n"
    " \n"
    "Allows modifying the regex list.\n"
    " \n"
    "    LIST        Display regex list, includes regex number and creator nick for use in \002DEL\002\n"
    "    ADD         Add a regex string. A reason \002has\002 to be specified, use 'none' for default. BE CAREFUL!\n"
    "                To supply a reason consisting of multiple words replace spaces with '_'\n"
    "    DEL         Delete a regex string by reference, see \002LIST\002\n"
    " \n"
    "Details for: \002CONFIG SET\002\n"
    "Syntax: \002CONFIG SET \037option\037 \037value\037\002\n"
    " \n"
    "Allows temporary adjustments to runtime variables. Changes will be lost on services\n"
    "config reloads and restarts. \037option\037 can be one of:\n"
    " \n"
    "    MEMOUSER          Send memos to users on various occasions.\n"
    "    MEMOOPER          Send memos to opers on various occasions.\n"
    "    MEMOSETTERS       Send memos to host setters on various occasions.\n"
    "    DISPLAYMODE       Enable/Disable extended channel output.\n"
    "    DISPLAYCOLOR      Enable/Disable colored channel output.\n"
    "    DISPLAYMAX        Change maximum number of new requests that will be displayed.\n"
    "    TIMER             Change interval between announces.\n"
    "    REQUESTDELAY      Change delay between allowed vHost requests.\n"
    "                      \037Note\037: Services Admins are exempt from the delay.\n"
    "    DENYIDENTUPDATE   Deny ident@host requests.\n"
    " \n"
    "All options besides \002TIMER\002, \002REQUESTDELAY\002 and \002DISPLAYMAX\002 accept\n"
    "values: \0021\002 (ON) and \0020\002 (OFF).\n"
    "\002TIMER\002 accepts: \0020\002 (OFF) and \002>0\002 (INTERVAL). Minimum interval\n"
    "is 10 seconds, values between 1-9 will be adjusted accordingly.\n" 
    "\002REQUESTDELAY\002 accepts: \0020\002 (OFF) and \002>0\002 (DELAY).\n"
    "\002DISPLAYMAX\002 accepts: \0020\002 (NO LIMIT) and \002>0\002 (LIMITED).",
    /* LNG_VHOST_SYNTAX */
    "Available Options:\n"
    "- \002{WAITING|W} {\037nick\037|\037num\037}\002   - lists open vHost requests\n"
    "- \002{ACTIVATE|A} \037nick\037\002         - activate a request\n"
    "- \002{REJECT|R} \037nick\037 [\037reason\037]\002 - reject a request\n"
#ifdef SUPPORT_WILDCARD
    "  Note: Wildcard nick matching is enabled. Supported wildcard characters are '?' and '*'.\n"
#else
    "  Note: Wildcard nick matching is disabled.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Note: Regex nick matching is enabled. Prefix match string with '!'.\n"
#else
    "  Note: Regex nick matching is disabled.\n"
#endif
    " \n"
    "- \002ACTALL [\037reason\037]\002            - activate all requests\n"
    "- \002REJALL [\037reason\037]\002            - reject all requests",
    /* LNG_NO_REQUESTS */
    "No requests found.",
    /* LNG_NOT_ENOUGH_REQUESTS */
    "Not enough requests.",
    /* LNG_NO_MATCHES */
    "No requests matching '%s' found.",
    /* LNG_REGEX_DISABLED */
    "Support for regex nick matching is disabled.",
    /* LNG_WILDCARD_DISABLED */
    "Support for wildcard nick matching is disabled.",
    /* LNG_REGEX_ERROR */
    "Error in '%s': %s",
    /* LNG_CONF_SAVE_DB */
    "Done. Check services channel for possible errors.",
    /* LNG_CONF_REGEX_ADD_FAIL */
    "Could not add regex '%s'",
    /* LNG_CONF_REGEX_ADD */
    "Regex '%s' added.",
    /* LNG_CONF_REGEX_LIST */
    "#%d: '%s' by %s",
    /* LNG_CONF_REGEX_LIST_END */
    "End of regex list.",
    /* LNG_CONF_REGEX_DELETED */
    "All matching regexes deleted.",
    /* LNG_CONF_TEMPORARY */
    "Setting %s temporarily to: %d (%s)",
    /* LNG_VHOST_LISTING */
    "[\037%s\037] Listing open vHost requests...",
    /* LNG_VHOST_REQUEST_COUNT */
    "Request Count: \002%d\002",
    /* LNG_VHOST_REQUEST_DISPLAYED */
    "- Displayed %d request%s(Count: \002%d\002)",
    /* LNG_VHOST_REMINDER */
    "[\037Reminder\037] You have %d vHost%sawaiting approval. To view: !%s waiting",
    /* LNG_KEY_NICK */
    "Nick",
    /* LNG_KEY_OFFLINE */
    "offline",
    /* LNG_KEY_CURRENT */
    "Current",
    /* LNG_KEY_NONE */
    "none",
    /* LNG_KEY_REQUEST */
    "Request",
    /* LNG_KEY_AGO */
    "ago",
    /* LNG_KEY_OPER */
    "Oper",
    /* LNG_KEY_HOST */
    "Host",
    /* LNG_KEY_REASON */
    "Reason",
    /* LNG_REQUEST_ALREADY_APPROVED */
    "Your request '%s' has already been approved. Please type '/msg HostServ on' if the host isn't active.",
    /* LNG_REQUEST_REMOVED_HOST */
    "Pending vHost request for %s automatically removed. New request matched already approved host.",
    /* LNG_REQUEST_REMOVED_GROUP */
    "Pending vHost request for %s automatically removed. New request matched already approved host of grouped nick %s.",
    /* LNG_REQUEST_NOT_SHOWN */
    "[\037Notice\037] New vHost(s) have been requested that have not been displayed.",
    /* LNG_REQUEST_UPDATE_DENIED */
    "Your request has been denied. To change your ident:\n"
    "   1. Request: '/msg HostServ request *@%s'.\n"
    "   2. Change your ident in your client and reconnect.",
    /* LNG_REQUEST_AUTO_APPROVED */
    "Your request has been approved automatically",
    /* LNG_HOST_REJECTED */
    "Your requested vHost of \002%s\002 has been rejected. Reason: %s.",
    /* LNG_HOST_IDENT_REJECTED */
    "Your requested vHost of \002%s\002@\002%s\002 has been rejected. Reason: %s.",
    /* LNG_HOST_ACTIVATED */
    "Your vhost of \002%s\002 is now activated. Reason: %s.",
    /* LNG_HOST_IDENT_ACTIVATED */
    "Your vhost of \002%s\002@\002%s\002 is now activated. Reason: %s.",
    /* LNG_REQUEST_AUTO_REJECTED */
    "Your request has been rejected automatically",
    /* LNG_CONF_EXCEPTION_ADD_FAIL */
    "Could not add exception for nick '%s'.",
    /* LNG_CONF_EXCEPTION_ADD */
    "Exception for nick '%s' added.",
    /* LNG_CONF_EXCEPTION_LIST */
    "#%d: '%s' by %s",
    /* LNG_CONF_EXCEPTION_LIST_END */
    "End of exception list.",
    /* LNG_CONF_EXCEPTION_DELETED */
    "Matching exception deleted.",
    /* LNG_CONF_EXCEPTION_NOT_FOUND */
    "Entry not found on exception list.",
    /* LNG_REQUEST_REMOVED_GONE */
    "Pending vHost request for %s automatically removed. User does not exist anymore.",
    /* LNG_CONFIG_REGEX_SYNTAX */
    "Syntax: \002CONFIG REGEX {ADD \037reason\037 \037regex\037|DEL {\037#num\037|\037nick\037}|LIST}\002",
    /* LNG_CONFIG_EXCEPTION_SYNTAX */
    "Syntax: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#num\037|\037nick\037}|LIST}\002",
    /* LNG_USER_IS_EXEMPT */
    "Exception for user by"
};

/* Spanish translation */
char *langtable_es[] = {
    /* LNG_REQUEST_SYNTAX */
    "Sintaxis: \002REQUEST \037vHost\037\002",
    /* LNG_REQUESTED */
    "Su petici�n de vHost ha sido enviada. Por favor espere a que un operador lo active.",
    /* LNG_REQUEST_WAIT */
    "Por favor espere %d segundos antes de solicitar otro vHost.",
    /* LNG_REQUEST_MEMO */
    "[auto memo] vHost \002%s\002 ha sido solicitado.",
    /* LNG_ACTIVATE_SYNTAX */
    "Sintaxis: \002{ACTIVATE|A} \037nick\037 [\037raz�n\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Nota: Comodines para completado nicks est�n habilitados. Soportados caracteres comodines como '?' y '*'.\n"
#else
    "  Nota: Comodines para completado nicks est�n deshabilitados.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Nota: Expresiones regulares para completado de nicks habilitadas. El prefijo para completar series es '!'.",
#else
    "  Nota: Expresiones regulares para completado de nicks estan deshabilitadas.",
#endif
    /* LNG_ACTALL_SYNTAX */
    "Sintaxis: \002ACTALL [\037raz�n\037]\002",
    /* LNG_ACTIVATED */
    "El vHost de '%s' ha sido activado.",
    /* LNG_ACTIVATE_MEMO */
    "[auto memo] Su petici�n de vHost ha sido aprobada y ha sido activada.",
    /* LNG_ACTIVATE_MEMO_REASON */
    "[auto memo] Su petici�n de vHost ha sido aprobada y ha sido activada. Raz�n: %s",
    /* LNG_REJECT_SYNTAX */
    "Sintaxis: \002{REJECT|R} \037nick\037 [\037raz�n\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Nota: Comodines para completado nicks est�n habilitados. Soportados caracteres comodines como '?' y '*'.\n"
#else
    "  Nota: Comodines para completado nicks est�n deshabilitados.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Nota: Expresiones regulares para completado de nicks habilitadas. El prefijo para completar series es '!'.",
#else
    "  Nota: Expresiones regulares para completado de nicks estan deshabilitadas.",
#endif
    /* LNG_REJALL_SYNTAX */
    "Sintaxis: \002REJALL [\037raz�n\037]\002",
    /* LNG_REJECTED */
    "El vHost de '%s' ha sido rechazado.",
    /* LNG_REJECT_MEMO */
    "[auto memo] El vHost que ha solicitado ha sido rechazado.",
    /* LNG_REJECT_MEMO_REASON */
    "[auto memo] El vHost que ha solicitado ha sido rechazado. Raz�n: %s",
    /* LNG_NO_REQUEST */
    "No se encontraron peticiones del nick '%s'.",
    /* LNG_HELP */
    "    REQUEST     Solicitar un vHost para tu nick",
    /* LNG_HELP_SETTER */
    "    ACTIVATE    Aprobar la petici�n de vHost para un usuario\n"
    "                El atajo \002A\002 esta disponible\n"
    "    ACTALL      Aprobar todas las peticiones de vHosts\n"
    "    REJECT      Rechazar la petici�n de vHost de un usuario\n"
    "                El atajo \002R\002 esta disponible\n"
    "    REJALL      Rechazar todas las peticiones de vHosts\n"
    "    WAITING     Comando conveniente para LIST +req\n"
    "                El atajo \002W\002 esta disponible\n"
    "    CONFIG      Acceso a varias configuraciones",
    /* LNG_HELP_REQUEST */
    "Las peticiones de vHost para su nick necesitan ser activadas por los\n"
    "administradores de la red. Por favor sea paciente con su peticiones\n"
    "ser�n consideradas.",
    /* LNG_HELP_ACTIVATE */
    "Activa el vHost pedido para el nick dado",
    /* LNG_HELP_ACTALL */
    "Activar todas las peticiones de vHosts.",
    /* LNG_HELP_ACTIVATE_MEMO */
    "Un memo informando al usuario tambien le ser� enviado.",
    /* LNG_HELP_REJECT */
    "Rechaza el vHost pedido para el nick dado.",
    /* LNG_HELP_REJECT_MEMO */
    "Un memo informando al usuario tambien le ser� enviado.",
    /* LNG_HELP_REJALL */
    "Rechazar todas las peticiones de vHosts.",
    /* LNG_HELP_REJALL_MEMO */
    "Un memo informando a los usuarios tambien les sera enviado.",
    /* LNG_WAITING_SYNTAX */
    "Sintaxis: \002{WAITING|W} [{\037nick\037|\037num\037}]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Nota: Comodines para completado nicks est�n habilitados. Soportados caracteres comodines como '?' y '*'.\n"
#else
    "  Nota: Comodines para completado nicks est�n deshabilitados.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Nota: Expresiones regulares para completado de nicks habilitadas. El prefijo para completar series es '!'.",
#else
    "  Nota: Expresiones regulares para completado de nicks estan deshabilitadas.",
#endif
    /* LNG_HELP_WAITING */
    "Este comando es usado por conveniencia. Es esencialmente\n"
    "el mismo rendimiento que LIST +req  .\n"
    " \n"
    "La salida puede ser limitada ya sea especificando un nick, en\n"
    "tal caso solo el pedido perteneciente a ese nick sera\n"
    "mostrado, o especificando un numero, en tal caso\n"
    "el numero de entradas mostradas no excederan este numero.\n"
    "Los numeros 0 y menores mostraran el numero total de\n"
    "vHost pedidos.",
    /* LNG_REQUEST_FORBIDDEN */
    "El vHost que ha pedido esta prohibido.",
    /* LNG_REQUEST_FORBIDDEN_REASON */
    "El vHost que ha pedido esta prohibido. Raz�n: %s",
    /* LNG_CONFIG_SYNTAX */
    "Sintaxis: \002CONFIG \037opci�n\037 [\037parametros\037]\002",
    /* LNG_HELP_CONFIG */
    "Permite accesar a varias configuraciones. Las \037opciones\037 pueden ser:\n"
    " \n"
    "    REGEX       Modificar las listas de Expresiones Regulares\n"
    "    SAVE_DB     Recolecci�n manual de pedidos y bases de datos de expresiones regulares a archivos\n"
    "    LIST        Mostrar los valores de la configuraci�n\n"
    "    SET         Ajuste de variables de tiempo de ejecuci�n\n"
    " \n"
    "Detalles para: \002CONFIG REGEX\002\n"
    "Sintaxis: \002CONFIG REGEX {ADD \037raz�n\037 \037regex\037|DEL {\037#num\037|\037nick\037}|LIST}\002\n"
    " \n"
    "Permite la modificaci�n de la lista de expresiones regulares.\n"
    " \n"
    "    LIST        Muestra una lista de expresiones regulares, incluido los numeros en expresionas regulares y nick creador para uso en \002DEL\002\n"
    "    ADD         Agrega una serie de expresiones regulares. Una raz�n \002tiene\002 que ser especificada, use 'none' para el predeterminado. SEA CUIDADOSO!\n"
    "                Para suplir una raz�n consistente en m�ltiples palabras sustituya los espacios con '_'\n"
    "    DEL         Borra una serie de expresiones regulares por referencia, ver \002LIST\002\n"
    " \n"
    "Detalles para: \002CONFIG SET\002\n"
    "Sintaxis: \002CONFIG SET \037opci�n\037 \037valor\037\002\n"
    " \n"
    "Permite ajustes temporales en las variables. Los cambios se perderan en los servicios\n"
    "cuando la configuraci�n sea recargada o reiniciada. Las \037opciones\037 pueden ser:\n"
    " \n"
    "    MEMOUSER          Enviar memos a los usuarios en varias ocasiones.\n"
    "    MEMOOPER          Enviar memos a los operadores en varias ocasiones.\n"
    "    MEMOSETTERS       Enviar memos a los seteadores de vHost en varias ocasiones.\n"
    "    DISPLAYMODE       Activar/Desactivar el canal extendido de salida.\n"
    "    DISPLAYCOLOR      Activar/Desactivar los colores en el canal de salida.\n"
    "    DISPLAYMAX        Cambiar el n�mero m�ximo para que nuevas peticiones sean mostradas.\n"
    "    TIMER             Cambiar el intervalo del tiempo de los anuncios.\n"
    "    REQUESTDELAY      Cambiar el retardo entre las peticiones permitidas para los vHost.\n"
    "                      \037Nota\037: Administradores de Servicios estan excentos del retardo.\n"
    "    DENYIDENTUPDATE   Deny ident@host requests.\n"
    " \n"
    "Todas las opciones ademas de \002TIMER\002, \002REQUESTDELAY\002 y \002DISPLAYMAX\002 son aceptadas\n"
    "valores: \0021\002 (ON) y \0020\002 (OFF).\n"
    "\002TIMER\002 acepta: \0020\002 (OFF) y \002>0\002 (INTERVALO). Intervalo minimo\n"
    "es de 10 segundos, valores entre 1-9 seran ajustados adecuadamente.\n" 
    "\002REQUESTDELAY\002 acepta: \0020\002 (OFF) y \002>0\002 (DELAY).\n"
    "\002DISPLAYMAX\002 acepta: \0020\002 (NO LIMIT) y \002>0\002 (LIMITED).",
    /* LNG_VHOST_SYNTAX */
    "Opciones Disponibles:\n"
    "- \002{WAITING|W} {\037nick\037|\037num\037}\002  - lista las peticiones de vHosts\n"
    "- \002{ACTIVATE|A} \037nick\037\002        - activar una petici�n\n"
    "- \002{REJECT|R} \037nick\037 [\037raz�n\037]\002 - rechazar una petici�n\n"
#ifdef SUPPORT_WILDCARD
    "  Nota: Comodines para completado nicks est�n habilitados. Soportados caracteres comodines como '?' y '*'.\n"
#else
    "  Nota: Comodines para completado nicks est�n deshabilitados.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Nota: Expresiones regulares para completado de nicks habilitadas. El prefijo para completar series es '!'.\n"
#else
    "  Nota: Expresiones regulares para completado de nicks estan deshabilitadas.\n"
#endif
    " \n"
    "- \002ACTALL [\037raz�n\037]\002             - activar todas las peticiones\n"
    "- \002REJALL [\037raz�n\037]\002             - rechazar todas las peticiones",
    /* LNG_NO_REQUESTS */
    "No se encontraron peticiones.",
    /* LNG_NOT_ENOUGH_REQUESTS */
    "No hay suficientes peticiones.",
    /* LNG_NO_MATCHES */
    "No se encontraron peticiones para '%s'.",
    /* LNG_REGEX_DISABLED */
    "Soporte de expresiones regulares de autocompletado de nicks esta deshabilitado.",
    /* LNG_WILDCARD_DISABLED */
    "Soporte de comodines para autocomentado de nick esta deshabilitado.",
    /* LNG_REGEX_ERROR */
    "Error en '%s': %s",
    /* LNG_CONF_SAVE_DB */
    "Hecho. Revise los servicios del canal para posibles errores.",
    /* LNG_CONF_REGEX_ADD_FAIL */
    "No se pudo agregar la expresi�n regular '%s'",
    /* LNG_CONF_REGEX_ADD */
    "Expresi�n regular '%s' agregada.",
    /* LNG_CONF_REGEX_LIST */
    "#%d: '%s' por %s",
    /* LNG_CONF_REGEX_LIST_END */
    "Fin de la lista de expresiones regulares.",
    /* LNG_CONF_REGEX_DELETED */
    "Totas las expresiones regulares de autocompletado han sido borradas.",
    /* LNG_CONF_TEMPORARY */
    "Ajuste %s temporal a: %d (%s)",
    /* LNG_VHOST_LISTING */
    "[\037%s\037] Listando peticiones de vHost abiertas...",
    /* LNG_VHOST_REQUEST_COUNT */
    "Cuenta de las peticiones: \002%d\002",
    /* LNG_VHOST_REQUEST_DISPLAYED */
    "- Mostrando %d petici�n%s(Cuenta: \002%d\002)",
    /* LNG_VHOST_REMINDER */
    "[\037Recordatorio\037] Tu tienes %d vHost%sesperando por aprobaci�n. Para Verlos: !%s waiting",
    /* LNG_KEY_NICK */
    "Nick",
    /* LNG_KEY_OFFLINE */
    "desconectado",
    /* LNG_KEY_CURRENT */
    "Actual",
    /* LNG_KEY_NONE */
    "ninguno",
    /* LNG_KEY_REQUEST */
    "Petici�n",
    /* LNG_KEY_AGO */
    "antes",
    /* LNG_KEY_OPER */
    "Oper",
    /* LNG_KEY_HOST */
    "Host",
    /* LNG_KEY_Raz�n */
    "Raz�n",
    /* LNG_REQUEST_ALREADY_APPROVED */
    "Tu petici�n de '%s' ha sido aprovada. Por favor escribe '/msg HostServ on' si el vHost no ha sido activado.",
    /* LNG_REQUEST_REMOVED_HOST */
    "vHost pendiente de %s ha sido automaticamente removido. Nueva petici�n de vHost ha sido aprovado.",
    /* LNG_REQUEST_REMOVED_GROUP */
    "vHost pendientes solicitados %s han sido removidos automaticamente. Nueva petici�n encontrada ha sido aprobada para el host del nick agrupado %s.",
    /* LNG_REQUEST_NOT_SHOWN */
    "[\037Noticia\037] Nuevo(s) vHost(s) han sido pedidos pero no han sido aun mostrados.",
    /* LNG_REQUEST_UPDATE_DENIED */
    "Tu petici�n ha sido denegada. Para cambiar su ident:\n"
    "   1. Petici�n: '/msg HostServ request *@%s'.\n"
    "   2. Cambia tu ident en tu cliente y reconectate.",
    /* LNG_REQUEST_AUTO_APPROVED */
    "Tu petici�n ha sido aprobada automaticamente",
    /* LNG_HOST_REJECTED */
    "Tu petici�n del vHost para \002%s\002 ha sido rechazada. Raz�n: %s.",
    /* LNG_HOST_IDENT_REJECTED */
    "Tu petici�n del vHost para \002%s\002@\002%s\002 ha sido rechazada. Raz�n: %s.",
    /* LNG_HOST_ACTIVATED */
    "Tu vhost \002%s\002 ya est� activada. Raz�n: %s.",
    /* LNG_HOST_IDENT_ACTIVATED */
    "Tu vhost \002%s\002@\002%s\002 ya est� activada. Raz�n: %s.",
    /* LNG_REQUEST_AUTO_REJECTED */
    "Tu pedido ha sido rechazado autom�ticamente",
    /* LNG_CONF_EXCEPTION_ADD_FAIL */
    "No se puede agregar excepciones para el nick '%s'.",
    /* LNG_CONF_EXCEPTION_ADD */
    "Excepci�n para el nick '%s' agregada.",
    /* LNG_CONF_EXCEPTION_LIST */
    "#%d: '%s' by %s",
    /* LNG_CONF_EXCEPTION_LIST_END */
    "Fin de lista de excepciones.",
    /* LNG_CONF_EXCEPTION_DELETED */
    "Excepci�n concordante eliminada.",
    /* LNG_CONF_EXCEPTION_NOT_FOUND */
    "Entrada no encontrada en lista de excepciones.",
    /* LNG_REQUEST_REMOVED_GONE */
    "Pedido pendiente de vHost para %s removida autom�ticamente. El usuario ya no existe.",
    /* LNG_CONFIG_REGEX_SYNTAX */
    "Sintaxis: \002CONFIG REGEX {ADD \037raz�n\037 \037regex\037|DEL {\037#num\037|\037nick\037}|LIST}\002",
    /* LNG_CONFIG_EXCEPTION_SYNTAX */
    "Sintaxis: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#num\037|\037nick\037}|LIST}\002\n",
    /* LNG_USER_IS_EXEMPT */
    "Excepci�n para usuario por"
};

/* German translation */
char *langtable_de[] = {
    /* LNG_REQUEST_SYNTAX */
    "Syntax: \002REQUEST \037vHost\037\002",
    /* LNG_REQUESTED */
    "Dein vHost wurde beantragt. Bitte warte bis ein Operator �ber ihn entschieden hat.",
    /* LNG_REQUEST_WAIT */
    "Bitte warte %d Sekunden bevor du einen neuen vHost beantragst.",
    /* LNG_REQUEST_MEMO */
    "[auto memo] vHost \002%s\002 wurde beantragt.",
    /* LNG_ACTIVATE_SYNTAX */
    "Syntax: \002{ACTIVATE|A} \037Nick\037 [\037Grund\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Anmerkung: Wildcard Nick Matching ist aktiviert. Unterst�tzte Wildcard Charaktere sind '?' und '*'.\n"
#else
    "  Anmerkung: Wildcard Nick Matching ist deaktiviert.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Anmerkung: Regex Nick Matching ist aktiviert. Suchbegriffe m�ssen mit '!' beginnen.",
#else
    "  Anmerkung: Regex Nick Matching ist deaktiviert.",
#endif
    /* LNG_ACTALL_SYNTAX */
    "Syntax: \002ACTALL [\037Grund\037]\002",
    /* LNG_ACTIVATED */
    "vHost f�r '%s' wurde aktiviert.",
    /* LNG_ACTIVATE_MEMO */
    "[auto memo] Dein beantragter vHost wurde akzeptiert und gesetzt.",
    /* LNG_ACTIVATE_MEMO_REASON */
    "[auto memo] Dein beantragter vHost wurde akzeptiert und gesetzt. Grund: %s",
    /* LNG_REJECT_SYNTAX */
    "Syntax: \002{REJECT|R} \037Nick\037 [\037Grund\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Anmerkung: Wildcard Nick Matching ist aktiviert. Unterst�tzte Wildcard Charaktere sind '?' und '*'.\n"
#else
    "  Anmerkung: Wildcard Nick Matching ist deaktiviert.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Anmerkung: Regex Nick Matching ist aktiviert. Suchbegriffe m�ssen mit '!' beginnen.",
#else
    "  Anmerkung: Regex Nick Matching ist deaktiviert.",
#endif
    /* LNG_REJALL_SYNTAX */
    "Syntax: \002REJALL [\037Grund\037]\002",
    /* LNG_REJECTED */
    "vHost f�r '%s' wurde abgelehnt.",
    /* LNG_REJECT_MEMO */
    "[auto memo] Dein beantragter vHost wurde abgelehnt.",
    /* LNG_REJECT_MEMO_REASON */
    "[auto memo] Dein beantragter vHost wurde abgelehnt. Grund: %s",
    /* LNG_NO_REQUEST */
    "Es wurde kein vHost-Antrag f�r '%s' gefunden.",
    /* LNG_HELP */
    "    REQUEST     Beantrage einen vHost f�r deinen Nick",
    /* LNG_HELP_SETTER */
    "    ACTIVATE    Akzeptier den beantragten vHost eines Users\n"
    "                Das K�rzel \002A\002 ist verf�gbar\n"
    "    ACTALL      Akzeptier alle beantragten vHosts\n"
    "    REJECT      Lehn den beantragten vHost eines Users ab\n"
    "                Das K�rzel \002R\002 ist verf�gbar\n"
    "    REJALL      Lehn alle beantragten vHosts ab\n"
    "    WAITING     Befehl mit der selben Funktion wie LIST +req\n"
    "                Das K�rzel \002W\002 ist verf�gbar\n"
    "    CONFIG      Ver�nder verschiedene Einstellungen",
    /* LNG_HELP_REQUEST */
    "Beantrage einen vHost f�r deinen Nick. �ber dein Antrag wird\n"
    "daraufhin von Netzwerk Administratoren entschieden, was je nachdem\n"
    "einige Zeit dauern kann.",
    /* LNG_HELP_ACTIVATE */
    "Akzeptier den beantragten vHost des Users.",
    /* LNG_HELP_ACTALL */
    "Akzeptier alle beantragten vHosts.",
    /* LNG_HELP_ACTIVATE_MEMO */
    "Dem User wird, falls offline, eine Memo geschickt.",
    /* LNG_HELP_REJECT */
    "Lehn den beantragten vHost des Users ab.",
    /* LNG_HELP_REJECT_MEMO */
    "Dem User wird, falls offline, eine Memo geschickt.",
    /* LNG_HELP_REJALL */
    "Lehn alle beantragten vHosts ab.",
    /* LNG_HELP_REJALL_MEMO */
    "Dem User wird, falls offline, eine Memo geschickt.",
    /* LNG_WAITING_SYNTAX */
    "Syntax: \002{WAITING|W} [{\037Nick\037|\037Nummer\037}]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Anmerkung: Wildcard Nick Matching ist aktiviert. Unterst�tzte Wildcard Charaktere sind '?' und '*'.\n"
#else
    "  Anmerkung: Wildcard Nick Matching ist deaktiviert.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Anmerkung: Regex Nick Matching ist aktiviert. Suchbegriffe m�ssen mit '!' beginnen.",
#else
    "  Anmerkung: Regex Nick Matching ist deaktiviert.",
#endif
    /* LNG_HELP_WAITING */
    "Dieser Befehl wird aus reiner Bequemlichkeit angeboten. Er entspricht\n"
    "der Ausf�hrung von  LIST +req  .\n"
    " \n"
    "Ergebnisse k�nnen eingeschr�nkt werden entweder durch Angabe\n"
    "eines Nicks, in welchem Fall nur dessen spezieller Antrag\n"
    "angezeigt wird, oder durch Angabe einer Nummer, in welchem\n"
    "Fall die Anzahl angezeigter Antr�ge diese Nummer nicht �berschreiten\n"
    "wird. Nummern kleiner oder gleich 0 geben Auskunft �ber die Anzahl\n"
    "aller derzeit offenen vHost-Antr�ge.",
    /* LNG_REQUEST_FORBIDDEN */
    "Dein beantragter vHost ist nicht gestattet.",
    /* LNG_REQUEST_FORBIDDEN_REASON */
    "Dein beantragter vHost ist nicht gestattet. Grund: %s",
    /* LNG_CONFIG_SYNTAX */
    "Syntax: \002CONFIG \037Option\037 [\037Parameter\037]\002",
    /* LNG_HELP_CONFIG */
    "Erlaubt Zugriff auf verschiedene Optionen. \037Option\037 kann sein:\n"
    " \n"
    "    EXCEPTION   Ver�nder die Ausnahme-Liste\n"
    "    REGEX       Ver�nder die Regex-Liste\n"
    "    SAVE_DB     F�hre eine Sicherung der Antrag- und Regex-Datenbanken manuell aus\n"
    "    LIST        Zeige Konfigurations-Variablen an\n"
    "    SET         Justiere verschiedene Variablen in Echtzeit\n"
    " \n"
    "Details f�r: \002CONFIG EXCEPTION\002\n"
    "Syntax: \002CONFIG EXCEPTION {ADD \037Nick\037|DEL {\037#Nummer\037|\037Nick\037}|LIST}\002\n"
    " \n"
    "Erlaubt Ver�nderung der Ausnahme-Liste.\n"
    " \n"
    "    LIST        Zeige Ausnahme-Liste, beinhaltet Ausnahme Nummer und hinzugef�gt-von Nick f�r \002DEL\002 Befehl\n"
    "    ADD         F�ge eine Ausnahme hinzu. Dies wirkt sich auch auf alle Gruppenmitglieder aus.\n"
    "    DEL         L�sche eine Ausnahme per Referenz, siehe \002LIST\002\n"
    " \n"
    "Details f�r: \002CONFIG REGEX\002\n"
    "Syntax: \002CONFIG REGEX {ADD \037Grund\037 \037Regex\037|DEL {\037#Nummer\037|\037Nick\037}|LIST}\002\n"
    " \n"
    "Erlaubt Ver�nderung der Regex-Liste.\n"
    " \n"
    "    LIST        Zeite Regex-Liste, beinhaltet Regex Nummer und Ersteller-Nick f�r \002DEL\002 Befehl\n"
    "    ADD         F�ge eine Regex Zeichenfolge hinzu. Ein Grund \002muss\002 angegeben werde, benutze 'none' f�r einen Standardgrund. SEI VORSICHTIG!\n"
    "                Um einen l�ngeren Grund anzugeben ersetze Leerzeichen mit '_'\n"
    "    DEL         L�sche eine Regex Zeichenfolge per Referenz, siehe \002LIST\002\n"
    " \n"
    "Details f�r: \002CONFIG SET\002\n"
    "Syntax: \002CONFIG SET \037Option\037 \037Wert\037\002\n"
    " \n"
    "Erlaubt vor�bergehende Justierung verschiedener Variablen. Ver�nderungen gehen verloren\n"
    "w�hrend erneutem Laden der Konfigurationsdatei und Neustarts. \037Option\037 kann sein:\n"
    " \n"
    "    MEMOUSER          Sende Memos an Benutzer zu verschiedenen Anl�ssen.\n"
    "    MEMOOPER          Sende Memos an Oper zu verschiedenen Anl�ssen.\n"
    "    MEMOSETTERS       Sende Memos an Host Setter zu verschiedenen Anl�ssen.\n"
    "    DISPLAYMODE       Aktiviere/Deaktiviere erweiterte Channel Anzeige.\n"
    "    DISPLAYCOLOR      Aktiviere/Deaktiviere farbige Channel Anzeige.\n"
    "    DISPLAYMAX        Ver�ndere die maximale Anzahl an neuen Antr�gen die angezeigt wird.\n"
    "    TIMER             Ver�ndere den Intervall zwischen Erinnerungen.\n"
    "    REQUESTDELAY      Ver�ndere, wielange ein Benutzer zwischen 2 vHost Antr�gen warten muss.\n"
    "                      \037Anmerkung\037: Services Admins sind hiervon ausgenommen.\n"
    "    DENYIDENTUPDATE   Verbiete ident@host Antr�ge.\n"
    " \n"
    "Alle Optionen ausser \002TIMER\002, \002REQUESTDELAY\002 und \002DISPLAYMAX\002 akzeptieren\n"
    "folgende Werte: \0021\002 (ON) und \0020\002 (OFF).\n"
    "\002TIMER\002 akzeptiert: \0020\002 (OFF) und \002>0\002 (INTERVAL). Minimaler Interval\n"
    "ist 10 Sekunden, Werte zwischen 1-9 werden entsprechend justiert.\n" 
    "\002REQUESTDELAY\002 akzeptiert: \0020\002 (OFF) und \002>0\002 (DELAY).\n"
    "\002DISPLAYMAX\002 akzeptiert: \0020\002 (NO LIMIT) und \002>0\002 (LIMITED).",
    /* LNG_VHOST_SYNTAX */
    "Verf�gbare Optionen:\n"
    "- \002{WAITING|W} {\037Nick\037|\037Nummer\037}\002 - Zeige alle offenen vHost-Antr�ge an\n"
    "- \002{ACTIVATE|A} \037Nick\037\002          - Akzeptier einen Antrag\n"
    "- \002{REJECT|R} \037Nick\037 [\037Grund\037]\002   - Lehn einen Antrag ab\n"
#ifdef SUPPORT_WILDCARD
    "  Anmerkung: Wildcard Nick Matching ist aktiviert. Unterst�tzte Wildcard Charaktere sind '?' und '*'.\n"
#else
    "  Anmerkung: Wildcard Nick Matching ist deaktiviert.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Anmerkung: Regex Nick Matching ist aktiviert. Suchbegriffe m�ssen mit '!' beginnen.\n"
#else
    "  Anmerkung: Regex Nick Matching ist deaktiviert.\n"
#endif
    " \n"
    "- \002ACTALL [\037Grund\037]\002            - Akzeptier alle beantragten vHosts\n"
    "- \002REJALL [\037Grund\037]\002            - Lehn alle beantragten vHosts ab",
    /* LNG_NO_REQUESTS */
    "Keine Antr�ge vorhanden.",
    /* LNG_NOT_ENOUGH_REQUESTS */
    "Nicht gen�gend Antr�ge vorhanden.",
    /* LNG_NO_MATCHES */
    "Es wurden keine Antr�ge f�r den Such-String '%s' gefunden.",
    /* LNG_REGEX_DISABLED */
    "Unterst�tzung f�r Regex Nick Matching ist deaktiviert.",
    /* LNG_WILDCARD_DISABLED */
    "Unterst�tzung f�r Wildcard Nick Matching ist deaktiviert.",
    /* LNG_REGEX_ERROR */
    "Fehler in '%s': %s",
    /* LNG_CONF_SAVE_DB */
    "Fertig. M�gliche Fehler wurden im Services-Channel bekanntgegeben.",
    /* LNG_CONF_REGEX_ADD_FAIL */
    "Konnte Regex '%s' nicht hinzuf�gen",
    /* LNG_CONF_REGEX_ADD */
    "Regex '%s' hinzugef�gt.",
    /* LNG_CONF_REGEX_LIST */
    "#%d: '%s' von %s",
    /* LNG_CONF_REGEX_LIST_END */
    "Ende der Regex-Liste.",
    /* LNG_CONF_REGEX_DELETED */
    "Alle gefundenen Regexes wurden gel�scht.",
    /* LNG_CONF_TEMPORARY */
    "Setze %s vor�bergehend zu: %d (%s)",
    /* LNG_VHOST_LISTING */
    "[\037%s\037] Zeige offene vHost Antr�ge...",
    /* LNG_VHOST_REQUEST_COUNT */
    "Offene Antr�ge: \002%d\002",
    /* LNG_VHOST_REQUEST_DISPLAYED */
    "- Angezeigte Antr�ge: %d%s(Insgesamt: \002%d\002)", // horrible %s string, this should be changed
    /* LNG_VHOST_REMINDER */
    "[\037Erinnerung\037] Es warten %d vHost%sauf Begutachtung. Um sie anzuzeigen: !%s waiting",
    /* LNG_KEY_NICK */
    "Nick",
    /* LNG_KEY_OFFLINE */
    "offline",
    /* LNG_KEY_CURRENT */
    "Aktuell",
    /* LNG_KEY_NONE */
    "keiner",
    /* LNG_KEY_REQUEST */
    "Antrag",
    /* LNG_KEY_AGO */
    "zuvor",
    /* LNG_KEY_OPER */
    "Oper",
    /* LNG_KEY_HOST */
    "Host",
    /* LNG_KEY_REASON */
    "Grund",
    /* LNG_REQUEST_ALREADY_APPROVED */
    "Dein Antrag '%s' wurde bereits akzeptiert.\n"
    "Falls der Host nicht aktiv sein sollte, f�hre '/msg HostServ on' aus.",
    /* LNG_REQUEST_REMOVED_HOST */
    "Offener vHost Antrag f�r %s wurde automatisch gel�scht. Neuer Antrag entspricht derzeitigem vHost.",
    /* LNG_REQUEST_REMOVED_GROUP */
    "Offener vHost Antrag f�r %s wurde automatisch gel�scht. Neuer Antrag entspricht derzeitigem vHost des gruppierten Nicks %s.",
    /* LNG_REQUEST_NOT_SHOWN */
    "[\037Achtung\037] Ein oder mehrere vHosts wurden beantragt, welche nicht angezeigt werden konnten.",
    /* LNG_REQUEST_UPDATE_DENIED */
    "Dein Antrag wurde abgelehnt. Um deine Ident zu �ndern folge den Anweisungen:\n"
    "   1. Beantrage: '/msg HostServ request *@%s'.\n"
    "   2. �ndere die Ident in deinem Klienten und verbinde dich neu.",
    /* LNG_REQUEST_AUTO_APPROVED */
    "Dein Antrag wurde automatisch aktiviert",
    /* LNG_HOST_REJECTED */
    "Dein beantragter vHost \002%s\002 wurde abgelehnt. Grund: %s.",
    /* LNG_HOST_IDENT_REJECTED */
    "Dein beantragter vHost \002%s\002@\002%s\002 wurde abgelehnt. Grund: %s.",
    /* LNG_HOST_ACTIVATED */
    "Dein beantragter vHost \002%s\002 wurde akzeptiert. Grund: %s.",
    /* LNG_HOST_IDENT_ACTIVATED */
    "Dein beantragter vHost \002%s\002@\002%s\002 wurde akzeptiert. Grund: %s.",
    /* LNG_REQUEST_AUTO_REJECTED */
    "Dein Antrag wurde automatisch abgelehnt",
    /* LNG_CONF_EXCEPTION_ADD_FAIL */
    "Konnte keine Ausnahme f�r '%s' hinzuf�gen.",
    /* LNG_CONF_EXCEPTION_ADD */
    "Ausnahme f�r '%s' hinzugef�gt.",
    /* LNG_CONF_EXCEPTION_LIST */
    "#%d: '%s' von %s",
    /* LNG_CONF_EXCEPTION_LIST_END */
    "Ende der Ausnahme-Liste.",
    /* LNG_CONF_EXCEPTION_DELETED */
    "Gefundener Eintrag wurde gel�scht.",
    /* LNG_CONF_EXCEPTION_NOT_FOUND */
    "Kein Eintrag in der Ausnahme-Liste gefunden.",
    /* LNG_REQUEST_REMOVED_GONE */
    "Offener vHost Antrag f�r %s wurde automatisch gel�scht. Benutzer existiert nicht mehr.",
    /* LNG_CONFIG_REGEX_SYNTAX */
    "Syntax: \002CONFIG REGEX {ADD \037Grund\037 \037Regex\037|DEL {\037#Nummer\037|\037Nick\037}|LIST}\002",
    /* LNG_CONFIG_EXCEPTION_SYNTAX */
    "Syntax: \002CONFIG EXCEPTION {ADD \037Nick\037|DEL {\037#Nummer\037|\037Nick\037}|LIST}\002",
    /* LNG_USER_IS_EXEMPT */
    "Ausnahme-Regel f�r den Benutzer hinzugef�gt von"
};

/* Hungarian translation */
char *langtable_hun[] = {
    /* LNG_REQUEST_SYNTAX */
    "Szintaktika: \002REQUEST \037vHost\037\002",
    /* LNG_REQUESTED */
    "A vHost-od ig�nyl�sre ker�lt. K�rj�k v�rj, am�g egy oper�tor aktiv�lja.",
    /* LNG_REQUEST_WAIT */
    "K�rj�k v�rj %d m�sodpercet, mielott �j vHost-ot k�rsz.",
    /* LNG_REQUEST_MEMO */
    "[auto memo] A(z) \002%s\002 vHost ig�nyl�sre ker�lt.",
    /* LNG_ACTIVATE_SYNTAX */
    "Szintaktika: \002{ACTIVATE|A} \037nick\037 [\037indok\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Megj.: A wildcard nick szur�s akt�v. A t�mogatott joker karakterek: '?' �s '*'.\n"
#else
    "  Megj.: A wildcard nick szur�s inakt�v. \n"
#endif
#ifdef SUPPORT_REGEX
    "  Megj.: A regex nick szur�s akt�v. Kezdd a stringet a k�vetkezovel: '!'.",
#else
    "  Megj.: A regex nick szur�s inakt�v.",
#endif
    /* LNG_ACTALL_SYNTAX */
    "Szintaktika: \002ACTALL [\037indok\037]\002",
    /* LNG_ACTIVATED */
    "A vHost '%s'-hez aktiv�l�sra ker�lt.",
    /* LNG_ACTIVATE_MEMO */
    "[auto memo] A k�rt vHost-od j�v�hagy�sra �s be�ll�t�sra ker�lt.",
    /* LNG_ACTIVATE_MEMO_REASON */
    "[auto memo] Az k�rt vHost-od j�v�hagy�sra �s be�ll�t�sra ker�lt. Indok: %s",
    /* LNG_REJECT_SYNTAX */
    "Szintaktika: \002{REJECT|R} \037nick\037 [\037indok\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Megj,: A wildcard nick szur�s akt�v. A t�mogatott joker karakterek: '?' �s '*'.\n"
#else
    "  Megj.: A wildcard nick szur�s inakt�v. \n"
#endif
#ifdef SUPPORT_REGEX
    "  Megj.: A regex nick szur�s akt�v. Kezdd a stringet a k�vetkezovel: '!'.",
#else
    "  Megj.: A regex nick szur�s inakt�v.",
#endif
    /* LNG_REJALL_SYNTAX */
    "Szintaktika: \002REJALL [\037indok\037]\002",
    /* LNG_REJECTED */
    "A vHost '%s'-hez elutas�t�sra ker�lt.",
    /* LNG_REJECT_MEMO */
    "[auto memo] A k�rt vHost-od elutas�t�sra ker�lt.",
    /* LNG_REJECT_MEMO_REASON */
    "[auto memo] A k�rt vHost-od elutas�t�sra ker�lt. Indok: %s",
    /* LNG_NO_REQUEST */
    "Nem tal�ltunk ig�nyl�st a(z) '%s' nick-hez.",
    /* LNG_HELP */
    "    REQUEST     vHost ig�nyl�se a nick-edhez",
    /* LNG_HELP_SETTER */
    "    ACTIVATE    Egy user ig�nyelt vHost-j�nak j�v�hagy�sa\n"
    "                Haszn�lhat� az \002A\002 r�vid�t�s\n"
    "    ACTALL      Az �sszes ig�nyelt vHost j�v�hagy�sa\n"
    "    REJECT      Egy user ig�nyelt vHost-j�nak elutas�t�sa\n"
    "                Haszn�lhat� az \002R\002 r�vid�t�s\n"
    "    REJALL      Az �sszes ig�nyelt vHost elutas�t�sa\n"
    "    WAITING     K�nyelmi parancs a k�vetkezoh�z: LIST +req\n"
    "                Haszn�lhat� a \002W\002 r�vid�t�s\n"
    "    CONFIG      K�l�nb�zo be�ll�t�sokhoz val� hozz�f�r�s",
    /* LNG_HELP_REQUEST */
    "K�rd a nick-edhez megadott vHost aktiv�l�s�t\n"
    "a h�l�zati adminisztr�torokn�l. K�rj�k l�gy t�relemmel am�g\n"
    "a k�r�sed elb�r�l�sra ker�l.",
    /* LNG_HELP_ACTIVATE */
    "A nick-hez megadott vHost aktiv�l�sa.",
    /* LNG_HELP_ACTALL */
    "Az �sszes ig�nyelt vHost aktiv�l�sa.",
    /* LNG_HELP_ACTIVATE_MEMO */
    "A user-eket t�j�koztat� memo is k�ld�sre ker�l.",
    /* LNG_HELP_REJECT */
    "A nick-hez megadott vHost elutas�t�sa.",
    /* LNG_HELP_REJECT_MEMO */
    "A user-t t�j�koztat� memo is k�ld�sre ker�l.",
    /* LNG_HELP_REJALL */
    "Az �sszes ig�nyelt vHost elutas�t�sa.",
    /* LNG_HELP_REJALL_MEMO */
    "A user-eket t�j�koztat� memo is k�ld�sre ker�l.",
    /* LNG_WAITING_SYNTAX */
    "Szintaktika: \002{WAITING|W} [{\037nick\037|\037sz�m\037}]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Megj,: A wildcard nick szur�s akt�v. A t�mogatott joker karakterek: '?' �s '*'.\n"
#else
    "  Megj.: A wildcard nick szur�s inakt�v. \n"
#endif
#ifdef SUPPORT_REGEX
    "  Megj.: A regex nick szur�s akt�v. Kezdd a stringet a k�vetkezovel: '!'.",
#else
    "  Megj.: A regex nick szur�s inakt�v.",
#endif
    /* LNG_HELP_WAITING */
    "Ez egy k�nyelmi parancs. Alapvetoen ugyanaz,\n"
    "mint egy  LIST +req  .\n"
    " \n"
    "A kimenet korl�tozhat� vagy egy nick megad�s�val,\n"
    "mely esetben csak a nick-hez tartoz� ig�nyl�sek lesznek\n"
    "l�that�ak, vagy egy sz�m megad�s�val, mely esetben\n"
    "a bejegyz�sek sz�ma nem lesz t�bb enn�l a sz�mn�l.\n"
    "A 0 �s a kisebb sz�mok az �sszes nyitott vHost ig�nyl�s\n"
    "list�z�s�t eredm�nyezik.",
    /* LNG_REQUEST_FORBIDDEN */
    "Az �ltalad k�rt vHost le van tiltva.",
    /* LNG_REQUEST_FORBIDDEN_REASON */
    "Az �ltal�d k�rt vHost le van tiltva. Indok: %s",
    /* LNG_CONFIG_SYNTAX */
    "Szintaktika: \002CONFIG \037opci�\037 [\037param�terek\037]\002",
    /* LNG_HELP_CONFIG */
    "K�l�nb�zo be�ll�t�sokat tesz lehetov�. Az \037opci�\037 lehet:\n"
    " \n"
    "    KIV�TEL     Kivltel-lista m�dos�t�sa\n"
    "    REGEX       Regex-ek kist�j�nak m�dos�t�sa\n"
    "    SAVE_DB     Ig�nyl�sek �s regex adatb�zisok manu�lis ment�se file-okba\n"
    "    LIST        Konfig �rt�kek mutat�sa\n"
    "    SET         Runtime v�ltoz�k �ll�t�sa\n"
    " \n"
    "\002CONFIG EXCEPTION\002 r�szletez�se\n"
    "Szintaktika: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#sz�m\037|\037nick\037}|LIST}\002\n"
    " \n"
    "A kiv�tel-lista m�dos�t�s�hoz:\n"
    " \n"
    "    LIST        Kiv�tel-lista megtekint�se, benne a kiv�tel sz�m�val �s a be�ll�t� nick-kel a \002DEL\002-ben val� haszn�lathoz\n"
    "    ADD         Kiv�tel hozz�ad�sa. Minden csoporttagra is �rv�nyes lesz.\n"
    "    DEL         Kiv�tel t�rl�se hivatkoz�s alapj�n, l�sd \002LIST\002\n"
    " \n"
    "\002CONFIG REGEX\002 r�szletez�se\n"
    "Szintaktika: \002CONFIG REGEX {ADD \037indok\037 \037regex\037|DEL {\037#sz�m\037|\037nick\037}|LIST}\002\n"
    " \n"
    "Lehetov� teszi a regex lista m�dos�t�s�t.\n"
    " \n"
    "    LIST        Regex lista, tartalmazza a regex sz�mot, a k�sz�to nick-j�t \002DEL\002-hez\n"
    "    ADD         Regex string hozz�ad�sa. Egy okot \002meg kell\002 adni, haszn�lj 'none'-t alapb�l. L�GY �VATOS!\n"
    "                T�bb sz�t tartalmaz� indokl�sn�l haszn�lj '_' jelet space helyett\n"
    "    DEL         Regex string t�rl�se referencia alapj�n, l�sd \002LIST\002\n"
    " \n"
    "\002CONFIG SET\002\n r�szletez�se"
    "Szintaktika: \002CONFIG SET \037opci�\037 \037�rt�k\037\002\n"
    " \n"
    "Lehetov� teszi a runtime v�ltoz�k ideiglenes m�dos�t�s�t. A m�dos�t�sok a services\n"
    "konfigur�ci� �jrat�lt�sekn�l �s �jraind�t�sokn�l elvesznek. Az \037opci�\037 lehet:\n"
    " \n"
    "    MEMOUSER          Memo-t k�ld a usernek k�l�nb�zo alkalmakkor.\n"
    "    MEMOOPER          Memo-t k�ld az opereknek k�l�nb�zo alkalmakkor.\n"
    "    MEMOSETTERS       Memo-t k�ld a host setter-eknek k�l�nb�zo alkalmakkor.\n"
    "    DISPLAYMODE       Enged�lyezi/Letiltja a kibov�tett csatorna kimenetetet.\n"
    "    DISPLAYCOLOR      Eneged�lyezi/Letiltja a sz�nes csatorna kimenetet.\n"
    "    DISPLAYMAX        Megv�ltoztatja a megjelen�tendo �j ig�nyl�sek max sz�m�t.\n"
    "    TIMER             Megv�ltoztatja a bejelent�sek k�zti idotartamot.\n"
    "    REQUESTDELAY      Megv�ltoztatja az enged�lyezett vHost ig�nyek k�zti k�sleltet�st.\n"
    "                      \037Megj.\037: Services Admin-ok kiv�telt k�peznek a k�sleltet�s al�l.\n"
    "    DENYIDENTUPDATE   Elutas�tja az ident@host k�r�seket.\n"
    " \n"
    "Minden opci� a \002TIMER\002, a \002REQUESTDELAY\002 �s a \002DISPLAYMAX\002 mellett elfogadja\n"
    "az \0021\002 (ON) and \0020\002 (OFF) �rt�keket.\n"
    "A \002TIMER\002 elfogadja: \0020\002 (OFF) and \002>0\002 (INTERVAL). Minimum k�ztes ido\n"
    "10 m�sodperc, az 1-9 k�z�tti �rt�kek korrig�l�sra ker�lnek.\n"
    "\002REQUESTDELAY\002 elfogadja: \0020\002 (OFF) and \002>0\002 (DELAY).\n"
    "\002DISPLAYMAX\002 elfogadja: \0020\002 (NO LIMIT) and \002>0\002 (LIMITED).",
    /* LNG_VHOST_SYNTAX */
    "El�rheto opci�k:\n"
    "- \002{WAITING|W} {\037nick\037|\037sz�m\037}\002   - kilist�zza a nyitott vHost k�relmeket\n"
    "- \002{ACTIVATE|A} \037nick\037\002         - aktiv�l egy k�relmet\n"
    "- \002{REJECT|R} \037nick\037 [\037indok\037]\002   - elutas�t egy k�relmet\n"
#ifdef SUPPORT_WILDCARD
    "  Megj.: A wildcard nick szur�s akt�v. A t�mogatott joker karakterek: '?' �s '*'.\n"
#else
    "  Megj.: A wildcard nick szur�s inakt�v. \n"
#endif
#ifdef SUPPORT_REGEX
    "  Megj.: A regex nick szur�s akt�v. Kezdd a stringet a k�vetkezovel: '!'.\n"
#else
    "  Megj.: A regex nick szur�s inakt�v.\n"
#endif
    " \n"
    "- \002ACTALL [\037indok\037]\002            - az �sszes k�relem aktiv�l�sa\n"
    "- \002REJALL [\037indok\037]\002            - az �sszes k�relem eluta�t�sa",
    /* LNG_NO_REQUESTS */
    "Nincs k�relem.",
    /* LNG_NOT_ENOUGH_REQUESTS */
    "Nincs el�g k�relem.",
    /* LNG_NO_MATCHES */
    "Nem tal�lhat� '%s'-nek megfelelo k�relem.",
    /* LNG_REGEX_DISABLED */
    "Regex nick szur�s inakt�v.",
    /* LNG_WILDCARD_DISABLED */
    "Wildcard nick szur�s inkakt�v.",
    /* LNG_REGEX_ERROR */
    "Hiba '%s'-ben: %s",
    /* LNG_CONF_SAVE_DB */
    "K�sz. Ellenorizd a services csatorn�t a lehets�ges hib�k�rt.",
    /* LNG_CONF_REGEX_ADD_FAIL */
    "'%s' regex-et nem siker�lt hozz�adni",
    /* LNG_CONF_REGEX_ADD */
    "'%s' regex hozz�ad�sra ker�lt.",
    /* LNG_CONF_REGEX_LIST */
    "#%d: '%s' by %s",
    /* LNG_CONF_REGEX_LIST_END */
    "Regex lista v�ge.",
    /* LNG_CONF_REGEX_DELETED */
    "Az �sszes megfelelo regex t�r�lve.",
    /* LNG_CONF_TEMPORARY */
    "%s be�ll�t�s �tmenetileg a k�vetkezo lett: %d (%s)",
    /* LNG_VHOST_LISTING */
    "[\037%s\037] Nyitott vHost k�relmek list�z�sa...",
    /* LNG_VHOST_REQUEST_COUNT */
    "K�relmek sz�ma: \002%d\002",
    /* LNG_VHOST_REQUEST_DISPLAYED */
    "- %d ki�rt k�relem%s(�sszes: \002%d\002)",
    /* LNG_VHOST_REMINDER */
    "[\037Eml�kezteto\037] %d vHost%sj�v�hagy�sra v�r. Megtekint�s: !%s waiting",
    /* LNG_KEY_NICK */
    "Nick",
    /* LNG_KEY_OFFLINE */
    "offline",
    /* LNG_KEY_CURRENT */
    "Jelenlegi",
    /* LNG_KEY_NONE */
    "nincs",
    /* LNG_KEY_REQUEST */
    "K�relem",
    /* LNG_KEY_AGO */
    "ezelott",
    /* LNG_KEY_OPER */
    "Oper",
    /* LNG_KEY_HOST */
    "Host",
    /* LNG_KEY_REASON */
    "Indok",
    /* LNG_REQUEST_ALREADY_APPROVED */
    "A k�r�sed '%s'-re m�r j�v�hagy�sra ker�lt. �rj be '/msg HostServ on' ha a host nem akt�v.",
    /* LNG_REQUEST_REMOVED_HOST */
    "A f�ggoben l�vo vHost k�relem %s-re automatikusan elt�vol�t�sra ker�lt. Az �j k�relem egy m�r j�v�hagyott host-ot �rint.",
    /* LNG_REQUEST_REMOVED_GROUP */
    "A f�ggoben l�vo vHost k�relem %s-re automatikusan elt�vol�t�sra ker�lt. Az �j k�relem a(z) %s csoportos�tott nick-hez tartoz� j�v�hagyott host-ot �rint.",
    /* LNG_REQUEST_NOT_SHOWN */
    "[\037Figyelmeztet�s\037] �j vHost(okat) ig�nyeltek, amik nem ker�ltek megjelen�t�sre.",
    /* LNG_REQUEST_UPDATE_DENIED */
    "K�relem megtagadva. Az ident megv�ltoztat�s�hoz:\n"
    "   1. K�relem: '/msg HostServ request *@%s'.\n"
    "   2. V�ltoztass ident-et a kliensedben �s kapcsol�dj �jra.",
    /* LNG_REQUEST_AUTO_APPROVED */
    "A k�relmed automatikusan elfogad�sra ker�lt",
    /* LNG_HOST_REJECTED */
    "Az k�rt \002%s\002 vHost-od elutas�t�sra ker�lt. Indok: %s.",
    /* LNG_HOST_IDENT_REJECTED */
    "Az k�rt \002%s\002@\002%s\002 vHost elutas�t�sra ker�lt. Indok: %s.",
    /* LNG_HOST_ACTIVATED */
    "A(z) \002%s\002 vhost-od aktiv�l�sra ker�lt. Indok: %s.",
    /* LNG_HOST_IDENT_ACTIVATED */
    "A(z) \002%s\002@\002%s\002 vhost-od aktiv�l�sra ker�lt. Indok: %s.",
    /* LNG_REQUEST_AUTO_REJECTED */
    "A k�r�sed automatikusan elutas�t�sra ker�lt",
    /* LNG_CONF_EXCEPTION_ADD_FAIL */
    "Nem lehetett kiv�telt hozz�adni a(z) '%s' nickhez.",
    /* LNG_CONF_EXCEPTION_ADD */
    "'%s' nickhez a kiv�tel hozz�adva.",
    /* LNG_CONF_EXCEPTION_LIST */
    "#%d: '%s', hozz�ad�: %s",
    /* LNG_CONF_EXCEPTION_LIST_END */
    "Kiv�tel-lista v�ge.",
    /* LNG_CONF_EXCEPTION_DELETED */
    "Megfelelo kiv�telek t�r�lve.",
    /* LNG_CONF_EXCEPTION_NOT_FOUND */
    "Nincs ilyen bejegyz�s a kiv�tel-list�ban.",
    /* LNG_REQUEST_REMOVED_GONE */
    "A f�ggoben l�vo vHost k�r�s a(z) %s nickhez automatikusan elt�vol�t�sra ker�lt. A felhaszn�l� m�r nem l�tezik.",
    /* LNG_CONFIG_REGEX_SYNTAX */
    "Szintaktika: \002CONFIG REGEX {ADD \037indok\037 \037regex\037|DEL {\037#sz�m\037|\037nick\037}|LIST}\002",
    /* LNG_CONFIG_EXCEPTION_SYNTAX */
    "Szintaktika: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#sz�m\037|\037nick\037}|LIST}\002",
    /* LNG_USER_IS_EXEMPT */
    "A felhaszn�l� kiv�tel�t be�ll�totta: "
};
    moduleInsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
    moduleInsertLanguage(LANG_ES, LNG_NUM_STRINGS, langtable_es);
    moduleInsertLanguage(LANG_DE, LNG_NUM_STRINGS, langtable_de);
    moduleInsertLanguage(LANG_HUN, LNG_NUM_STRINGS, langtable_hun);
}

/* EOF */
