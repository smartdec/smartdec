int main() {
	int i = 0;

	try {
		i++;
		throw "DRAMA!";
	} catch (const char *text) {
		i++;
	}

	return i;
}
