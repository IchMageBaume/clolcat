#ifndef _HELP_H_INCLUDED_
#define _HELP_H_INCLUDED_

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
	"Report bugs to https://github.com/IchMageBaume/clolcat/issues>\n"
	"\n";

#endif
