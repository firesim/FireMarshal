#include <stdio.h>

#include <mem.h>

int main(void) {
	printf("Hello started\n");
	init_scheduler();
	schedule_something();
	printf("Something was scheduled!\n");
	sleep(5);
	return 0;
}