#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>

#include <hifal.h>



enum {
	ARG_PROGNAME = 0,
	ARG_ROOTPATH,
	ARG_PORT,
	ARG_CACHE
};

int isStrNumber(char *str);



int main(int argc, char **argv) {
	size_t cacheMemSize;
	HIFAL_T *server;
	DIR *root;
	int port;

	if(argc < ARG_PORT + 1 || argc > ARG_CACHE + 1) {
		fprintf(stderr, "Execution command line:\n");
		fprintf(stderr, "\t%s %s %s %s\n", argv[ARG_PROGNAME], "ARG_ROOTPATH", "ARG_PORT", "[ARG_CACHE]");
		fprintf(stderr, "\tWhere:\n");
		fprintf(stderr, "\t\t%s -> %s\n", "ARG_ROOTPATH", "Path to the root folder for the web server.");
		fprintf(stderr, "\t\t%s -> %s\n", "ARG_PORT", "The port at which the server will be listening (recommended: 80 or 8080).");
		fprintf(stderr, "\t\t%s -> %s\n", "[ARG_CACHE]", "Optional. Memory cache size in bytes (recommended: any value >= 2048). If not provided or zero, no cache is used.");
		return EXIT_FAILURE;
	}

	root = opendir(argv[ARG_ROOTPATH]);
	if(root == NULL) {
		fprintf(stderr, "Could not start server.\n");
		fprintf(stderr, "Could not open folder '%s'. Please, confirm whether this folder really exists and check its permissions.\n", argv[ARG_ROOTPATH]);
		return EXIT_FAILURE;
	}
	closedir(root);

	if(isStrNumber(argv[ARG_PORT]) == 0 || (port = atoi(argv[ARG_PORT])) < 0 || port > 65535) {
		fprintf(stderr, "Could not start server.\n");
		fprintf(stderr, "The port '%s' is not valid. Please use a numeric value (%d-%d).\n", argv[ARG_ROOTPATH], 0, 65535);
		return EXIT_FAILURE;
	}

	if(port == 0) {
		printf("Starting server at '%s:%s'...\n", "localhost", "*");
	} else {
		printf("Starting server at '%s:%d'...\n", "localhost", port);
	}
	if(argc == ARG_CACHE + 1) {
		cacheMemSize = atol(argv[ARG_CACHE]);
		server = HIFAL_CacheNew(argv[ARG_ROOTPATH], port, cacheMemSize);
	} else {
		server = HIFAL_New(argv[ARG_ROOTPATH], port);
	}

	if(server == NULL) {
		fprintf(stderr, "Could not start server.\n");
		fprintf(stderr, "Maybe, there's no available memory to initialize it, or port '%d' is invalid/unavailable.\n", port);
		return EXIT_FAILURE;
	}

	HIFAL_ServeForever(server);

	HIFAL_Destroy(server);
	return EXIT_SUCCESS;
}

int isStrNumber(char *str) {
	while(*str != '\0') {
		if(isdigit(*str) == 0) {
			return 0;
		}
		str++;
	}
	return 1;
}


