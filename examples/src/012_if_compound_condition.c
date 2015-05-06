/* gcc -nostdlib -o ../012_if_compound_condition 012_if_compound_condition.c */

void g() {}

void h() {}

void f(int a, int b, int c) {
	if (a && b && c) {
		g();
	} else {
		h();
	}
}
