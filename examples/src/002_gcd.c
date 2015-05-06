/* gcc -nostdlib -o ../002_gcd 002_gcd.c */

int gcd(int a, int b) {
	if (b == 0) {
		return a;
	} else {
		return gcd(b, a % b);
	}
}
