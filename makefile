 #
 #	makefile
 #	This file is part of bc-dl.
 #	See bc-dl.c for copyright or LICENSE for license information.
 #
   
CC=gcc
CFLAGS=-O2 -ansi
LDFLAGS=-lcurl
SRCDIR=src
INCLUDES=-Iinclude
INSTALLDIR=/usr/local/bin
MANDIR=/usr/share/man/man1
MANPAGE=$(OUTPUT).1
INPUT=$(wildcard $(SRCDIR)/*.c)
OUTPUT=bc-dl

.PHONY: all clean install uninstall remove

all: $(INPUT)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUT) $(INPUT) $(LDFLAGS)

clean:
	rm -rf $(OUTPUT)

install: all
	mv $(OUTPUT) $(INSTALLDIR)
	cp $(MANPAGE) $(MANDIR) && gzip -f $(MANDIR)/$(MANPAGE)

uninstall:	
	rm -rf $(INSTALLDIR)/$(OUTPUT)
	rm -rf $(MANDIR)/$(MANPAGE).gz

remove: uninstall
