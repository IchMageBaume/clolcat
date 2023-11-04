#ifndef _HELP_H_INCLUDED_
#define _HELP_H_INCLUDED_

char* help_str =
	"\n"
	"Usage: clolcat [OPTION]... [FILE]...\n"
	"\n"
	"Concatenate FILE(s), or standard input, to standard output.\n"
	"With no FILE, or when FILE is -, read standard input.\n"
	"\n"
	"  -F                    Rainbow frequency (default: 0.1)\n"
	"  -S                    Rainbow seed, 0 = random (default: 0)\n"
	"  -i                    Invert fg and bg\n"
	"  -q                    Continue even if a file couldn\'t be read\n"
	"  -f                    Force color even when stdout is not a tty and\n"
	"                            force usage of high frequency\n"
	"  -h                    Show this help\n"
	"\n"
	"Examples:\n"
	"  clolcat f - g      Output f's contents, then stdin, then g's contents.\n"
	"  clolcat            Copy standard input to standard output.\n"
	"  fortune | clolcat  Display a rainbow cookie.\n"
	"\n"
	"Report bugs to https://github.com/IchMageBaume/clolcat/issues>\n"
	"\n";

#endif
