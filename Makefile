CC ?= gcc
CFLAGS ?= -Wall -Werror -O3

.PHONY: install clean

lolcat:
	$(CC) $(CFLAGS) clolcat.c -lm -o clolcat

install:
	install clolcat $(DESTDIR)/clolcat

clean:
	rm -f clolcat
