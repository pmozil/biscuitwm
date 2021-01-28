.POSIX:
ALL_WARNING = -Wall -Wextra -pedantic
ALL_LDFLAGS = -lxcb -lxcb-keysyms $(LDFLAGS)
ALL_CFLAGS = $(CPPFLAGS) $(CFLAGS) -std=c99 $(ALL_WARNING)
PREFIX = /usr/local
LDLIBS = -lm -lxcb-keysyms -lxcb -lxcb-randr -lxcb-shape
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

all:xwm

biscuitwm: wm.o screen_data.o
	$(CC) $(ALL_LDFLAGS) -o biscuitwmwm wm.o  screen_data.o $(LDLIBS)
	rm *.o
	mv -v biscuitwm ~/bin/

wm.o: wm.c wm.h screen_data.h
	$(CC) $(ALL_CFLAGS) wm.c -c

screen_data.o:
	$(CC) $(ALL_CFLAGS) screen_data.c -lxcb -lxcb-randr -c