/* gcc -nostdlib -o ../040_loop_nests 040_loop_nests.c */

int g() {}
int h() {}

void f(int a, int b, int c, int d) {
	while (a) {
		while (h()) {
			++b;
			if (b == c) {
				return;
			}
		}
		++b;
	}


	while (a + b - c) {
		while (h()) {
			do {
				if (a) {
					d = c;
				}
			} while (g() && h());
			--a;
		}

		while (g() && h()) {
			++a;
			if (!d) {
				break;
			}
			if (a) {
				continue;
			}
			d = 20;
		}
	}
}
