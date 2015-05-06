/* gcc -nostdlib -std=gnu99 -o ../005_for 005_for.c */

void puts(const char *something) {}

int main() {
	for (int i = 0; i < 10; ++i) {
		puts("Hello!\n");
	}
	return 0;
}
