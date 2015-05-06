/* gcc -nostdlib -o ../014_calls 014_calls.c */

int h() { return 1; }

int g(int a, int b, int c, int d) {
	return a + b + c + d;
}

int f() {
	int some_value = 20;
	if (h()) {
		some_value += g(1, 2, 3, 4);
	}
	some_value += 10;
	return g(some_value, some_value, some_value, some_value);
}
