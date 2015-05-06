/* gcc -o ../019_struct 019_struct.c */

struct s1 {
	int a;
	int b;
};

void f(struct s1 *s) {
	s->a = 10;
	s->b = 20;
}
