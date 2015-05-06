class AAAAA {
public:
	__declspec(noinline) AAAAA() {
		v = 100;
	}

	virtual int f() { return 1; }
	
protected:
	int v;
};

class BBBBB: public AAAAA {
public:
	__declspec(noinline) BBBBB() {
		v++;
	}
	
	virtual int f() { return v; }
};

int main(int argc, char **argv) {
	return (new BBBBB())->f() + (new AAAAA())->f() + (new BBBBB())->f() + (new AAAAA())->f();
}