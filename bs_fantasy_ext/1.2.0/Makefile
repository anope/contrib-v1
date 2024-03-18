SRCS= main.c
TARGET=bs_fantasy_ext

all: language
install: lang_install
distclean: clean

include ../Makefile.sub

# Here we force rebuild the index on every make since we suck at detecting changes..s
language:
	(cd lang ; $(MAKE) ${MAKEARGS} spotless all index.h ; )

lang_install:
	(cd lang ; $(MAKE) ${MAKEARGS} install ; )
	
clean:
	rm -f index.h