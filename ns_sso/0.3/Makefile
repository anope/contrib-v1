SRCS=ns_sso.c
TARGET=ns_sso

MAKEARGS = 'CFLAGS=${CFLAGS}' 'CC=${CC}' 'ANOPELIBS=${ANOPELIBS}' \
	   'LDFLAGS=${LDFLAGS}' 'BINDEST=${BINDEST}' 'INSTALL=${INSTALL}' \
           'INCLUDEDIR=${INCLUDEDIR}' 'RM=${RM}' 'CP=${CP}' \
      	   'TOUCH=${TOUCH}' 'SHELL=${SHELL}' 'DATDEST=${DATDEST}' \
   	   'RUNGROUP=${RUNGROUP}' 'MODULE_PATH=${MODULE_PATH}' \
	   'PROFILE=${PROFILE}' 'SHARED=${SHARED}' 'MODULEFLAGS=${MODULEFLAGS}'

OBJECTS= $(SRCS:.c=.o)
SO_FILES=$(OBJECTS:.o=.s)
CDEFS= -rdynamic -Wall

all: module

module: $(OBJECTS) so

distclean: spotless

.c.o:
	$(CC) ${CFLAGS} ${CDEFS} ${MODULEFLAGS} -I../ -I../../${INCLUDEDIR} -c $<

so:
	$(CC) ${SHARED} ../../mod_version.o  $(OBJECTS) -L/usr/local/pgsql/lib -lpq -o ../$(TARGET).so ${PROFILE}

clean:
	rm -f *.o *.so *.c~ core

spotless:
	rm -f *~ *.o *.so *.c~ core
