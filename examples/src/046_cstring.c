/* gcc -O2 -m32 -std=c99 -o ../046_cstring 046_cstring.c */

#include <stdio.h>
#include <string.h>

int main() {
	const char *s = "Hello, World!";
	size_t len = strlen(s);

	for (int i = 0; i < len; ++i) {
		printf("%c", s[i]);
	}
	printf("\n");
	return 0;
}
