#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "help.h"

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

void write_rainbow();
void fputs_rainbow();
uint32_t rainbow();
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

	// we dont want the positions to get too high or double inaccuracy might kick in.
	// the color repeats revery time freq * n equals 2 * pi,
	// so we compute an integer that is close to i * freq == 2 * pi 
	// (suffices -0.01 < (i * freq) % (2 * pi) < 0.01)
	// and do posX % i and posY % i to keep them small. (tested with dmesg | clolcat,
	//  looks much better now)
	double two_pi = M_PI * 2;
	for(int i = 1; ; i++) {
		double d = fmod(freq * i + 0.01, two_pi);
		if(d < 0.02) {
			printf("%d * %.2f = %.8f, %.8f %% %.4f = %f\n",
				i, freq, freq * i, freq * i, two_pi, d - 0.01);
			
			posY_mod = i;	
			break;
		}
	}


	char* buf = (char*)malloc(65536);

	if(optind == argc) {
		for(;;) {
			ssize_t ret = read(0, buf, 65536);
			if(ret < 0) {
				exit_ioerr();
			}
			if(ret == 0) {
				break;
			}
			
			write_rainbow(1, buf, ret);
		}
	}
	else {
		do {
			int fd = open(argv[optind], O_RDONLY);
			
			for(;;) {
				ssize_t ret = read(fd, buf, 65536);
				if(ret < 0) {
					exit_ioerr();
				}
				if(ret == 0) {
					break;
				}

				write_rainbow(1, buf, ret);
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
		char escape_str[20];

		do {
			if(data[written] == '\n') {
				posX = 0;
				posY++;
				posY %= pos_mod;
			}

			posX++;
			posX %= pos_mod;
	
			uint32_t color = rainbow();
			int esc_len;
			if(!invert) {
				sprintf(escape_str, "\033[38;2;%u;%u;%um%n",
						(color & 0xff000000) >> 24,
						(color & 0x00ff0000) >> 16,
						(color & 0x0000ff00) >>  8,
						&esc_len);
			}
			else {
				sprintf(escape_str, "\033[48;2;%u;%u;%um%n",
						(color & 0xff000000) >> 24,
						(color & 0x00ff0000) >> 16,
						(color & 0x0000ff00) >>  8,
						&esc_len);
			}

			size_t esc_written = 0;
			do {
				ssize_t ret = write(fd, escape_str + esc_written,
						esc_len - esc_written);
	
				if(ret < 0) {
					exit_ioerr();
				}
	
				esc_written += ret;
					
			} while(esc_written < esc_len);

			ssize_t ret = write(fd, data + written, 1);

			if(ret < 0) {
				exit_ioerr();
			}

			written++;
		} while(written < size);
	}
	else {
		// TODO
	}
}

void fputs_rainbow(char* str, FILE* file) {
	write_rainbow(fileno(file), str, strlen(str));
}

uint32_t rainbow() {
	return 
		((uint8_t)(sin(freq * (posX + posY))                * 127 + 128) << 24) |
		((uint8_t)(sin(freq * (posX + posY) + M_PI / 3 * 2) * 127 + 128) << 16) |
		((uint8_t)(sin(freq * (posX + posY) + M_PI / 3 * 4) * 127 + 128) <<  8) |
		((0x00)                                                          <<  0);
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
