class AAAAA {
public:
	virtual int f() = 0;
};

class BBBBB: public AAAAA {
public:
	virtual int f() {
		return 0;
	}
};

int main() {
	return (new BBBBB())->f();
}