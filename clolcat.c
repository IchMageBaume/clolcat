#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "help.h"

#define BUFSZ 65536

int use_color = 1;
int spread = 30;
double freq = 0.1;
uint8_t seed = 0;
int animate = 0;
int duration = 12;
int speed = 200;
int invert = 0;
int truecolor = 1;

size_t posX = 0;
size_t posY = 0;
size_t pos_mod = 0;
size_t max_width = 0;
uint32_t* color_map = NULL;

// to minimize writing overhead we buffer output
char* output_buffer = NULL;
off_t output_offs = 0;

void write_rainbow(int fd, char* data, size_t size);
void fputs_rainbow(char* str, FILE* file);
uint32_t rainbow(size_t i);
void exit_ioerr();

int main(int argc, char** argv) {
	int force = 0;
	
	int c;
	while((c = getopt(argc, argv, "p:F:S:ad:s:itfvh")) != -1) {
		switch(c) {
		case 'p':
			spread = atof(optarg) * 10; break;
		case 'F':
			freq = atof(optarg); break;
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
			fputs_rainbow("v1.0\n", stderr); system("stty sane"); return 0;
		case 'h':
			fputs_rainbow(help_str, stderr); system("stty sane"); return 0;
		
		default:
			fputs_rainbow(help_str, stderr); system("stty sane"); return 1;
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

	if(seed == 0) {
		int fd = open("/dev/urandom", O_RDONLY);
		if(fd < 0) exit_ioerr();
		
		read(fd, &seed, 1);

		close(fd);
	}
	posY = seed;

	if(!force && !isatty(1)) {
		use_color = 0;
	}

	if(freq > 0.01) {
		// we dont want the positions to get too high or
		// double inaccuracy might kick in.
		// the color repeats revery time freq * n equals 2 * pi,
		// so we compute an integer that is close to i * freq == 2 * pi 
		// (suffices -0.01 < (i * freq) % (2 * pi) < 0.01)
		// and do posX % i and posY % i to keep them small. 
		// (tested with dmesg | clolcat, looks much better now)
		double two_pi = M_PI * 2;
		for(int i = 1; ; i++) {
			double d = fmod(freq * i + 0.01, two_pi);
			if(d < 0.02) {
				pos_mod = i;	
				break;
			}
		}
	
		// also, since the colors will be repeating every pos_mod characters,
		// we can make a color map and avoid calculating 3
		// sines for every byte from stdin
		color_map = (uint32_t*)malloc(pos_mod * sizeof(uint32_t));
		for(size_t i = 0; i < pos_mod; i++) {
			color_map[i] = rainbow(i);
		}
	}

	char* input_buffer = (char*)malloc(BUFSZ);
	output_buffer = (char*)malloc(BUFSZ * 21);

	if(optind == argc) {
		for(;;) {
			ssize_t ret = read(0, input_buffer, BUFSZ);
			if(ret < 0) {
				exit_ioerr();
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
			
			for(;;) {
				ssize_t ret = read(fd, input_buffer, BUFSZ);
				if(ret < 0) {
					exit_ioerr();
				}
				if(ret == 0) {
					break;
				}

				write_rainbow(1, input_buffer, ret);
			}
			
			close(fd);

		} while(++optind < argc);
	}

	if(isatty(0)) system("stty sane");
	return 0;
}

void write_rainbow(int fd, char* data, size_t size) {
	// ret will never be 0 or less, so do{...} while(written < size); is fine

	if(!use_color) {
		size_t written = 0;
		do {
			ssize_t ret = write(fd, data + written, size - written);

			if(ret < 0) {
				exit_ioerr();
			}

			written += ret;
				
		} while(written < size);
	}
	else if(truecolor) {
		size_t written = 0;
		output_offs = 0;

		do {
			if(data[written] == '\n') {
				posX = 0;
				posY++;
			}
	
			uint32_t color = color_map?
				color_map[(posX + posY) % pos_mod] : 
				rainbow((posX + posY) % pos_mod);
			int esc_len;
			if(!invert) {
				sprintf(output_buffer + output_offs,
						"\033[38;2;%u;%u;%um%n",
						(color & 0xff000000) >> 24,
						(color & 0x00ff0000) >> 16,
						(color & 0x0000ff00) >>  8,
						&esc_len);
			}
			else {
				sprintf(output_buffer + output_offs, "\033[48;2;%u;%u;%um%n",
						(color & 0xff000000) >> 24,
						(color & 0x00ff0000) >> 16,
						(color & 0x0000ff00) >>  8,
						&esc_len);
			}
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

		printf("ill write: %ld\n", output_offs);

		// reuse written for how much buffer written to fd
		written = 0;
		
		do {
			ssize_t ret = write(fd, output_buffer + written, 
				output_offs - written);

			if(ret < 0) {
				exit_ioerr();
			}

			written += ret;
		} while(written < output_offs);
	}
	else {
		// TODO
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

void exit_ioerr() {
	char* errstr = strerror(errno);
	if(errstr) {
		fprintf(stderr, "I/O error: %s\n", errstr);
	}
	else {
		fprintf(stderr, "I/O error: %d\n", errno);
	}
	
	if(isatty(0)) system("stty sane");
	exit(1);
}
