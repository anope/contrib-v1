# Makefile for language module

MAKEARGS = 'CFLAGS=${CFLAGS}' 'CC=${CC}' 'ANOPELIBS=${ANOPELIBS}' \
	   'LDFLAGS=${LDFLAGS}' 'BINDEST=${BINDEST}' 'INSTALL=${INSTALL}' \
           'INCLUDEDIR=${INCLUDEDIR}' 'RM=${RM}' 'CP=${CP}' \
     	   'TOUCH=${TOUCH}' 'SHELL=${SHELL}' 'DATDEST=${DATDEST}' \
   	   'RUNGROUP=${RUNGROUP}' 'USE_MODULES=${USE_MODULES}'

LANGOBJS = en_us
LANGSRCS = en_us.l

LANGCOMP = ./langcomp
#LANGCOMP = ./langcomp -w


all: $(LANGOBJS)

distclean: clean spotless

install: all
	test -d $(DATDEST)/languages/bs_fantasy_ext || mkdir $(DATDEST)/languages/bs_fantasy_ext
	@if [ "$(RUNGROUP)" ] ; then \
		echo chgrp -R $(RUNGROUP) $(DATDEST)/languages/bs_fantasy_ext ; \
		chgrp -R $(RUNGROUP) $(DATDEST)/languages/bs_fantasy_ext ; \
		echo chmod -R g+rw $(DATDEST)/languages/bs_fantasy_ext ; \
		chmod -R g+rw $(DATDEST)/languages/bs_fantasy_ext ; \
		$(CP) $(LANGOBJS) $(DATDEST)/languages/bs_fantasy_ext ; \
		echo chgrp -R $(RUNGROUP) $(DATDEST)/languages/bs_fantasy_ext ; \
		chgrp -R $(RUNGROUP) $(DATDEST)/languages/bs_fantasy_ext/* ; \
		echo chmod -R g+rw $(DATDEST)/languages/bs_fantasy_ext/* ; \
		chmod -R g+rw $(DATDEST)/languages/bs_fantasy_ext/* ; \
	fi
	@if [ ! "$(RUNGROUP)" ] ; then \
		chmod 700 $(DATDEST)/languages/bs_fantasy_ext/ ; \
		$(CP) $(LANGOBJS) $(DATDEST)/languages/bs_fantasy_ext ; \
		chmod 600 $(DATDEST)/languages/bs_fantasy_ext/* ; \
	fi
	
clean:
	rm -f $(LANGOBJS) langcomp index.h

spotless: clean
	rm -f index


en_us:    en_us.l    langcomp index
	./langcomp $@.l

nl:       nl.l    langcomp index
	./langcomp $@.l

langcomp: langcomp.c
	$(CC) $(CFLAGS) langcomp.c -o $@


index.h: index Makefile
	@perl -e <index >$@ 'print STDERR "Generating index.h... "; $$i=0; while (<>) { s/(\n|\r)//g; printf "#define %-32s %d\n", $$_, $$i++; } print "\n#define MOD_NUM_STRINGS $$i\n"; print STDERR "$$i strings\n";'
	cp index.h ../

index: en_us.l
	grep '^[A-Z]' en_us.l >index
