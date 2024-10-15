#include <stdio.h>

#include <mem.h>

int main(void) {
	printf("Goodbye started\n");
	init_scheduler();
	wait_until_scheduled();
	printf("Something was waited for!\n");
	sleep(5);
	return 0;
}
