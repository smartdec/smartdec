/* gcc -m32 -std=c99 -o ../051_misaligned_arg 051_misaligned_arg.c */

int g;

void f(int i) {
	g = i;
	*(int *)((char *)&g + 1) = *(int *)((char *)&i + 1);
}

int main() {
	for (int i = 0; i < 10; ++i) {
		f(i);
	}
	return 0;
}
