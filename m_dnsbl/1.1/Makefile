include ../Makefile.inc

MAKEARGS = 'CFLAGS=${CFLAGS}' 'CC=${CC}' 'ANOPELIBS=${ANOPELIBS}' \
	   'LDFLAGS=${LDFLAGS}' 'BINDEST=${BINDEST}' 'INSTALL=${INSTALL}' \
           'INCLUDEDIR=${INCLUDEDIR}' 'RM=${RM}' 'CP=${CP}' \
      	   'TOUCH=${TOUCH}' 'SHELL=${SHELL}' 'DATDEST=${DATDEST}' \
   	   'RUNGROUP=${RUNGROUP}' 'MODULE_PATH=${MODULE_PATH}' \
	   'PROFILE=${PROFILE}' 'SHARED=${SHARED}' 'MODULEFLAGS=${MODULEFLAGS}'

SRCS=m_dnsbl.c
OBJECTS=$(SRCS:.c=.o)
SO_FILES=$(OBJECTS:.o=.s)
CDEFS= -rdynamic -Wall

all: ${OBJECTS} ${SO_FILES}

services_executable:
	${MAKE} -C ../../ services MLIBS=-lpthread

install: services_executable
	$(CP) ./*.so $(MODULE_PATH) 

.c.o:
	$(CC) ${CFLAGS} ${CDEFS} ${MODULEFLAGS} -I../../${INCLUDEDIR} -c $<

.o.s:
	$(CC) ${SHARED} ../../mod_version.o -lpthread $< -o $*.so  ${PROFILE}
	@$(TOUCH) $*.s

distclean: spotless

clean:
	rm -f *.o *.s *.so *.c~ core

spotless:
	rm -f *.o *.s *.so *.c~ core *.so Makefile.inc
