#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>

#include <socketBuffer.h>
#include <mimes.h>
#include <web.h>
#include "hifal.h"

#include <unistd.h>
#include <dirent.h>
#include <sched.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>



#if HIFAL_PATH_MAXLEN >= HIFAL_BUFFER_LENGTH
	#error "Error! HIFAL_PATH_MAXLEN must be < HIFAL_BUFFER_LENGTH."
#endif



typedef struct __hifal_t {
	char *root, *transmissionBuffer, *pathBuffer; /* Server web root and pre-allocated buffers. */
	struct sockaddr_in6 socketAddr; /* Server Address. */
	struct timeval socketIOTimeout; /* Socket IO Timeout. */
	MIMES_T *mimesTable; /* Mime Types reference table. */
	size_t cacheSize; /* Cache size in bytes. */
	int socketFd; /* Server Socket File Descriptor. */
} HIFAL_T;

typedef struct __hifal_connection_t {
	struct sockaddr_in6 clientAddr; /* Client Address. */
	socklen_t clientAddrLength; /* Client Address Length. */
	SBUFFER_T *buffer; /* Connection read/write buffer. */
	char *requestURI; /* Request URI provided by client. */
	char *resource; /* Resource path in file system. */
	/*int fd; /* Connection File Descriptor. */
} CONNECTION_T;



HIFAL_T *HIFAL_CacheNew(char *root, int port, size_t cache) {
	size_t auxDirPathLen;
	char *auxDirPath;
	HIFAL_T *aux;
	DIR *auxDir;
	int auxOn;
	if(root == NULL || port < 0 || port > 65535) {
		return NULL;
	}

	/* Try to alloc structure and buffers. */
	aux = (HIFAL_T *) malloc(sizeof(HIFAL_T));
	if(aux == NULL) {
		return NULL;
	}
	aux->transmissionBuffer = (char *) malloc(sizeof(char) * HIFAL_BUFFER_LENGTH);
	if(aux->transmissionBuffer == NULL) {
		free(aux);
		return NULL;
	}

	/* Get realpath for root folder. */
	aux->root = realpath(root, NULL);
	if(aux->root == NULL) {
		free(aux->transmissionBuffer);
		free(aux);
		return NULL;
	}

	/* Append "/" to the root folder if necessary. */
	auxDirPathLen = strlen(aux->root);
	if(auxDirPathLen > 0 && (aux->root[auxDirPathLen - 1] == '/')) {
		auxDirPath = (char *) realloc(aux->root, sizeof(char) * (auxDirPathLen + 1));
		if(auxDirPath != NULL) {
			aux->root = auxDirPath;
		}
	} else {
		auxDirPath = (char *) realloc(aux->root, sizeof(char) * (auxDirPathLen + 2));
		if(auxDirPath == NULL) {
			free(aux->transmissionBuffer);
			free(aux->root);
			free(aux);
			return NULL;
		} else {
			aux->root = auxDirPath;
		}
		aux->root[auxDirPathLen] = '/';
		aux->root[auxDirPathLen + 1] = '\0';
	}

	aux->pathBuffer = (char *) malloc(sizeof(char) * (HIFAL_PATH_MAXLEN + strlen(aux->root) + 1));
	if(aux->pathBuffer == NULL) {
		free(aux->transmissionBuffer);
		free(aux->root);
		free(aux);
	}

	/* Checks if it really is a folder, and not a file. */
	auxDir = opendir(aux->root);
	if(auxDir == NULL) {
		free(aux->transmissionBuffer);
		free(aux->pathBuffer);
		free(aux->root);
		free(aux);
		return NULL;
	}
	closedir(auxDir);

	/* Adjust OS signal to prevented being killed by "broken pipe". */
	signal(SIGPIPE, SIG_IGN);

	/* Try to create socket. */
	aux->socketFd = socket(AF_INET6, SOCK_STREAM, 0);
	if(aux->socketFd < 0) {
		free(aux->transmissionBuffer);
		free(aux->pathBuffer);
		free(aux->root);
		free(aux);
		return NULL;
	}

	/* Set server options. */
	auxOn = 1;
	if(setsockopt(aux->socketFd, SOL_SOCKET, SO_REUSEADDR, (const char *) &auxOn, sizeof(auxOn)) < 0) {
		close(aux->socketFd);
		free(aux->transmissionBuffer);
		free(aux->pathBuffer);
		free(aux->root);
		free(aux);
		return NULL;
	}
	aux->socketIOTimeout.tv_sec = HIFAL_IO_TIMEOUT;
	aux->socketIOTimeout.tv_usec = 0;
	if(setsockopt(aux->socketFd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &aux->socketIOTimeout, sizeof(aux->socketIOTimeout)) < 0) {
		close(aux->socketFd);
		free(aux->transmissionBuffer);
		free(aux->pathBuffer);
		free(aux->root);
		free(aux);
		return NULL;
	}
	if(setsockopt(aux->socketFd, SOL_SOCKET, SO_SNDTIMEO, (const char *) &aux->socketIOTimeout, sizeof(aux->socketIOTimeout)) < 0) {
		close(aux->socketFd);
		free(aux->transmissionBuffer);
		free(aux->pathBuffer);
		free(aux->root);
		free(aux);
		return NULL;
	}

	/* Bind server. */
	memset(&aux->socketAddr, 0, sizeof(struct sockaddr_in6));
	aux->socketAddr.sin6_family = AF_INET6; /* IPv6. */
	aux->socketAddr.sin6_port = htons(port); /* Run at provided port. */
	aux->socketAddr.sin6_addr = in6addr_any; /* Accepts IPv4 and IPv6. */
	if(bind(aux->socketFd, (const struct sockaddr *) &aux->socketAddr, sizeof(struct sockaddr_in6)) < 0) {
		close(aux->socketFd);
		free(aux->transmissionBuffer);
		free(aux->pathBuffer);
		free(aux->root);
		free(aux);
		return NULL;
	}

	/* Try to allocate Mime Types table. */
	aux->mimesTable = MIMES_New();
	if(aux->mimesTable == NULL) {
		close(aux->socketFd);
		free(aux->transmissionBuffer);
		free(aux->pathBuffer);
		free(aux->root);
		free(aux);
		return NULL;
	}
	MIMES_AddCommon(aux->mimesTable);

	return aux;
}

HIFAL_T *HIFAL_New(char *root, int port) {
	return HIFAL_CacheNew(root, port, HIFAL_CACHE_DEFAULT_SIZE);
}

int _HIFAL_OutputFile(CONNECTION_T *c, HIFAL_T *s) {
	ssize_t i, j, fileLength;
	char *mimeType;
	FILE *fileStream;
	if((fileStream = fopen(c->resource, "rb")) == NULL) {
		return 1;
	}
	if(fseek(fileStream, 0, SEEK_END) != 0) {
		fclose(fileStream);
		return 2;
	}
	fileLength = ftell(fileStream);
	if(fileLength < 0) {
		fclose(fileStream);
		return 2;
	}
	if(fseek(fileStream, 0, SEEK_SET) != 0) {
		fclose(fileStream);
		return 2;
	}
	SB_SendString("HTTP/1.0 200 OK\r\nContent-Type: ", c->buffer);
	mimeType = MIMES_GetMimeForFile(c->resource, s->mimesTable);
	if(mimeType == NULL) {
		SB_SendString("application/octet-stream", c->buffer);
	} else if(strncmp(mimeType, "text/", 5) == 0) {
		SB_SendString(mimeType, c->buffer);
		SB_SendString("; charset=utf-8", c->buffer);
	} else {
		SB_SendString(mimeType, c->buffer);
	}
	SB_SendString("\r\nContent-Length: ", c->buffer);
	SB_SendNumberAsString(fileLength, c->buffer);
	SB_SendString("\r\n\r\n", c->buffer);
	SB_FlushOutput(c->buffer); /* Flushes output to guarantee best performance. */
	for(i = 0; i < fileLength; i += HIFAL_BUFFER_LENGTH * sizeof(char)) {
		j = fread(s->transmissionBuffer, sizeof(char), HIFAL_BUFFER_LENGTH, fileStream);
		if(j <= 0) {
			fclose(fileStream);
			return 0;
		}
		SB_Send(s->transmissionBuffer, j * sizeof(char), c->buffer);
	}
	fclose(fileStream);
	return 0;
}

int _HIFAL_OutputFolder(CONNECTION_T *c, HIFAL_T *s) {
	struct dirent *folderItem;
	DIR *folderStream;
	size_t tmpStrLen;
	char *tmpStr;
	if((folderStream = opendir(c->resource)) == NULL) { /* It's not a folder. */
		return 1;
	}
	if(c->requestURI[strlen(c->requestURI) - 1] != '/') {
		SB_SendString(HIFAL_FOLDERREDIR_MSGSTART, c->buffer);
		SB_SendString(c->requestURI, c->buffer);
		SB_SendString("/", c->buffer);
		SB_SendString(HIFAL_FOLDERREDIR_MSGEND, c->buffer);
		closedir(folderStream);
		return 0;
	}
	/* Try to find 'index.html' in this folder. */
	tmpStr = (char *) realloc(c->resource, sizeof(char) * (strlen(c->resource) + 12)); /* Reallocate to ensure there's memory space for "/index.html". */
	if(tmpStr != NULL) {
		c->resource = tmpStr;
		tmpStrLen = strlen(c->resource);
		if(c->resource[tmpStrLen - 1] != '/') {
			strcat(c->resource, "/");
		}
		strcat(c->resource, "index.html");
		if(_HIFAL_OutputFile(c, s) == 0) {
			closedir(folderStream);
			return 0;
		}
		c->resource[tmpStrLen] = '\0';
	}
	SB_SendString(HIFAL_FOLDERLIST_MSGSTART, c->buffer);
	SB_SendString("<a href=\"../\" style=\"font-weight: bold;\">../</a>", c->buffer);
	while((folderItem = readdir(folderStream)) != NULL) {
		if(folderItem->d_name[0] == '.') { /* Do not list hidden files (starting with '.'). */
			continue;
		}
		SB_SendString("<li><a href=\"", c->buffer);
		SB_SendString(folderItem->d_name, c->buffer);
		SB_SendString("\">", c->buffer);
		SB_SendString(folderItem->d_name, c->buffer);
		#ifdef DT_DIR /* Some compilers or systems don't support this. */
		if(folderItem->d_type == DT_DIR) {
			SB_SendString("/", c->buffer);
		}
		#endif
		SB_SendString("</a></li>", c->buffer);
	}
	SB_SendString(HIFAL_FOLDERLIST_MSGEND, c->buffer);
	closedir(folderStream);
	return 0;
}

void _HIFAL_OutputResource(CONNECTION_T *conn, HIFAL_T *s) {
	char *resourceBuffer;
	size_t resourceLen;
	resourceBuffer = s->pathBuffer;

	/* Appends Request URI to Web Root folder. */
	strcpy(resourceBuffer, s->root);
	resourceLen = strlen(resourceBuffer);
	if(conn->requestURI[0] == '/') {
		strcat(resourceBuffer, &conn->requestURI[1]);
	} else {
		strcat(resourceBuffer, conn->requestURI);
	}

	/* URL Decodes request URI in resource path. */
	Web_UrlDecode(&resourceBuffer[resourceLen]);

	/* Checks if resource realpath is subdirectory or file inside web root folder. */
	conn->resource = realpath(resourceBuffer, NULL);
	if(conn->resource == NULL) { /* Resource doesn't exist OR there isn't enough memory to create its realpath. Either way, output a 404 error. */
		SB_SendString(HIFAL_NOTFOUND_MSG, conn->buffer);
		return;
	}
	if(strncmp(s->root, conn->resource, strlen(s->root)) != 0 && (strncmp(s->root, conn->resource, strlen(conn->resource)) != 0 || strlen(conn->resource) != strlen(s->root) - 1)) { /* Resource is located outside web root folder. */
		SB_SendString(HIFAL_NOTFOUND_MSG, conn->buffer);
		free(conn->resource);
		return;
	}

	if(_HIFAL_OutputFolder(conn, s) == 0) {
		free(conn->resource);
		return;
	}

	if(_HIFAL_OutputFile(conn, s) == 0) {
		free(conn->resource);
		return;
	}

	/* Fallback: not a file and not a folder. Output a 404 error. */
	SB_SendString(HIFAL_NOTFOUND_MSG, conn->buffer);
	free(conn->resource);
	return;
}

int HIFAL_ProcessConnection(CONNECTION_T *conn, HIFAL_T *s) {
	ssize_t i, j, len;
	char *buffer;
	if(conn == NULL || s == NULL) {
		return 0;
	}

	buffer = s->transmissionBuffer; /* Transmission buffer is already pre-allocated. */
	if((j = SB_ReceiveLine(buffer, HIFAL_BUFFER_LENGTH + 1, "\n", conn->buffer)) < 0) { /* Unexpected end of stream. */
		SB_SendString(HIFAL_BADREQUEST_MSG, conn->buffer);
		return 1;
	}
	if(j > 0) { /* Full line didn't fit in buffer. */
		SB_SendString(HIFAL_URITOOLONG_MSG, conn->buffer);
		return 1;
	}
	len = strlen(buffer);

	/* First line of transmission parsing. */
	for(i = 0; i < len && isspace(buffer[i]) != 0; i++); /* Skips 0...* spacing at start of the transmission. */
	for(j = len - 1; j >= i && isspace(buffer[j]) != 0; j--); /* Skips 0...* spacing at end of the first line of the transmission. */
	buffer[j] = '\0';
	len = strlen(&buffer[i]);
	memmove(buffer, &buffer[i], sizeof(char) * (len + 1));

	/* Only GET method is supported. */
	if(strncmp("GET ", buffer, 4) != 0) {
		SB_SendString(HIFAL_METHODNOTALLOWED_MSG, conn->buffer);
		return 1;
	}

	/* Resource URI parsing. */
	for(i = 0; isspace(buffer[i]) == 0 && i < len; i++); /* Skips method. */
	for(; isspace(buffer[i]) != 0 && i < len; i++); /* Skips spacing after method. */
	for(j = i; j < len && buffer[j] != '?' && buffer[j] != '#' && isspace(buffer[j]) == 0; j++); /* Finds end of resource URI (end of line, spacing, start of query, or start of fragment). */
	buffer[j] = '\0';
	len = strlen(&buffer[i]);
	if(len > HIFAL_PATH_MAXLEN) { /* Checks length of request URI. */
		SB_SendString(HIFAL_URITOOLONG_MSG, conn->buffer);
		return 1;
	}

	/* Output HTTP Response. */
	conn->requestURI = &buffer[i];
	_HIFAL_OutputResource(conn, s);
	return 1;
}

int HIFAL_Serve(int *stop, HIFAL_T *s) {
	int auxForever, connFd;
	CONNECTION_T conn;
	if(s == NULL) {
		return 0;
	}

	/* If no stop variable is provided, then serve forever. */
	if(stop == NULL) {
		auxForever = 0;
		stop = &auxForever;
	}

	/* Listen on port. */
	if(listen(s->socketFd, HIFAL_CONNECTION_QUEUE_SIZE) < 0) {
		return 0;
	}

	while(*stop == 0) { /* No need for mutex, since it is just reading the variable value. */
		connFd = accept(s->socketFd, NULL, NULL);

		if(connFd < 0) {
			sched_yield(); /* If for some reason, 'accept' is always returning -1, this prevents high CPU usage. */
			continue;
		}

		/* Get client address. */
		conn.clientAddrLength = sizeof(conn.clientAddr);
		getpeername(connFd, (struct sockaddr *) &conn.clientAddr, (socklen_t *) &conn.clientAddrLength);

		/* Try to allocate read/write buffer. */
		conn.buffer = SB_New(connFd, SBUFFER_OPTIONS_IO, HIFAL_BUFFER_LENGTH);
		if(conn.buffer == NULL) {
			close(connFd);
			continue;
		}

		/*char str[INET6_ADDRSTRLEN];
		if(inet_ntop(AF_INET6, &conn.clientAddr.sin6_addr, str, sizeof(str))) {
			printf("Client connected, using address '%s' and port '%d'.\n", str, ntohs(conn.clientAddr.sin6_port));
		}*/

		/* Process HTTP Request and send HTTP Response. */
		HIFAL_ProcessConnection(&conn, s);

		if(SB_Close(conn.buffer) != 0) {
			SB_Destroy(conn.buffer);
			close(connFd);
		}
	}

	return 1;
}

int HIFAL_ServeForever(HIFAL_T *s) {
	int stop;
	stop = 0;
	return HIFAL_Serve(&stop, s);
}

void HIFAL_Destroy(HIFAL_T *s) {
	if(s == NULL) {
		return;
	}
	close(s->socketFd);
	MIMES_Destroy(s->mimesTable);
	free(s->transmissionBuffer);
	free(s->pathBuffer);
	free(s->root);
	free(s);
}


