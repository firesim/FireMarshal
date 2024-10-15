#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define PAGE_SIZE (4*1024)

#define DRAM_BASE ((uint64_t)0x80000000L)
#define DRAM_SIZE (((uint64_t)1) << 30)
#define SHARED_MEM_SIZE (PAGE_SIZE)
#define SHARED_MEM_LOCATION ((DRAM_BASE + DRAM_SIZE) - SHARED_MEM_SIZE)
#define DEBUG

#define MAX_LEN 5
typedef struct scheduler {
	int len;
	char buf[MAX_LEN];
} scheduler_t;
scheduler_t* sptr = NULL;
sem_t* sem = NULL;

void init_scheduler(void) {
	if (sem == NULL) {
		printf("creating semaphore\n");
		sem = sem_open("/scheduler_sem", O_CREAT, 0644, 1); // by default shared between processes
	}

	if (sptr == NULL) {
		printf("setting up shared memory in separate process\n");

		// only 1 thread can setup shared memory at a time
		sem_wait(sem);

		int fd;
		fd = shm_open("/scheduler", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // TODO: unsure what S_... flags are
		if (fd == -1) {
			printf("failed to run shm_open\n");
			exit(-1);
		}

		if (ftruncate(fd, sizeof(scheduler_t)) == -1) {
			printf("failed to ftruncate\n");
			exit(-1);
		}

		sptr = mmap(NULL, sizeof(scheduler_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (sptr == MAP_FAILED) {
			printf("failed to mmap\n");
			exit(-1);
		}

		sptr->len = 0;

		sem_post(sem);
	}

	printf("done initializing\n");
}

void schedule_something(void) {
	if (sptr == NULL || sem == NULL) {
		printf("something went wrong\n");
		exit(-1);
	}

	sem_wait(sem);
	sptr->buf[sptr->len] = 'A';
	++(sptr->len);
	sem_post(sem);
}

void wait_until_scheduled(void) {
	if (sptr == NULL || sem == NULL) {
		printf("something went wrong\n");
		exit(-1);
	}

	sem_wait(sem);
	while (1) {
		if (sptr->len == 1) {
			printf("%c obtained\n", sptr->buf[sptr->len - 1]);
			sem_post(sem);
			break;
		} else {
			sem_post(sem);
			sleep(1);
		}
	}
}

// void *mapmem(uint64_t base, uint64_t size)
// {
//    int mem_fd;
//    uint64_t offset = base % PAGE_SIZE;
//    base = base - offset;
//    size = size + offset;
//    /* open /dev/mem */
//    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
//       printf("can't open /dev/mem\nThis program should be run as root. Try prefixing command with: sudo\n");
//       exit (-1);
//    }
//    void *mem = mmap(
//       0,
//       size,
//       PROT_READ|PROT_WRITE,
//       MAP_SHARED,
//       mem_fd,
//       base);
// #ifdef DEBUG
//    printf("base=0x%x, mem=%p\n", base, mem);
// #endif
//    if (mem == MAP_FAILED) {
//       printf("mmap error %" PRIdPTR "\n", (intptr_t)mem);
//       exit (-1);
//    }
//    close(mem_fd);
//    return (char *)mem + offset;
// }
//
// void unmapmem(void *addr, uint64_t size)
// {
//    const intptr_t offset = (intptr_t)addr % PAGE_SIZE;
//    addr = (char *)addr - offset;
//    size = size + offset;
//    int s = munmap(addr, size);
//    if (s != 0) {
//       printf("munmap error %d\n", s);
//       exit (-1);
//    }
// }

#endif
