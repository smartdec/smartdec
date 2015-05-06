/* gcc -nostdlib -o ../038_compound_condition 038_compound_condition.c */

int g() {}
int h() {}

void f(int a, int b, int c, int d) {
	if ((a && b) || (c && d)) {
		g();
	} else {
		h();
	}
	if ((a || b) && (c || d)) {
		g();
	}
	if (a || (b && c) || d) {
		h();
	}
}
