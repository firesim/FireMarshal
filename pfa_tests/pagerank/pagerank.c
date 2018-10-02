#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "pagerank.h"
#include "util.h"
#include "barrier.h"

#define COL_GROUP 512
#define ROW_GROUP 16
#define BLOCK_SIZE 128
#define MAX_ITERS 30

int pgsize, pgdoubles;
long start_page = 0, barrier_page = 0;
uint64_t *barrier_buf;
long barrier_paddr;

void matmul_group(double *mat, double *src_v, double *dst_v, int n)
{
	for (int bj = 0; bj < COL_GROUP; bj += BLOCK_SIZE) {
		for (int i = 0; i < ROW_GROUP; i+=4) {
			dotprod4(&dst_v[i],
				&mat[(i + 0) * n + bj],
				&mat[(i + 1) * n + bj],
				&mat[(i + 2) * n + bj],
				&mat[(i + 3) * n + bj],
				&src_v[bj], BLOCK_SIZE);
		}
	}
}

void matmul(double *mat, double *src_v, double *dst_v, int n)
{
	int nrows = n;
	int startrow = 0;
	int endrow = startrow + nrows;

	for (int i = startrow; i < endrow; i++)
		dst_v[i] = 0.0;

	for (int j = 0; j < n; j += COL_GROUP) {
		for (int i = startrow; i < endrow; i += ROW_GROUP) {
			matmul_group(&mat[i * n + j], &src_v[j], &dst_v[i], n);
		}
	}
}

static int pagerank(
	void *iomem, double *mat, double *v, unsigned long *vpaddr,
	int n, double err)
{
	double *last_v = v + n, *cur_v = v;
	unsigned long *last_vpaddr = vpaddr + n / pgdoubles;
	unsigned long *cur_vpaddr = vpaddr;
	int iters = 0;

	do {
		// Ping-pong the M-V multiplication results between arrays
		double *temp_v = cur_v;
		unsigned long *temp_vpaddr = cur_vpaddr;
		cur_v = last_v; cur_vpaddr = last_vpaddr;
		last_v = temp_v; last_vpaddr = temp_vpaddr;
		matmul(mat, last_v, cur_v, n);
		iters += 1;
	} while (!converged(cur_v, last_v, n, err) && iters < MAX_ITERS);

	// If the final result ends up in the buffer,
	// copy it over to the destination array
	if (cur_v != v) {
		for (int i = 0; i < n; i++)
			v[i] = cur_v[i];
	}

	return iters;
}

int main(int argc, char *argv[])
{
	struct timespec start, end;
	double *ranks, err = 1e-8;
	long nanos;
	unsigned long *rankpaddr;
	double *pr_matrix, *expected;
	int n = 512, iters, opt;
  FILE *matrix_file;
	void *fmem, *iomem = NULL;

	pgsize = sysconf(_SC_PAGESIZE);
	pgdoubles = pgsize / sizeof(double);

	while ((opt = getopt(argc, argv, "n:e:s:b:")) != -1) {
		switch (opt) {
		case 'n':
			n = atoi(optarg);
			break;
		case 'e':
			err = atof(optarg);
			break;
		case 's':
			start_page = atol(optarg);
			break;
		case 'b':
			barrier_page = atol(optarg);
			break;
		default:
			fprintf(stderr, "Unrecognized option -%c\n", opt);
			exit(EXIT_FAILURE);
		}
	}

	if (optind == argc) {
		fprintf(stderr, "Usage: %s [-n <num_nodes>] [-e <error>] data.bin\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	matrix_file = fopen(argv[optind], "r");
	if (matrix_file == NULL) {
		fprintf(stderr, "Could not open %s\n", argv[optind]);
		perror("open()");
		exit(EXIT_FAILURE);
	}

  fmem = malloc(n * (n + 1) * sizeof(double));
  if (fmem == NULL) {
    perror("Allocate matrix memory");
    exit(EXIT_FAILURE);
  }
  if(fread(fmem, sizeof(double), n*(n+1), matrix_file) != n*(n+1)) {
    perror("Read matrix file");
    fclose(matrix_file);
    exit(EXIT_FAILURE);
  }
  fclose(matrix_file);

	pr_matrix = (double *) fmem;
	expected  = (double *) (fmem + n * n * sizeof(double));

	ranks = mmap_alloc(2 * n * sizeof(double));
	if (ranks == MAP_FAILED) {
		perror("mmap() ranks");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 2 * n; i++)
		ranks[i] = 1.0 / n;

	// Fault the matrix into memory
	/* for (int i = 0; i < n * n; i += pgdoubles) { */
	/* 	if (mlock(&pr_matrix[i], pgsize)) { */
	/* 		fprintf(stderr, "Could not lock page %d\n", i / pgdoubles); */
	/* 		perror("mlock()"); */
	/* 		exit(EXIT_FAILURE); */
	/* 	} */
	/* 	munlock(&pr_matrix[i], pgsize); */
	/* } */

	mb();
	clock_gettime(CLOCK_MONOTONIC, &start);
	iters = pagerank(iomem, pr_matrix, ranks, rankpaddr, n, err);
	mb();
	clock_gettime(CLOCK_MONOTONIC, &end);
	nanos = timediff(&end, &start);

	if (!converged(ranks, expected, n, err)) {
		fprintf(stderr, "PageRank result does not match expected\n");
		exit(EXIT_FAILURE);
	}

	printf("N = %d\n", n);
	printf("iterations = %d\n", iters);
	printf("nanos = %ld\n", nanos);

	/* munmap(fmem, n * (n + 1) * sizeof(double)); */
  free(fmem);
	/* munmap(ranks, 2 * n * sizeof(double)); */
  free(ranks);

	return 0;
}
