/* g++ -o ../047_int2bool 047_int2bool.cpp */

int f() { return 10; }

int g(bool x) {
}

int main() {
	g(f());
	return 0;
}
