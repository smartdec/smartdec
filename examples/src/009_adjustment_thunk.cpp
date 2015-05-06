class Z {
public:
	Z(): z(10) {}
	int z;
	
	virtual ~Z() {}
};

class A {
public:
	virtual int *f() {
		return new int[10];
	}

	int a;
	
	virtual ~A() {}
};

class B {
public:
	virtual int *g() {
		return new int[20];
	}
	
	int b;
	
	virtual ~B() {}
};

class C: public Z, public A, public B {
public:
	virtual int *f() {
		return &z;
	}

	virtual int *g() {
		return &z;
	}

};


int main() {

	C *c = new C();
	c->g();

	return 0;
}