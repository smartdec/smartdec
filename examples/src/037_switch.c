/* g++ -nostdlib -o ../037_switch 037_switch.c */

int vowels(const char *s) {
	int result = 0;

	while (*s) {
		switch (*s) {
			case 'a':
			case 'e':
			case 'i':
			case 'o':
			case 'r':
			case 'u':
			case 'y':
				++result;
				break;
			case 0:
				return -1;
			default:
				break;
		}
		++s;
	}

	return result;
}
