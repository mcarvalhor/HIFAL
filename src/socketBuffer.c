#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/socket.h>

#include "socketBuffer.h"



typedef struct __socketBuffer {
	void *inputBuffer, *outputBuffer; /* Input and output buffer memory pointers. */
	size_t bufferLength, inputUsed, outputUsed; /* Length of buffers, used space of input and output buffers. */
	int fileDescriptor; /* System file descriptor. */
} SBUFFER_T;



SBUFFER_T *SB_New(int fileDescriptor, int bufferOptions, size_t bufferLength) {
	SBUFFER_T *aux;
	if(fileDescriptor < 0 || bufferLength < 0) {
		return NULL;
	}

	/* Tries to allocate structure. */
	aux = (SBUFFER_T *) malloc(sizeof(SBUFFER_T));
	if(aux == NULL) {
		return NULL;
	}

	/* Sets values. */
	aux->fileDescriptor = fileDescriptor;
	aux->bufferLength = bufferLength;
	aux->inputBuffer = NULL;
	aux->outputBuffer = NULL;
	aux->inputUsed = 0;
	aux->outputUsed = 0;

	/* Tries to allocate input buffer, if necessary. */
	if(bufferOptions&SBUFFER_OPTIONS_INPUT != 0) {
		aux->inputBuffer = (void *) malloc(bufferLength);
		if(aux->inputBuffer == NULL) {
			free(aux);
			return NULL;
		}
	}

	/* Tries to allocate output buffer, if necessary. */
	if(bufferOptions&SBUFFER_OPTIONS_OUTPUT != 0) {
		aux->outputBuffer = (void *) malloc(bufferLength);
		if(aux->outputBuffer == NULL) {
			free(aux->inputBuffer);
			free(aux);
			return NULL;
		}
	}

	return aux;
}



ssize_t SB_Receive(void *data, size_t length, SBUFFER_T *buffer) {
	ssize_t received, aux;
	if(buffer == NULL || data == NULL || length < 1) {
		return -1;
	}

	if(buffer->inputBuffer == NULL) { /* There's no input buffer. */
		return recv(buffer->fileDescriptor, data, length, 0);
	}

	if(length <= buffer->inputUsed) { /* The 'data' is already available in buffer. */
		memcpy(data, buffer->inputBuffer, length);
		memmove(buffer->inputBuffer, buffer->inputBuffer + length, buffer->inputUsed - length);
		buffer->inputUsed -= length;
		return length;
	}

	/* The full 'data' is not available in buffer, but something may be, so copy what's already available. */
	received = buffer->inputUsed;
	memcpy(data, buffer->inputBuffer, received);
	data += received;
	buffer->inputUsed = 0;

	/* While haven't received full 'data'... */
	while(received < length) {
		aux = recv(buffer->fileDescriptor, buffer->inputBuffer, buffer->bufferLength, 0); /* Receive something from connection. */
		if(aux <= 0) { /* Ops! Error. Return what was already saved at 'data' from buffer. */
			return received;
		}
		if(received + aux > length) { /* Yes! We have received even more data than required, so just store what was missing to 'data' and keep the rest of received bytes in buffer. */
			received = length - received;
			memcpy(data, buffer->inputBuffer, received);
			memmove(buffer->inputBuffer, buffer->inputBuffer + received, aux - received);
			buffer->inputUsed = aux - received;
			return length;
		}
		/* We received less than required data, or exactly what was required. In this case, copy what we received to 'data' and clear buffer. */
		memcpy(data, buffer->inputBuffer, aux);
		buffer->inputUsed = 0;
		received += aux;
		data += aux;
	}
	return received;
}

int SB_ReceiveChar(SBUFFER_T *buffer) {
	char c;
	if(buffer == NULL) {
		return EOF;
	}
	/* Try to receive a single char. */
	if(SB_Receive(&c, sizeof(char), buffer) != sizeof(char)) {
		return EOF;
	}
	return c;
}

int SB_ReceiveLine(char *str, size_t length, char *lineBreak, SBUFFER_T *buffer) {
	size_t lnbrLength;
	int aux;
	if(buffer == NULL || str == NULL || length <= 1) {
		return -2;
	}
	if(lineBreak == NULL) { /* Set default line break, if not provided, as '\n'. */
		lineBreak = "\n";
	}

	length--;
	/* While there's still room for char at 'str' and hasn't reached end of stream or end of line break... */
	for(lnbrLength = 0; length > 0 && lineBreak[lnbrLength] != '\0' && (aux = SB_ReceiveChar(buffer)) != EOF; length--, str++) {
		if(lineBreak[lnbrLength] == aux) { /* Is line break found? */
			lnbrLength++;
		} else {
			lnbrLength = 0; /* No. */
		}
		*str = aux; /* Copy to string. */
	}
	*str = '\0'; /* End of string in C language. */
	if(lineBreak[lnbrLength] == '\0') { /* Perfect! Found line break and everything fit in 'str'. */
		return 0;
	}

	/* Hasn't found end of line break, maybe there was no room left for new chars at string, or reached end of stream. */
	if(aux == EOF) {
		return -1;
	}
	return 1;
}

int SB_FlushInput(SBUFFER_T *buffer) {
	ssize_t aux;
	if(buffer == NULL || buffer->inputBuffer == NULL) {
		return -1;
	}

	/* For input, this is just clearing what we have at buffer. There's no receive or send to be made. */
	buffer->inputUsed = 0;
	return 0;
}


ssize_t SB_Send(void *data, size_t length, SBUFFER_T *buffer) {
	ssize_t sent, aux;
	if(buffer == NULL || data == NULL || length < 1) {
		return -1;
	}

	if(buffer->outputBuffer == NULL) { /* There's no output buffer. */
		return send(buffer->fileDescriptor, data, length, 0);
	}

	if(buffer->outputUsed + length <= buffer->bufferLength) { /* There's room for the 'data' at buffer! Then, just store it. */
		memcpy(buffer->outputBuffer + buffer->outputUsed, data, length);
		buffer->outputUsed += length;
		return length;
	}

	/* There's no room for the full 'data' at buffer. So, store whatever part of 'data' we have room for. */
	sent = buffer->bufferLength - buffer->outputUsed;
	memcpy(buffer->outputBuffer + buffer->outputUsed, data, sent);
	buffer->outputUsed = buffer->bufferLength;
	data += sent;

	/* While haven't sent or buffered all 'data'... */
	while(sent < length) {
		aux = send(buffer->fileDescriptor, buffer->outputBuffer, buffer->outputUsed, 0); /* Send all buffer to the connection. */
		if(aux <= 0) { /* Ops! Error. Just return what we have sent or stored at buffer. */
			return sent;
		}
		if(aux < buffer->outputUsed) { /* Documentation is vague and this is not supposed to happen on blocking sockets. However, I'm treating as error. */
			memmove(buffer->outputBuffer, buffer->outputBuffer + aux, buffer->bufferLength - aux);
			buffer->outputUsed = buffer->bufferLength - aux;
			return sent; /* On non-blocking sockets, this 'return' is necessary. On blocking sockets, not even God knows what to do here, so I accuse error. */
		} else { /* Everything was sent successfully. */
			buffer->outputUsed = 0;
		}
		if(length - sent <= buffer->bufferLength - buffer->outputUsed) { /* Yes! Now the remaining 'data' can be just stored, without any further send. */
			memcpy(buffer->outputBuffer + buffer->outputUsed, data, length - sent);
			buffer->outputUsed += length - sent;
			return length;
		}
		/* There's no room for all the remaining 'data', so we need to store a little bit more and send. */
		memcpy(buffer->outputBuffer + buffer->outputUsed, data, buffer->bufferLength - buffer->outputUsed);
		sent += buffer->bufferLength - buffer->outputUsed;
		data += buffer->bufferLength - buffer->outputUsed;
		buffer->outputUsed = buffer->bufferLength;
	}
	return length;
}

int SB_SendString(char *str, SBUFFER_T *buffer) {
	size_t length;
	if(buffer == NULL || str == NULL) {
		return 0;
	}

	length = strlen(str);
	if(length < 1) { /* Error: there's not a single char to be sent! */
		return 0;
	}

	if(SB_Send(str, length, buffer) != length) { /* Tries to send string (except '\0'). */
		return 0;
	}
	return 1;
}

int SB_SendChar(char c, SBUFFER_T *buffer) {
	if(buffer == NULL) {
		return 0;
	}

	if(SB_Send(&c, sizeof(char), buffer) != sizeof(char)) { /* Tries to send a single char. */
		return 0;
	}
	return 1;
}

int SB_SendLine(char *str, char *lineBreak, SBUFFER_T *buffer) {
	if(buffer == NULL || str == NULL) {
		return 0;
	}
	if(lineBreak == NULL) { /* Set default line break, if not provided, as '\n'. */
		lineBreak = "\n";
	}
	if(SB_SendString(str, buffer) != 1 || SB_SendString(lineBreak, buffer) != 1) { /* Tries to send string and line break. */
		return 0;
	}
	return 1;
}

int SB_SendNumberAsString(long number, SBUFFER_T *buffer) {
	char str[sizeof(number) * 3 + 2];

	str[0] = '\0';
	sprintf(str, "%ld", number);

	return SB_SendString(str, buffer);
}

int SB_FlushOutput(SBUFFER_T *buffer) {
	ssize_t aux;
	if(buffer == NULL || buffer->outputBuffer == NULL) {
		return -1;
	}

	/* There's data to be flushed? */
	if(buffer->outputUsed > 0) {
		aux = send(buffer->fileDescriptor, buffer->outputBuffer, buffer->outputUsed, 0);
		if(aux <= 0) { /* Ops! Error. Can't flush. */
			return -1;
		}
		if(aux < buffer->outputUsed) { /* I don't know how, but some data could be flushed, but not everything. */
			memmove(buffer->outputBuffer, buffer->outputBuffer + aux, buffer->outputUsed - aux);
			buffer->outputUsed -= aux;
			return -1;
		}
		/* Data flushed. Just clear buffer, then. */
		buffer->outputUsed = 0;
	}
	return 0;
}


int SB_GetFileDescriptor(SBUFFER_T *buffer) {
	if(buffer == NULL) {
		return -1;
	}
	return buffer->fileDescriptor; /* Just return file descriptor. */
}


void SB_Destroy(SBUFFER_T *buffer) {
	if(buffer == NULL) {
		return;
	}
	free(buffer->inputBuffer);
	free(buffer->outputBuffer);
	free(buffer);
}

int SB_Close(SBUFFER_T *buffer) {
	if(buffer == NULL) {
		return 0;
	}
	if(SB_FlushOutput(buffer) != 0) {
		return -1;
	}
	if(SB_FlushInput(buffer) != 0) {
		return -1;
	}
	if(shutdown(buffer->fileDescriptor, SHUT_RDWR) != 0) {
		return -1;
	}
	if(close(buffer->fileDescriptor) != 0) {
		return -1;
	}
	SB_Destroy(buffer);
	return 0;
}


