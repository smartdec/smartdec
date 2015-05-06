/* gcc -nostdlib -m32 -o ../003_gcd_m32 003_gcd_m32.c */

int gcd(int a, int b) {
	if (b == 0) {
		return a;
	} else {
		return gcd(b, a % b);
	}
}
