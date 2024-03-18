include ../Makefile.inc

MAKEARGS = 'CFLAGS=${CFLAGS}' 'CC=${CC}' 'ANOPELIBS=${ANOPELIBS}' \
	   'LDFLAGS=${LDFLAGS}' 'BINDEST=${BINDEST}' 'INSTALL=${INSTALL}' \
           'INCLUDEDIR=${INCLUDEDIR}' 'RM=${RM}' 'CP=${CP}' \
      	   'TOUCH=${TOUCH}' 'SHELL=${SHELL}' 'DATDEST=${DATDEST}' \
   	   'RUNGROUP=${RUNGROUP}' 'MODULE_PATH=${MODULE_PATH}' \
	   'PROFILE=${PROFILE}' 'SHARED=${SHARED}' 'MODULEFLAGS=${MODULEFLAGS}'

SRCS=ssl.c
OBJECTS=$(SRCS:.c=.o)
SO_FILES=$(OBJECTS:.o=.s)
CDEFS= -rdynamic -Wall

all: ${OBJECTS} ${SO_FILES}
	@if [ ! -f anope.key ] ; then \
		openssl genrsa -out anope.key 1024 ; \
		openssl req -new -x509 -key anope.key -out anope.crt -days 365 ; \
	fi

install: 
	$(CP) ./*.so $(MODULE_PATH) 
	${CP} ./anope.{key,crt} ${DATDEST}

.c.o:
	$(CC) ${CFLAGS} ${CDEFS} ${MODULEFLAGS} -I../../${INCLUDEDIR} -c $<

.o.s:
	$(CC) ${SHARED} ../../mod_version.o -lssl -lcrypto $< -o $*.so  ${PROFILE}
	@$(TOUCH) $*.s

distclean: spotless

clean:
	rm -f *.o *.s *.so *.c~ core

spotless:
	rm -f *.o *.s *.so *.c~ core *.so Makefile.inc
