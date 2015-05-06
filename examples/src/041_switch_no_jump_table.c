/* g++ -nostdlib -m32 -o ../041_switch_no_jump_table 041_switch_no_jump_table.c */

const char *do_switch(int x) {
	switch (x) {
	case 0:
		return "zero";
	case 1:
		return "one";
	case 2:
		return "two";
	case 3:
		return "three";
	case 4:
		return "four";
	case 5:
		return "five";
	case 6:
		return "six";
	case 7:
		return "seven";
	case 8:
		return "eight";
	case 9:
		return "nine";
	case 10:
		return "ten";
	case 100500:
		return "stopyatsot";
	default:
		return "many";
	};
}
