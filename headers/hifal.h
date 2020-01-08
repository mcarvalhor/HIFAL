


#ifndef HIFAL_H_
#define HIFAL_H_
#include <stdlib.h>



	/* Cache size in bytes. */
	#define HIFAL_CACHE_DEFAULT_SIZE 1024*1024 /* 1 MiB */
	#define HIFAL_CACHE_NOCACHE 0

	/* Incoming connections queue size. */
	#define HIFAL_CONNECTION_QUEUE_SIZE 32

	/* Send and Receive timeout in seconds. */
	#define HIFAL_IO_TIMEOUT 2

	/* Transmission buffer length in bytes. */
	#define HIFAL_BUFFER_LENGTH 1024 /* 1024 or 512 */

	/* Maximum path length in bytes. This must be < HIFAL_BUFFER_LENGTH. */
	#define HIFAL_PATH_MAXLEN 255

	/* Folder file list message start and end. */
	/* TODO: use chuncked encoding. */
	#define HIFAL_FOLDERLIST_MSGSTART "HTTP/1.0 200 Ok\r\nContent-Type: text/html; charset=utf-8\r\n\r\n<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,height=device-height,initial-scale=1\"><meta name=\"robots\" content=\"noindex,nofollow,noarchive,noimageindex\"><style type=\"text/css\">a{color:#000000;}</style><title>File list</title></head><body><h1>Files in this folder:</h1><br><ul>"
	#define HIFAL_FOLDERLIST_MSGEND "</ul></body></html>"

	/* Folder: redirect to append "/" at end of resource URI. */
	#define HIFAL_FOLDERREDIR_MSGSTART "HTTP/1.0 302 Found\r\nLocation:"
	#define HIFAL_FOLDERREDIR_MSGEND "\r\n\r\n"

	/* Not allowed method message. */
	#define HIFAL_METHODNOTALLOWED_MSG "HTTP/1.0 405 Method Not Allowed\r\nAllow: GET\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 23\r\n\r\n405 Method Not Allowed."

	/* Long URI resource name message. */
	#define HIFAL_URITOOLONG_MSG "HTTP/1.0 414 URI Too Long\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 17\r\n\r\n414 URI Too Long."

	/* Bad request message. */
	#define HIFAL_BADREQUEST_MSG "HTTP/1.0 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 16\r\n\r\n400 Bad Request."

	/* Not found message. */
	#define HIFAL_NOTFOUND_MSG "HTTP/1.0 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 14\r\n\r\n404 Not Found."

	/* Unavailable message. */
	#define HIFAL_SERVERUNAVAILABLE_MSG "HTTP/1.0 503 Service Unavailable\r\nRetry-After:120\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 24\r\n\r\n503 Service Unavailable."



	typedef struct __hifal_t HIFAL_T;



	HIFAL_T *HIFAL_New(char *root, int port); /* Creates a new server using 'root' as web root folder and 'port' as TCP port. */
	HIFAL_T *HIFAL_CacheNew(char *root, int port, size_t cache); /* Creates a new server using 'root' as web root folder, 'port' as TCP port and 'cache' as cache size (in bytes). */

	int HIFAL_Serve(int *stop, HIFAL_T *s); /* Start serving 's' while variable 'stop' contains value '0'. This is a blocking function. */
	int HIFAL_ServeForever(HIFAL_T *s); /* Start serving 's' until process is killed. This is a blocking function. */

	void HIFAL_Destroy(HIFAL_T *s); /* Destroy memory resources and close file descriptors used by server 's'. */



#endif


