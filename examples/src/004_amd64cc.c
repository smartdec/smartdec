/* gcc -nostdlib -o ../004_amd64cc 004_amd64cc.c */

int f(int a, short b, char c, int d, short e, char f, int g, short h, char i) {}

int main() {
	f(1, 2, 3, 4, 5, 6, 7, 8, 9);
	
	return 0;
}

