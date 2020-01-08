


#ifndef SOCKETBUFFER_H_
#define SOCKETBUFFER_H_
#include <stdlib.h>
#include <unistd.h> /* POSIX */



	#define SBUFFER_OPTIONS_NONE 0
	#define SBUFFER_OPTIONS_INPUT 1 /* Allows buffering for input. */
	#define SBUFFER_OPTIONS_OUTPUT 2 /* Allows buffering for output. */
	#define SBUFFER_OPTIONS_IO 3 /* Allows buffering for input and output. */
	/*#define SBUFFER_OPTIONS_NONBLOCKING 4*/ /* (TODO) This is a non-blocking socket. */



	typedef struct __socketBuffer SBUFFER_T;



	SBUFFER_T *SB_New(int fileDescriptor, int bufferOptions, size_t bufferLength); /* Creates a new Socket Buffer, using options SBUFFER_OPTIONS_*, fileDescriptor and bufferLength. */

	ssize_t SB_Receive(void *data, size_t length, SBUFFER_T *buffer); /* Receive 'length' bytes and store in 'data' from 'buffer'. Returns number os bytes received or negative value on error. */
	int SB_ReceiveChar(SBUFFER_T *buffer); /* Receive a single char from 'buffer', or EOF on error. */
	int SB_ReceiveLine(char *str, size_t length, char *lineBreak, SBUFFER_T *buffer); /* Receive a whole line terminating with 'lineBreak' and store in 'str' with a maximum of 'length' chars from 'buffer'. */
	int SB_FlushInput(SBUFFER_T *buffer); /* Clear the input bufferred data. */

	ssize_t SB_Send(void *data, size_t length, SBUFFER_T *buffer); /* Send 'length' bytes from 'data' to 'buffer'. Returns the number of bytes sent or negative value on error. */
	int SB_SendString(char *str, SBUFFER_T *buffer); /* Send the whole string 'str' to 'buffer'. */
	int SB_SendChar(char c, SBUFFER_T *buffer); /* Send a single char 'c' to 'buffer'. */
	int SB_SendLine(char *str, char *lineBreak, SBUFFER_T *buffer); /* Send a line 'str' terminating with 'lineBreak' to 'buffer'. */
	int SB_SendNumberAsString(long number, SBUFFER_T *buffer); /* Converts 'number' to its string decimal representation and outputs this string to 'buffer'. */
	int SB_FlushOutput(SBUFFER_T *buffer); /* Force sending of all buffered data to 'buffer'. */

	int SB_GetFileDescriptor(SBUFFER_T *buffer); /* Returns the file descriptor from 'buffer'. */

	void SB_Destroy(SBUFFER_T *buffer); /* Destroy memory resources used by 'buffer'. File descriptor used is not closed and buffered data is not flushed. */
	int SB_Close(SBUFFER_T *buffer); /* Destroy memory resources and close file descriptors used by 'buffer'. Buffered data is flushed if possible. */



#endif


