#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>



#define MIMES_HASHTABLE_LENGTH 16 /* Hash table LENGTH (number os items). */
#define MIMES_EXTENSION_LENGTH 7
#define MIMES_NAME_LENGTH 63



typedef struct __mimes_node_t {
	struct __mimes_node_t *next; /* Next node in this hashtable row. */
	char extension[MIMES_EXTENSION_LENGTH + 1]; /* File extension. */
	char name[MIMES_NAME_LENGTH + 1]; /* Name of Mime Type. */
	char caseSensitive; /* Case insensitive or case sensitive. */
} MIMES_NODE_T;

typedef struct __mimes_t {
	MIMES_NODE_T **table; /* Hashtable. */
	size_t hashLength, nItems; /* Length of hashtable and number of items effectively added to it. */
} MIMES_T;



MIMES_T *MIMES_New() {
	MIMES_T *aux;
	size_t i;

	/* Try to allocate structure. */
	aux = (MIMES_T *) malloc(sizeof(MIMES_T));
	if(aux == NULL) {
		return NULL;
	}

	/* Set default values. */
	aux->hashLength = MIMES_HASHTABLE_LENGTH;
	aux->nItems = 0;

	/* Try to allocate hashtable. */
	aux->table = (MIMES_NODE_T **) malloc(sizeof(MIMES_NODE_T *) * aux->hashLength);
	if(aux->table == NULL) {
		free(aux);
		return NULL;
	}

	/* Set hashtable default NULL value. */
	/*for(i = 0; i < aux->hashLength; i++) {
		aux->table[i] = NULL;
	}*/
	memset(aux->table, 0, sizeof(MIMES_NODE_T *) * aux->hashLength);
	return aux;
}

int _MIMES_StrCaseinCmp(char *a, char *b) {
	char auxA, auxB;
	while(*a != '\0' || *b != '\0') {
		if((auxA = tolower(*a)) != (auxB = tolower(*b))) {
			return auxA - auxB; /* string a don't equals string b */
		}
		a++, b++;
	}
	return 0; /* string a equals string b */
}
size_t _MIMES_HashKey(char *key) {
	size_t counter;
	for(counter = 0; *key != '\0'; key++) {
		counter += tolower(*key);
	}
	return counter;
}

int MIMES_AddMime(char *extension, char *name, int caseSensitive, MIMES_T *m) {
	MIMES_NODE_T *aux, **it;
	size_t i, len;
	if(extension == NULL || name == NULL || m == NULL) {
		return 0;
	}

	/* Ignores dot if it's present. */
	if(*extension == '.') {
		extension++;
	}

	/* Check if strings are not empty or larger than limits. */
	if((len = strlen(extension)) > MIMES_EXTENSION_LENGTH || *extension == '\0' || strlen(name) > MIMES_NAME_LENGTH || *name == '\0') {
		return 0;
	}

	/* Try to allocate structure. */
	aux = (MIMES_NODE_T *) malloc(sizeof(MIMES_NODE_T));
	if(aux == NULL) {
		return 0;
	}

	/* Set values. */
	aux->caseSensitive = (caseSensitive == 0) ? 1:0;
	strcpy(aux->extension, extension);
	strcpy(aux->name, name);

	/* Get hashtable position for this extension. */
	i = _MIMES_HashKey(extension) % m->hashLength;

	/* Add mime type to table. */
	it = &m->table[i];
	if(aux->caseSensitive == 0) { /* Case insensitive: add after all case sensitives. */
		while(*it != NULL) {
			if((*it)->caseSensitive == 0) {
				break;
			}
			it = &(*it)->next;
		}
	}
	aux->next = *it;
	*it = aux;
	m->nItems++;
	return 1;
}

char *MIMES_GetMimeForFile(char *fileName, MIMES_T *m) {
	MIMES_NODE_T *it;
	size_t i, len;
	char *key;
	if(fileName == NULL || m == NULL) {
		return NULL;
	}

	/* Check if 'fileName' is not empty. */
	if((len = strlen(fileName)) < 1) {
		return NULL;
	}

	/* Find beginning of file extension and check if 'fileName' really has extension.. */
	for(key = &fileName[len - 1]; key >= fileName && *key != '.' && *key != '/'; key--);
	if(key < fileName || *key != '.') {
		return NULL;
	}

	/* Check if extension is not empty or longer than maximum allowed. */
	key++;
	if((len = strlen(key)) < 1 || len > MIMES_EXTENSION_LENGTH) {
		return NULL;
	}

	/* Get hashtable position for this extension. */
	i = _MIMES_HashKey(key) % m->hashLength;

	/* Find Mime type for 'fileName'. */
	it = m->table[i];
	while(it != NULL) {
		if(it->caseSensitive != 0) { /* Case sensitive. */
			if(strcmp(it->extension, key) == 0) {
				return it->name;
			}
		} else { /* Case insensitive. */
			if(_MIMES_StrCaseinCmp(it->extension, key) == 0) {
				return it->name;
			}
		}
		it = it->next;
	}
	return NULL; /* Mime type not found in table. */
}

int MIMES_AddCommon(MIMES_T *m) {
	if(m == NULL) {
		return 0;
	}
	MIMES_AddMime(".html", "text/html", 0, m);
	MIMES_AddMime(".htm", "text/html", 0, m);
	MIMES_AddMime(".xhtml", "application/xhtml+xml", 0, m);
	MIMES_AddMime(".js", "text/javascript", 0, m);
	MIMES_AddMime(".css", "text/css", 0, m);
	MIMES_AddMime(".txt", "text/plain", 0, m);
	MIMES_AddMime(".md", "text/plain", 0, m);
	MIMES_AddMime(".jpg", "image/jpeg", 0, m);
	MIMES_AddMime(".jpeg", "image/jpeg", 0, m);
	MIMES_AddMime(".png", "image/png", 0, m);
	MIMES_AddMime(".gif", "image/gif", 0, m);
	MIMES_AddMime(".mp4", "video/mp4", 0, m);
	MIMES_AddMime(".ogg", "video/ogg", 0, m);
	MIMES_AddMime(".ogv", "video/ogg", 0, m);
	MIMES_AddMime(".ogx", "application/ogg", 0, m);
	MIMES_AddMime(".webm", "video/webm", 0, m);
	MIMES_AddMime(".pdf", "application/pdf", 0, m);
	MIMES_AddMime(".csv", "text/csv", 0, m);
	MIMES_AddMime(".json", "application/json", 0, m);
	MIMES_AddMime(".xml", "application/xml", 0, m);
	MIMES_AddMime(".mp3", "audio/mpeg", 0, m);
	MIMES_AddMime(".svg", "image/svg+xml", 0, m);
	MIMES_AddMime(".swf", "application/x-shockwave-flash", 0, m);
	return 1;
}

void MIMES_Destroy(MIMES_T *m) {
	MIMES_NODE_T *it, *aux;
	size_t i;
	if(m == NULL) {
		return;
	}
	for(i = 0; i < m->hashLength; i++) {
		it = m->table[i];
		while(it != NULL) {
			aux = it;
			it = it->next;
			free(aux);
		}
	}
	free(m->table);
	free(m);
}
