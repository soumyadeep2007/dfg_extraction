#include <stdio.h>

void f1(int);
void f2(int);

int main() {
	f1(5);
	return 0;
}

void f1(int p1) {
	f2(p1);
}

void f2(int p1) {
	printf("%d", p1);
}