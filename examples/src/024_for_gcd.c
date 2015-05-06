/* gcc -std=gnu99 -o ../024_for_gcd 024_for_gcd.c */

int gcd(int a, int b) {
	if (b) {
		return gcd(b, a % b);
	} else {
		return a;
	}
}

int main() {
	for (int i = 0; i < 10; ++i) {
		gcd(i, 2 * i + 1);
	}
}
