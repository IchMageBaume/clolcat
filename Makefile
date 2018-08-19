yeet:
	gcc -Wall -Werror -lm clolcat.c -O3 -o clolcat

install:
	cp clolcat /bin/clolcat

clean:
	rm -f clolcat
