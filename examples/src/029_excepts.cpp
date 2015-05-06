// excepts.cpp : Defines the entry point for the console application.
//
#include <stdio.h>

void do_fact(int arg, int in)
{
	if (arg <= 0) throw in;
	do_fact(arg - 1, arg * in);
}

int fact(int arg)
{
	try {
		do_fact(arg, 1);
	} catch (int res) {
		return res;
	}
	return 1;
}

int main()
{
	int val;
	scanf("%d", &val);
	val = fact(val);
	printf("%d\n", val);
	return 0;
}