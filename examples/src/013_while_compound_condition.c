/* gcc -nostdlib -o ../013_while_compound_condition 013_while_compound_condition.c */

void g(int *a, int *b, int *c) {
	++*a;
	--*b;
	*c = *a**b;
}

void f(int a, int b, int c) {
	while (a && b && c) {
		g(&a, &b, &c);
	}
}
