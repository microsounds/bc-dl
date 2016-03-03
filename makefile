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
INPUT=$(wildcard $(SRCDIR)/*.c)
OUTPUT=bc-dl

.PHONY: all clean install uninstall remove

all: $(INPUT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $(OUTPUT) $(INPUT)

clean:
	rm -rf $(OUTPUT)

install: all
	mv $(OUTPUT) $(INSTALLDIR)

uninstall:	
	rm -rf $(INSTALLDIR)/$(OUTPUT)

remove: uninstall
