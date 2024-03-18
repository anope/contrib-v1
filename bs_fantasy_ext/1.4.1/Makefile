SRCS= main.c
TARGET=bs_fantasy_ext

all: module
install: lang_install
distclean: clean

OBJECTS= $(SRCS:.c=.o)
SO_FILES= $(OBJECTS:.o=.s)
CDEFS= -rdynamic -Wall

module: language $(OBJECTS) so

.c.o:
	$(CC) ${CFLAGS} ${CDEFS} ${MODULEFLAGS} -I../ -I../../${INCLUDEDIR} -c $<

so:
	$(CC) ${SHARED} ../../mod_version.o  $(OBJECTS) -o ../$(TARGET).so ${PROFILE}

# Here we force rebuild the index on every make since we suck at detecting changes..
language:
	(cd lang ; $(MAKE) ${MAKEARGS} spotless all index.h ; )

lang_install:
	(cd lang ; $(MAKE) ${MAKEARGS} install ; )

clean:
	rm -f index.h
	rm -f *.o *.s *.so *.c~ core
	(cd lang ; $(MAKE) ${MAKEARGS} distclean ; )