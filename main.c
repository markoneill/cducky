#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "encode.h"

#define DEFAULT_OUTFILE		"inject.bin"
#define DEFAULT_KB_LAYOUT	"languages/us.txt"

void usage(char* name);

int main(int argc, char* argv[]) {
	int c;
	char* infile_path = NULL;
	char* outfile_path = NULL;
	char* keyboard_layout = NULL;

	outfile_path = DEFAULT_OUTFILE;
	keyboard_layout = DEFAULT_KB_LAYOUT;

	while ((c = getopt(argc, argv, "loi:")) != -1) {
		switch (c) {
			case 'l':
				keyboard_layout = optarg;
				break;
			case 'o':
				outfile_path = optarg;
				break;
			case 'i':
				infile_path = optarg;
				break;
			case '?':
				if (optopt == 'i' || optopt == 'o' || optopt == 'l') {
					fprintf(stderr, "Option -%c requires an argument\n", optopt);
					usage(argv[0]);
					exit(EXIT_FAILURE);
				}
			default:
				fprintf(stderr, "Unknown option encountered\n");
				usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (infile_path == NULL) {
		fprintf(stderr, "input file not specified\n");
		exit(EXIT_FAILURE);
	}

	printf("Phoenix_1's C ducky encoder\n\n");	
	printf("Output will be stored at %s\n", outfile_path);
	printf("Using keyboard layout file %s\n", keyboard_layout);
	printf("Loading script file %s\n", infile_path);
	encode(infile_path, outfile_path, keyboard_layout);
	return 0;
}

void usage(char* name) {
	printf("Usage: %s -i infile [-o file]\n", name);
	printf("Example:\n");
        printf("\t%s -i myscript.txt\n", name);
	return;
}

