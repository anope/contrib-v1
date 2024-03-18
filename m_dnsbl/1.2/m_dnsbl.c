#ifdef _WIN32
# define _WIN32_WINNT 0x0400
# include <winsock2.h>
# include <Ws2tcpip.h>
#endif

#include "module.h"

#define mymalloc(size) calloc(1, size)
typedef boolean bool;

typedef struct node_ node_t;
typedef struct dnsbl_list_ dnsbl_list_t;

struct node_
{
	node_t *next, *prev;
	void *data;
};

struct dnsbl_list_
{
	node_t *head, *tail;
	size_t count;
	bool (*compare)(const void *data1, const void *data2);
	void (*free)(void *data);
};

#define LIST_FOREACH(list, var) for (var = (list)->head; var; var = var->next)
#define LIST_FOREACH_SAFE(list, var1, var2) for (var1 = (list)->head, var2 = var1 ? var1->next : NULL; var1; var1 = var2, var2 = var1 ? var1->next : NULL)

node_t *dnsbl_list_add(dnsbl_list_t *list, void *data)
{
	node_t *node;

	node = mymalloc(sizeof(node_t));
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

void *dnsbl_list_del_node(dnsbl_list_t *list, node_t *node)
{
	void *data = NULL;

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

void dnsbl_list_clear(dnsbl_list_t *list)
{
	node_t *n, *tn;

	LIST_FOREACH_SAFE(list, n, tn)
	{
		dnsbl_list_del_node(list, n);
	}
}

#ifdef _WIN32
# pragma comment(lib, "Ws2_32")
# ifndef INET_ADDRSTRLEN
#  define INET_ADDRSTRLEN 16
# endif
typedef HANDLE ThreadHandle;
typedef CRITICAL_SECTION MutexHandle;
#else
# include <pthread.h>
typedef pthread_t ThreadHandle;
typedef pthread_mutex_t MutexHandle;
#endif

struct thread
{
	ThreadHandle handle;
	MutexHandle mutex;
	char *nick;
	struct in_addr addr;
	char *dnsbl;

	int result;
};

dnsbl_list_t dnsbl;
dnsbl_list_t threads;

static void real_entry_point(struct thread *t);
static void thread_free(void *ptr);

#ifdef _WIN32
static bool init_engine() { return true; }
static void shutdown_engine() { }

static DWORD WINAPI entry_point(void *parameter)
{
	struct thread *t = (struct thread *) parameter;
	real_entry_point(t);
	return 0;
}

static bool start_thread(struct thread *t)
{
	t->handle = CreateThread(NULL, 0, entry_point, t, 0, NULL);

	if (!t->handle)
	{
		alog("Unable to spawn thread: %s", strerror(errno));
		thread_free(t);
		return false;
	}

	return true;
}

static void dnsbl_join(ThreadHandle handle)
{
	WaitForSingleObject(handle, INFINITE);
}

static void dnsbl_mutex_init(MutexHandle *handle)
{
	InitializeCriticalSection(handle);
}

static void dnsbl_mutex_lock(MutexHandle *handle)
{
	EnterCriticalSection(handle);
}

static int dnsbl_mutex_trylock(MutexHandle *handle)
{
	return !TryEnterCriticalSection(handle);
}

static void dnsbl_mutex_unlock(MutexHandle *handle)
{
	LeaveCriticalSection(handle);
}

static void dnsbl_mutex_destroy(MutexHandle *handle)
{
	DeleteCriticalSection(handle);
}

int inet_pton(int af, const char *src, void *dst)
{
	int address_length;
	struct sockaddr_storage sa;
	struct sockaddr_in *sin = (struct sockaddr_in *) &sa;
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &sa;

	switch (af)
	{
		case AF_INET:
			address_length = sizeof(struct sockaddr_in);
			break;
		case AF_INET6:
			address_length = sizeof(struct sockaddr_in6);
			break;
		default:
			return -1;
	}

	if (!WSAStringToAddress((LPSTR) src, af, NULL, (LPSOCKADDR) &sa, &address_length))
	{
		switch (af)
		{
			case AF_INET:
				memcpy(dst, &sin->sin_addr, sizeof(struct in_addr));
				break;
			case AF_INET6:
				memcpy(dst, &sin6->sin6_addr, sizeof(struct in6_addr));
				break;
		}
		return 1;
	}

	return 0;
}

const char *inet_ntop(int af, const void *src, char *dst, size_t size)
{
	int address_length;
	DWORD string_length = size;
	struct sockaddr_storage sa;
	struct sockaddr_in *sin = (struct sockaddr_in *) &sa;
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &sa;

	memset(&sa, 0, sizeof(sa));

	switch (af)
	{
		case AF_INET:
			address_length = sizeof(struct sockaddr_in);
			sin->sin_family = af;
			memcpy(&sin->sin_addr, src, sizeof(struct in_addr));
			break;
		case AF_INET6:
			address_length = sizeof(struct sockaddr_in6);
			sin6->sin6_family = af;
			memcpy(&sin6->sin6_addr, src, sizeof(struct in6_addr));
			break;
		default:
			return NULL;
	}

	if (!WSAAddressToString((LPSOCKADDR) &sa, address_length, NULL, dst, &string_length))
		return dst;

	return NULL;
}

#else
static pthread_attr_t threadengine_attr;

static bool init_engine()
{
	if (pthread_attr_init(&threadengine_attr) || pthread_attr_setdetachstate(&threadengine_attr, PTHREAD_CREATE_JOINABLE))
	{
		alog("Unable to initialize threadengine: %s", strerror(errno));
		return false;
	}

	return true;
}

static void shutdown_engine()
{
	pthread_attr_destroy(&threadengine_attr);
}

static void *entry_point(void *parameter)
{
	struct thread *t = (struct thread *) parameter;
	real_entry_point(t);
	pthread_exit(0);
}

static bool start_thread(struct thread *t)
{
	if (pthread_create(&t->handle, &threadengine_attr, entry_point, t))
	{
		alog("Unable to spawn thread: %s", strerror(errno));
		thread_free(t);
		return false;
	}

	return true;
}

static void dnsbl_join(ThreadHandle handle)
{
	pthread_join(handle, NULL);
}

static void dnsbl_mutex_init(MutexHandle *handle)
{
	pthread_mutex_init(handle, NULL);
}

static void dnsbl_mutex_lock(MutexHandle *handle)
{
	pthread_mutex_lock(handle);
}

static int dnsbl_mutex_trylock(MutexHandle *handle)
{
	return pthread_mutex_trylock(handle);
}

static void dnsbl_mutex_unlock(MutexHandle *handle)
{
	pthread_mutex_unlock(handle);
}

static void dnsbl_mutex_destroy(MutexHandle *handle)
{
	pthread_mutex_destroy(handle);
}
#endif

static void thread_free(void *ptr)
{
	struct thread *t = (struct thread *) ptr;

	dnsbl_mutex_destroy(&t->mutex);
	if (t->nick)
		free(t->nick);
	if (t->dnsbl)
		free(t->dnsbl);
	free(t);
}

static MutexHandle dns_mutex;

static void real_entry_point(struct thread *t)
{
	char reverse_addr_buf[INET_ADDRSTRLEN];
	struct in_addr reverse_addr;
	
	dnsbl_mutex_lock(&t->mutex);

	t->result = -1;
	reverse_addr.s_addr = ((t->addr.s_addr & 0xFF) << 24) | ((t->addr.s_addr & 0xFF00) << 8) | ((t->addr.s_addr & 0xFF0000) >> 8) | ((t->addr.s_addr & 0xFF000000) >> 24);

	if (inet_ntop(AF_INET, &reverse_addr, reverse_addr_buf, sizeof(reverse_addr_buf)) != NULL)
	{
		size_t buf_size = strlen(reverse_addr_buf) + strlen(t->dnsbl) + 2;
		char *lookup_address = mymalloc(buf_size);
		struct addrinfo *result;

		snprintf(lookup_address, buf_size, "%s.%s", reverse_addr_buf, t->dnsbl);

		dnsbl_mutex_lock(&dns_mutex);
		if (getaddrinfo(lookup_address, NULL, NULL, &result) == 0)
		{
			if (result && result->ai_addr && result->ai_addr->sa_family == AF_INET)
			{
				struct sockaddr_in *ip = (struct sockaddr_in *) result->ai_addr;
				t->result = (ip->sin_addr.s_addr & 0xFF000000) >> 24;
			}

			freeaddrinfo(result);
		}
		dnsbl_mutex_unlock(&dns_mutex);

		free(lookup_address);
	}

	dnsbl_mutex_unlock(&t->mutex);
}

static time_t dnsbl_expire = 14400;
static bool callback_enabled = false;

static int dnsbl_check(int argc, char **argv)
{
	node_t *node, *node2;
	static char inet_addr[INET_ADDRSTRLEN + 2], reason[BUFSIZE];
	time_t now = time(NULL);

	LIST_FOREACH_SAFE(&threads, node, node2)
	{
		struct thread *t = (struct thread *) node->data;

		if (dnsbl_mutex_trylock(&t->mutex))
			continue;
		else if (t->result == -2)
		{
			dnsbl_mutex_unlock(&t->mutex);
			continue;
		}

		if (t->result >= 0 && inet_ntop(AF_INET, &t->addr, inet_addr + 2, sizeof(inet_addr) - 2) != NULL)
		{
			snprintf(reason, sizeof(reason), "Your host is listed in the DNS blacklist %s with code %d", t->dnsbl, t->result);
			if (!ircd->szline || add_szline(NULL, inet_addr + 2, s_OperServ, now + dnsbl_expire, reason) < 0)
			{
				inet_addr[0] = '*';
				inet_addr[1] = '@';
				add_akill(NULL, inet_addr, s_OperServ, now + dnsbl_expire, reason);
			}
			alog("%s: Connecting user %s (%s) found in DNSBL %s with result %d", s_OperServ, t->nick, inet_addr + 2, t->dnsbl, t->result);
		}
		else if (debug)
			alog("debug: m_dnsbl: Connecting client %s is clean on %s", t->nick, t->dnsbl);

		dnsbl_join(t->handle);
		dnsbl_mutex_unlock(&t->mutex);
		dnsbl_list_del_node(&threads, node);
	}

	if (threads.count)
		moduleAddCallback("m_dnsbl", now + 1, dnsbl_check, 0, NULL);
	else
		callback_enabled = false;

	return MOD_CONT;
}

static int dnsbl_new_nick(int argc, char **argv)
{
	User *u;
	struct in_addr addr;

	if (!dnsbl.count || !argc || !(u = finduser(argv[0])) || !u->hostip || !is_sync(u->server))
		return MOD_CONT;
	
	if (inet_pton(AF_INET, u->hostip, &addr) == 1)
	{
		node_t *node;

		LIST_FOREACH(&dnsbl, node)
		{
			char *dnsbl_name = node->data;
			struct thread *t = mymalloc(sizeof(struct thread));
			dnsbl_mutex_init(&t->mutex);
			t->nick = sstrdup(u->nick);
			t->addr = addr;
			t->dnsbl = sstrdup(dnsbl_name);
			t->result = -2;

			if (start_thread(t))
			{
				dnsbl_list_add(&threads, t);
				
				if (callback_enabled == false)
				{
					moduleAddCallback("m_dnsbl", time(NULL) + 1, dnsbl_check, 0, NULL);
					callback_enabled = true;
				}
			}
		}
	}
	
	return MOD_CONT;
}

static int dnsbl_reload(int argc, char **argv)
{
	int i;
	char *blacklists = NULL;

	Directive confvalues[][1] = {
		{{"DNSBlacklists",
			{{PARAM_STRING, PARAM_RELOAD, &blacklists}}}},
		{{"DNSBanTime",
			{{PARAM_TIME, PARAM_TIME, &dnsbl_expire}}}}
	};

	for (i = 0; i < 2; i++)
		moduleGetConfigDirective(confvalues[i]);
	
	dnsbl_list_clear(&dnsbl);
	if (blacklists)
	{
		int ac;
		char **av;

		alog("m_dnsbl: Set blacklists to: %s", blacklists);

		ac = split_buf(blacklists, &av, 0);
		for (i = 0; i < ac; ++i)
			dnsbl_list_add(&dnsbl, sstrdup(av[i]));
		free(blacklists);
	}

	return MOD_CONT;
}

int AnopeInit()
{
	EvtHook *hook;

	moduleAddAuthor("Adam");
	moduleAddVersion("1.2");

	if (strstr(IRCDModule, "inspircd"))
		ircd->nickip = 1;
	else if (!ircd->nickip)
	{
		alog("Your IRCd does not support NICKIP, m_dnsbl does not support it.");
		return MOD_STOP;
	}

	if (!init_engine())
		return MOD_STOP;
	dnsbl_mutex_init(&dns_mutex);

	dnsbl_reload(0, NULL);
	
	dnsbl.free = free;
	threads.free = thread_free;
	
	hook = createEventHook(EVENT_NEWNICK, dnsbl_new_nick);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_RELOAD, dnsbl_reload);
	moduleAddEventHook(hook);

	return MOD_CONT;
}

void AnopeFini()
{
	shutdown_engine();
	dnsbl_mutex_destroy(&dns_mutex);
}

