


#ifndef MIMES_H_
#define MIMES_H_
#include <stdlib.h>



	typedef struct __mimes_t MIMES_T;



	MIMES_T *MIMES_New(); /* Creates a new empty MIME Types Table. */

	int MIMES_AddMime(char *extension, char *name, int caseSensitive, MIMES_T *m); /* Adds extension 'extension' as Mime Type 'name' to table 'm'. If extension is case sensitive, 'caseSensitive' must be != 0, otherwise == 0. */
	char *MIMES_GetMimeForFile(char *fileName, MIMES_T *m); /* Get Mime Type for file 'fileName', using table 'm' as reference. */
	int MIMES_AddCommon(MIMES_T *m); /* Populates table 'm' with the most common used and known Mime Types. */

	void MIMES_Destroy(MIMES_T *m); /* Destroy memory resources used by table 'm'. */



#endif


