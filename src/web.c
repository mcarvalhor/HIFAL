#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>



void Web_UrlDecode(char *str) {
	char *resultingStr;
	int aux;
	if(str == NULL) {
		return;
	}
	resultingStr = str;
	for(; *str != '\0'; str++, resultingStr++) {
		if(*str == '+') {
			*resultingStr = ' ';
		} else if(*str == '%' && isxdigit(*(str + 1)) != 0 && isxdigit(*(str + 2)) != 0) {
			aux = -1;
			sscanf(str + 1, "%2x", &aux);
			if(aux >= 0 && aux <= 255) {
				*resultingStr = aux;
			}
			str += 2;
		/*} else if(*str == '%' && isxdigit(*(str + 1)) != 0 && isxdigit(*(str + 2)) == 0) {
			aux = -1;
			sscanf(str + 1, "%1x", &aux);
			if(aux >= 0 && aux <= 255) {
				*resultingStr = aux;
			}
			str += 1; */
		} else {
			*resultingStr = *str;
		}
	}
	*resultingStr = '\0';
}

int Web_UrlDecodeTo(char **dest, char *src, size_t length) {
	// TODO
}

void Web_UrlEncode(char *str) {
	// TODO
}

int Web_UrlEncodeTo(char **dest, char *src, size_t length) {
	// TODO
}

int Web_HtmlConvertTo(char **dest, char *src, size_t length) {
	/* TODO
	* &		-> &amp;
	* "		-> &quot;
	* '		-> &apos;
	* <		-> &lt;
	* >		-> &gt;
	*/
}


