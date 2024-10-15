#include <stdio.h>

int main(void) {
	__asm__("addi x0, x1, 0");
	printf("Hello world!\n");
	return 0;
}
