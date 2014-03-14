/* g++ -c -o ../056_call_struct.o 056_call_struct.cpp */

struct A {
	int a;
	short b;
	char c;
};

struct B {
	int q;
	int w;
	int e;
	int r;
};

void f(A a, B b) {
}

void g() {
	f(A(), B());
}
