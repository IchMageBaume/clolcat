#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "help.h"

#define BUFSZ 65536

int use_color = 1;
double freq = 0.1;
uint16_t seed = 0;
int invert = 0;
int ignore_file_errors = 0;

size_t posX = 0;
size_t posY = 0;
size_t pos_mod = 0;
size_t max_width = 0;

// to minimize processor usage we buffer color escape strings
char** color_map = NULL;
uint8_t* color_map_lens = NULL;

// to minimize writing overhead we buffer output
char* output_buffer = NULL;

void write_rainbow(int fd, char* data, size_t size);
void fputs_rainbow(char* str, FILE* file);
uint32_t rainbow(size_t i);

// not used if file won't even open, just for other I/O errors
void exit_ioerr(char* file);

int main(int argc, char** argv) {
	int force = 0;
	int print_help = 0;
	int exit_status = EXIT_SUCCESS;
	
	int c;
	while((c = getopt(argc, argv, "F:S:iqfh")) != -1) {
		switch(c) {
		case 'F':
			freq = atof(optarg); break;
		case 'S':
			seed = atoi(optarg); break;
		case 'i':
			invert = 1; break;
		case 'q':
			ignore_file_errors = 1; break;
		case 'f':
			force = 1; break;
		
		// show help later, after stuff needed for rainbow output
		// has been initialized
		case 'h':
			print_help = 1; break;

		default:
			print_help = 1; exit_status = EXIT_FAILURE; break;
		}
		
	}

	// if a single line is too fat it might break down to multiple visual lines,
	// but the color will be as if it was a single visual line. So we get the
	// width of the terminal and every time it is reached we act as if a \n
	// appeared
	if(isatty(1)) {
		struct winsize termsz;
		ioctl(1, TIOCGWINSZ, &termsz);
		max_width = termsz.ws_col;
	}

	// add some offset to posY for random color start, either based on
	// a random number or a pnrg number seeded by user-given seed
	srand(seed == 0? time(0) : seed);
	posY = rand() & 0xFFFF;

	if(!force && !isatty(1)) {
		use_color = 0;
	}
	else if(!force && freq < 0.001) {
		fputs("Because it is pretty useless and might consume a lot of"
			"memory, without -f I'll refuse to use color frequencies less"
			"than 0.001.\n", stderr);
		use_color = 0;
	}
	else {
		// we dont want the positions to get too high or
		// double inaccuracy might kick in.
		// the color repeats revery time freq * n equals 2 * pi,
		// so we compute an integer that is close to i * freq == 2 * pi 
		// (suffices -freq / 10 < (i * freq) % (2 * pi) < freq / 10)
		// and do posX % i and posY % i to keep them small. 
		// (tested with dmesg | clolcat, looks much better now)
		double two_pi = M_PI * 2;
		for(int i = 1; ; i++) {
			double d = fmod(freq * i + freq / 10, two_pi);
			if(d < freq / 5) {
				pos_mod = i;	
				break;
			}
		}
	
		// also, since the colors will be repeating every pos_mod characters,
		// we can make a color map (with the actual color escape strings)
		// and avoid calculating 3 sines for every byte from stdin
		color_map = (char**)malloc(pos_mod * sizeof(char*));
		color_map_lens = (uint8_t*)malloc(pos_mod * sizeof(uint8_t));
		for(size_t i = 0; i < pos_mod; i++) {
			color_map[i] = (char*)malloc(21);
			uint32_t color = rainbow(i);
			int esc_len;
			if(!invert) {
				sprintf(color_map[i],
					 "\033[38;2;%u;%u;%um%n",
					(color & 0xff000000) >> 24,
					(color & 0x00ff0000) >> 16,
					(color & 0x0000ff00) >>  8,
					&esc_len);
			}
			else {
				sprintf(color_map[i],
					"\033[48;2;%u;%u;%um%n",
					(color & 0xff000000) >> 24,
					(color & 0x00ff0000) >> 16,
					(color & 0x0000ff00) >>  8,
					&esc_len);
			}
			color_map_lens[i] = esc_len;
		}
	}

	// BUFSZ is the maximum number of input bytes to process.
	// 1 input byte generates less than 21 output bytes
	// but Ill roll with this, just to be sure.
	char* input_buffer = (char*)malloc(BUFSZ);
	output_buffer = (char*)malloc(BUFSZ * 21);

	if(print_help) {
		fputs_rainbow(help_str, stderr);
		goto epilog;
	}

	if(optind == argc) {
		for(;;) {
			ssize_t ret = read(0, input_buffer, BUFSZ);
			if(ret < 0) {
				exit_ioerr("stdin");
			}
			if(ret == 0) {
				break;
			}
			
			write_rainbow(1, input_buffer, ret);
		}
	}
	else {
		do {
			int fd = open(argv[optind], O_RDONLY);
			if(fd < 0) {
				if(!ignore_file_errors) {
					fprintf(stderr, "Error: couldn't open %s\n", argv[optind]);
					exit_status = EXIT_FAILURE;
					goto epilog;
				}
				else {
					goto next_file;
				}
			}
			
			for(;;) {
				ssize_t ret = read(fd, input_buffer, BUFSZ);
				if(ret < 0) {
					if(!ignore_file_errors) {
						exit_ioerr(argv[optind]);
					}
					else {
						goto next_file;
					}
				}
				if(ret == 0) {
					break;
				}

				write_rainbow(1, input_buffer, ret);
			}
			
			close(fd);
			
			next_file:
			optind++;
		} while(optind < argc);
	}

	epilog:
	if(isatty(1)) printf("\033[39m\033[49m");
	free(input_buffer);
	free(output_buffer);
	return exit_status;
}

void write_rainbow(int fd, char* data, size_t size) {
	if(!use_color) {
		// just write buffer to fd
		size_t written = 0;
		do {
			ssize_t ret = write(fd, data + written, size - written);

			if(ret < 0) {
				exit_ioerr(NULL);
			}

			written += ret;
				
		} while(written < size);
	}
	else {
		size_t written = 0;
		off_t output_offs = 0;

		do {
			if(data[written] == '\n') {
				posX = 0;
				posY++;
			}


			size_t esc_len = color_map_lens[(posX + posY) % pos_mod];
			memcpy(
				output_buffer + output_offs,
				color_map[(posX + posY) % pos_mod],
				esc_len);
			
			output_buffer[output_offs + esc_len] = data[written];
			
			output_offs += esc_len + 1;
			written++;

			if(max_width && posX == max_width) {
				posX = 0;
				posY++;
			}
			else {
				posX++;
			}
		} while(written < size);

		// reuse written for how much buffer written to fd
		written = 0;
		
		do {
			ssize_t ret = write(fd, output_buffer + written, 
				output_offs - written);

			if(ret < 0) {
				exit_ioerr(NULL);
			}

			written += ret;
		} while(written < output_offs);
	}
}

void fputs_rainbow(char* str, FILE* file) {
	write_rainbow(fileno(file), str, strlen(str));
}

uint32_t rainbow(size_t i) {
	return 
		((uint8_t)(sin(freq * i)                * 127 + 128) << 24) |
		((uint8_t)(sin(freq * i + M_PI / 3 * 2) * 127 + 128) << 16) |
		((uint8_t)(sin(freq * i + M_PI / 3 * 4) * 127 + 128) <<  8) |
		((0x00)                                              <<  0);
}

void exit_ioerr(char* file) {
	char* errstr = strerror(errno);
	if(errstr) {
		fprintf(stderr, "I/O error%s%s: %s\n", file? " on file " : "", file,
			errstr);
	}
	else {
		fprintf(stderr, "I/O error%s%s: %d\n", file? " on file " : "", file,
			errno);
	}
	
	if(isatty(1)) printf("\033[39m\033[49m\n");
	exit(1);
}
