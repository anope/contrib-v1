#include "module.h"
#include "timeout.h"
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>

/******************************************************/

static void *new(size_t size)
{
	return scalloc(1, size);
}

/******************************************************/

typedef boolean bool;

#define return_val_if_fail(var, val) \
	if (!(var)) \
		return (val);

typedef struct libol_node_ libol_node_t;
typedef struct libol_list_ libol_list_t;

struct libol_node_
{
	libol_node_t *next, *prev;
	void *data;
};

struct libol_list_
{
	libol_node_t *head, *tail;
	size_t count;
	bool (*compare)(const void *data1, const void *data2);
	void (*free)(void *data);
};

#define LIST_FOREACH(list, var) for (var = (list)->head; var; var = var->next)
#define LIST_FOREACH_SAFE(list, var1, var2) for (var1 = (list)->head, var2 = var1 ? var1->next : NULL; var1; var1 = var2, var2 = var1 ? var1->next : NULL)

bool libol_list_compare_regular(const void *data1, const void *data2)
{
	return data1 == data2;
}

libol_node_t *libol_list_add(libol_list_t *list, void *data)
{
	libol_node_t *node;

	return_val_if_fail(list && data, NULL);

	node = new(sizeof(libol_node_t));
	node->data = data;

	if (list->tail)
	{
		list->tail->next = node;
		node->prev = list->tail;
		list->tail = node;
	}
	else
	{
		list->head = list->tail = node;
	}

	list->count++;

	return node;
}

void *libol_list_del_node(libol_list_t *list, libol_node_t *node)
{
	void *data = NULL;

	return_val_if_fail(list && node, NULL);

	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;
	if (list->head == node)
		list->head = node->next;
	if (list->tail == node)
		list->tail = node->prev;
	
	list->count--;

	if (list->free)
		list->free(node->data);
	else
		data = node->data;
	free(node);

	return data;
}

/******************************************************/

extern char **my_av, **my_envp;

static libol_list_t sockets;
static fd_set read_fds, write_fds;
static int max_fd = 0;

static SSL_CTX *ssl_ctx;
static int pipe_fds[2];

struct socket_info
{
	libol_node_t *node;

	int socket;
	SSL *sslsocket;

	bool dead;
	char outbuf[NET_BUFSIZE];
	size_t outbuf_size;
	char inbuf[BUFSIZE];
	size_t inbuf_size;

	bool (*on_read)(struct socket_info *socket, const char *buffer);

	size_t (*read)(struct socket_info *socket, char *buffer, size_t size);
	size_t (*write)(struct socket_info *socket, const char *buffer, size_t size);
};

static struct socket_info *create_socket(int socket)
{
	struct socket_info *sock = new(sizeof(struct socket_info));
	sock->socket = socket;
	sock->node = libol_list_add(&sockets, sock);
	if (sock->socket > max_fd)
		max_fd = sock->socket;
	FD_SET(sock->socket, &read_fds);
	return sock;
}

static int ssl_connect(struct socket_info *socket)
{
	socket->sslsocket = SSL_new(ssl_ctx);
	SSL_set_connect_state(socket->sslsocket);
	SSL_set_fd(socket->sslsocket, socket->socket);
	return SSL_connect(socket->sslsocket);
}

static void free_socket(void *data)
{
	struct socket_info *socket = data;
	FD_CLR(socket->socket, &read_fds);
	FD_CLR(socket->socket, &write_fds);
	if (socket->sslsocket)
	{
		SSL_shutdown(socket->sslsocket);
		SSL_free(socket->sslsocket);
	}
	close(socket->socket);
	free(socket);
}

/* Not needed currently but could be used in the future
static size_t socket_info_send(struct socket_info *socket, const char *buffer, size_t size)
{
	return send(socket->socket, buffer, size, 0);
}

static size_t socket_info_recv(struct socket_info *socket, char *buffer, size_t size)
{
	return recv(socket->socket, buffer, size, 0);
}

static size_t socket_info_write(struct socket_info *socket, const char *buffer, size_t size)
{
	return write(socket->socket, buffer, size);
}*/

static size_t socket_info_read(struct socket_info *socket, char *buffer, size_t size)
{
	return read(socket->socket, buffer, size);
}

static size_t socket_info_ssl_write(struct socket_info *socket, const char *buffer, size_t size)
{
	return SSL_write(socket->sslsocket, buffer, size);
}

static size_t socket_info_ssl_read(struct socket_info *socket, char *buffer, size_t size)
{
	return SSL_read(socket->sslsocket, buffer, size);
}

static void handle_socket_read(struct socket_info *sock)
{
	char *start, *end, *p;
	int len = sock->read(sock, sock->inbuf + sock->inbuf_size, sizeof(sock->inbuf) - sock->inbuf_size - 1);
	if (len < 0)
	{
		sock->dead = true;
		return;
	}

	sock->inbuf[sock->inbuf_size + len] = 0;

	end = sock->inbuf;
	for (;;)
	{
		start = end;
		end = strchr(start, '\n');

		if (end == NULL)
			break;
	
		*end++ = 0;

		p = strchr(start, '\r');
		if (p)
			*p = 0;

		sock->on_read(sock, start);
	}
	sock->inbuf_size = strlen(start);
	memmove(sock->inbuf, start, sock->inbuf_size);
}

static void handle_socket_write(struct socket_info *sock)
{
	FD_CLR(sock->socket, &write_fds);
	if (sock->dead || !sock->outbuf_size)
		return;
	sock->write(sock, sock->outbuf, sock->outbuf_size);
	*sock->outbuf = 0;
	sock->outbuf_size = 0;
}

static void socket_write(struct socket_info *sock, const char *message)
{
	int msg_len = strlen(message);
	if (msg_len + sock->outbuf_size > sizeof(sock->outbuf))
		handle_socket_write(sock);
	strlcat(sock->outbuf + sock->outbuf_size, message, sizeof(sock->outbuf) - sock->outbuf_size);
	sock->outbuf_size += msg_len;
	FD_SET(sock->socket, &write_fds);
}

static void process_sockets()
{
	fd_set my_read_fds = read_fds, my_write_fds = write_fds;
	struct timeval tval;
	int ret;
	libol_node_t *node, *node2;
	
	tval.tv_sec = ReadTimeout;
	tval.tv_usec = 0;
	
	ret = select(max_fd + 1, &my_read_fds, &my_write_fds, NULL, &tval);

	if (ret == -1)
	{
		quitting = 1;
		alog("[ssl]: Error selecting sockets: %s", strerror(errno));
	}
	else if (ret > 0)
	{
		LIST_FOREACH(&sockets, node)
		{
			struct socket_info *socket = node->data;

			if (FD_ISSET(socket->socket, &my_read_fds))
				handle_socket_read(socket);
			if (FD_ISSET(socket->socket, &my_write_fds))
				handle_socket_write(socket);
		}
	}

	LIST_FOREACH_SAFE(&sockets, node, node2)
	{
		struct socket_info *socket = node->data;

		if (socket->dead)
			libol_list_del_node(&sockets, socket->node);
	}
}

static void services_shutdown(void)
{
	User *u, *next;

	send_event(EVENT_SHUTDOWN, 1, EVENT_START);

	if (!quitmsg)
		quitmsg = "Terminating, reason unknown";
	alog("%s", quitmsg);
	anope_cmd_squit(ServerName, quitmsg);
	Anope_Free(uplink);
	Anope_Free(mod_current_buffer);
	if (ircd->chanmodes)
		Anope_Free(ircd->chanmodes);
	u = firstuser();
	while (u)
	{
		next = nextuser();
		delete_user(u);
		u = next;
	}
	send_event(EVENT_SHUTDOWN, 1, EVENT_STOP);
	disconn(servsock);
	/* First don't unload protocol module, then do so */
	modules_unload_all(true, false);
	modules_unload_all(true, true);
	/* just in case they weren't all removed at least run once */
	ModuleRunTimeDirCleanUp();
}

static void my_main()
{
	volatile time_t last_update, last_expire, last_check, last_DefCon;
	last_update = last_expire = last_check = last_DefCon = time(NULL);

	while (!quitting)
	{
		time_t t = time(NULL);

		if (debug >= 2)
			alog("debug: Top of my_main loop");

		if (save_data || t - last_expire >= ExpireTimeout)
		{
			expire_all();
			last_expire = t;
		}

		if (!readonly && (save_data || t - last_update >= UpdateTimeout))
		{
			if (delayed_quit)
				anope_cmd_global(NULL, "Updating databases on shutdown, please wait.");

			save_databases();

			if (save_data < 0)
				break;

			save_data = 0;
			last_update = t;
		}

		if (DefConTimeOut && (t - last_DefCon >= dotime(DefConTimeOut)))
		{
			resetDefCon(5);
			last_DefCon = t;
		}

		if (delayed_quit)
			break;

		moduleCallBackRun();

		if (t - last_check >= TimeoutCheck)
		{
			check_timeouts();
			last_check = t;
		}
		
		process_sockets();
	}
	
	/* Check for restart instead of exit */
	if (save_data == -2)
	{
#ifdef SERVICES_BIN
		alog("Restarting");
		if (!quitmsg)
			quitmsg = "Restarting";
		anope_cmd_squit(ServerName, quitmsg);
		disconn(servsock);
		close_log();
		execve(SERVICES_BIN, my_av, my_envp);
		if (!readonly)
		{
			open_log();
			log_perror("Restart failed");
			close_log();
		}
		return;
#else
		quitmsg = "Restart attempt failed--SERVICES_BIN not defined (rerun configure)";
#endif
	}

	/* Disconnect and exit */
	services_shutdown();
}

static struct socket_info *uplink_socket;

static bool uplink_on_read(struct socket_info *socket, const char *buffer)
{
	strlcpy(inbuf, buffer, sizeof(inbuf));
	process();
	return true;
}

static bool read_on_write(struct socket_info *socket, const char *buffer)
{
	if (!uplink_socket)
		return true;
	socket_write(uplink_socket, buffer);
	socket_write(uplink_socket, "\r\n");
	return true;
}

static int ssl_event_connect(int argc, char **argv)
{
	struct socket_info *sock;
	int uplink_sock;

	if (!argc || strcmp(argv[0], EVENT_START))
		return MOD_CONT;

	/* Connect to the remote server
	 * This pulled directly from init.c
	 */
	uplink_sock = conn(RemoteServer, RemotePort, LocalHost, LocalPort);
	if (uplink_sock < 0 && RemoteServer2) {
		uplink_sock = conn(RemoteServer2, RemotePort2, LocalHost, LocalPort);
		if (uplink_sock < 0 && RemoteServer3) {
			uplink_sock =
				conn(RemoteServer3, RemotePort3, LocalHost, LocalPort);
			if (uplink_sock < 0) {
				fatal_perror("Can't connect to server");
			} else {
				servernum = 3;
				alog("Connected to Server %d (%s:%d)", servernum,
					 RemoteServer3, RemotePort3);
			}
		} else {
			if (uplink_sock < 0) {
				fatal_perror("Can't connect to server");
			}
			servernum = 2;
			alog("Connected to Server %d (%s:%d)", servernum,
				 RemoteServer2, RemotePort2);
		}
	} else {
		if (uplink_sock < 0) {
			fatal_perror("Can't connect to server");
		}
		servernum = 1;
		alog("Connected to Server %d (%s:%d)", servernum, RemoteServer,
			 RemotePort);
	}
	/* End of crap from init.c */

	servsock = pipe_fds[1];

	sock = create_socket(uplink_sock);
	sock->on_read = uplink_on_read;
	sock->read = socket_info_ssl_read;
	sock->write = socket_info_ssl_write;
	ssl_connect(sock);
	uplink_socket = sock;
	
	sock = create_socket(pipe_fds[0]);
	sock->on_read = read_on_write;
	sock->read = socket_info_read;

	anope_cmd_connect(servernum);
	send_event(EVENT_CONNECT, 1, EVENT_STOP);

	if (!ircd->delay_cl_intro)
		init_tertiary();

	my_main();

	exit(0);
	
	return MOD_CONT;
}

static int ssl_event_restart(int argc, char **argv)
{
	anope_cmd_squit(ServerName, quitmsg);
	/* Not a typo, hack to make restart work */
	process_sockets();
	process_sockets();
	return MOD_CONT;
}

static int ssl_always_accept(int i, X509_STORE_CTX *c)
{
	return 1;
}

int AnopeInit(int argc, char **argv)
{
	EvtHook *hook;

	moduleAddAuthor("Adam");
	moduleAddVersion("1.0");
	moduleSetType(PROTOCOL);
	
	hook = createEventHook(EVENT_CONNECT, ssl_event_connect);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_RESTART, ssl_event_restart);
	moduleAddEventHook(hook);

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);

	sockets.free = free_socket;

	SSL_library_init();
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();

	ssl_ctx = SSL_CTX_new(SSLv23_client_method());

	if (!ssl_ctx)
	{
		alog("Error initializing SSL CTX: %s", ERR_error_string(ERR_get_error(), NULL));
		return MOD_STOP;
	}
	if (!SSL_CTX_use_certificate_file(ssl_ctx, "anope.crt", SSL_FILETYPE_PEM))
	{
		alog("Error loading certificate anope.crt, %s", ERR_error_string(ERR_get_error(), NULL));
		return MOD_STOP;
	}
	if (!SSL_CTX_use_PrivateKey_file(ssl_ctx, "anope.key", SSL_FILETYPE_PEM))
	{
		alog("Error loading private key anope.key, %s", ERR_error_string(ERR_get_error(), NULL));
		return MOD_STOP;
	}
	
	SSL_CTX_set_mode(ssl_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, ssl_always_accept);

	if (pipe(pipe_fds))
	{
		alog("Unable to call pipe(): %s", strerror(errno));
		return MOD_STOP;
	}

	return MOD_CONT;
}

void AnopeFini()
{
	SSL_CTX_free(ssl_ctx);
}

