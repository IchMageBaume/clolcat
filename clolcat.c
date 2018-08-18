#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char* help_str =
"\n"
"Usage: lolcat [OPTION]... [FILE]...\n"
"\n"
"Concatenate FILE(s), or standard input, to standard output.\n"
"With no FILE, or when FILE is -, read standard input.\n"
"\n"
"  -p, --spread=<f>      Rainbow spread (default: 3.0)\n"
"  -F, --freq=<f>        Rainbow frequency (default: 0.1)\n"
"  -S, --seed=<i>        Rainbow seed, 0 = random (default: 0)\n"
"  -a, --animate         Enable psychedelics\n"
"  -d, --duration=<i>    Animation duration (default: 12)\n"
"  -s, --speed=<f>       Animation speed (default: 20.0)\n"
"  -i, --invert          Invert fg and bg\n"
"  -t, --truecolor       24-bit (truecolor)\n"
"  -f, --force           Force color even when stdout is not a tty\n"
"  -v, --version         Print version and exit\n"
"  -h, --help            Show this message\n"
"\n"
"Examples:\n"
"  lolcat f - g      Output f's contents, then stdin, then g's contents.\n"
"  lolcat            Copy standard input to standard output.\n"
"  fortune | lolcat  Display a rainbow cookie.\n"
"\n"
"Report lolcat bugs to <https://github.com/busyloop/lolcat/issues>\n"
"lolcat home page: <https://github.com/busyloop/lolcat/>\n"
"Report lolcat translation bugs to <http://speaklolcat.com/>\n"
"\n";

// TODO
void fputs_rainbow(char* str, FILE* file) {
	fputs(str, file);
}

int main(int argc, char** argv) {
	int spread = 30;
	int freq = 1;
	int seed = 0;
	int animate = 0;
	int duration = 12;
	int speed = 200;
	int invert = 0;
	int truecolor = 0;
	int force = 0;

	/*
	static struct option long_options[] = {
		{"spread"



	}
	*/ // TODO
	
	int c;
	while((c = getopt(argc, argv, "p:F:S:ad:s:itfvh")) != -1) {
		switch(c) {
		case 'p':
			spread = atof(optarg) * 10; break;
		case 'F':
			freq = atof(optarg) * 10; break;
		case 'S':
			seed = atoi(optarg); break;
		case 'a':
			animate = 1; break;
		case 'd':
			duration = atoi(optarg); break;
		case 's':
			speed = atof(optarg); break;
		case 'i':
			invert = 1; break;
		case 't':
			truecolor = 1; break;
		case 'f':
			force = 1; break;
		case 'v':
			fputs_rainbow("Alpha, don't use.\n", stderr); return 0;
		case 'h':
			fputs_rainbow(help_str, stderr); return 0;
		
		default:
			fputs_rainbow(help_str, stderr); return 1;
		}
		
	}

	printf("%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
		spread, freq, seed, animate, duration, speed, invert, truecolor, force);

	for(int i = optind; i < argc; i++) {
		
	}
}
