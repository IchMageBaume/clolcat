CC ?= gcc
CFLAGS ?= -Wall -Werror -lm -O3

.PHONY: install clean

lolcat:
	$(CC) $(CFLAGS) clolcat.c -o clolcat

install:
	install clolcat $(DESTDIR)/clolcat

clean:
	rm -f clolcat
