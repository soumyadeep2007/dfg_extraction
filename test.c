#include <stdio.h>

void funca(int);
void funcb(int);

int main() {
	int a = 6;
	funca(5);
	int b = 7;
	funcb(2);
	return 0;
}

void funca(int p1) {
	int a = 6;
	funcb(p1);
	int b = 7;
}

void funcb(int p1) {
	int d = 10;
}