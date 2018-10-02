#ifndef __PAGERANK_H__
#define __PAGERANK_H__

#include "memblade.h"
#include <stdio.h>

#define MAX_INFLIGHT 16
extern int pgsize, pgdoubles, nprocs, procid;

static inline double dotprod(double *v1, double *v2, int n)
{
	double result = 0.0;

	for (int i = 0; i < n; i+=4) {
		double prod0, prod1, prod2, prod3;
		prod0 = v1[i] * v2[i];
		prod1 = v1[i+1] * v2[i+1];
		prod2 = v1[i+2] * v2[i+2];
		prod3 = v1[i+3] * v2[i+3];
		result += prod0 + prod1 + prod2 + prod3;
	}

	return result;
}

static inline void dotprod4(
		double *result,
		double *va0,
		double *va1,
		double *va2,
		double *va3,
		double *vb,
		int n)
{
	double p0 = 0.0, p1 = 0.0, p2 = 0.0, p3 = 0.0;

	for (int i = 0; i < n; i++) {
		double b = vb[i];
		p0 += va0[i] * b;
		p1 += va1[i] * b;
		p2 += va2[i] * b;
		p3 += va3[i] * b;
	}

	result[0] += p0;
	result[1] += p1;
	result[2] += p2;
	result[3] += p3;
}

static inline void dotprod8(
		double *result,
		double *va0,
		double *va1,
		double *va2,
		double *va3,
		double *va4,
		double *va5,
		double *va6,
		double *va7,
		double *vb,
		int n)
{
	double p0 = 0.0, p1 = 0.0, p2 = 0.0, p3 = 0.0;
	double p4 = 0.0, p5 = 0.0, p6 = 0.0, p7 = 0.0;

	for (int i = 0; i < n; i++) {
		double b = vb[i];
		p0 += va0[i] * b;
		p1 += va1[i] * b;
		p2 += va2[i] * b;
		p3 += va3[i] * b;
		p4 += va4[i] * b;
		p5 += va5[i] * b;
		p6 += va6[i] * b;
		p7 += va7[i] * b;
	}

	result[0] += p0;
	result[1] += p1;
	result[2] += p2;
	result[3] += p3;
	result[4] += p4;
	result[5] += p5;
	result[6] += p6;
	result[7] += p7;
}

static int converged(double *a, double *b, int n, double err)
{
	double norm2 = 0.0;

	for (int i = 0; i < n; i++) {
		double diff = a[i] - b[i];
		norm2 += diff * diff;
	}

	return norm2 < (err * err);
}

static inline void print_vec(double *v, int n)
{
	for (int i = 0; i < n; i++)
		printf("%f\n", v[i]);
}

static inline double sum_vec(double *v, int n)
{
	double res = 0.0;

	for (int i = 0; i < n; i++)
		res += v[i];

	return res;
}

// Pull results produced by other processes
static inline void push_result(void *iomem, long startpage,
		double *v, unsigned long *vpaddr, int n)
{
	int pnrows = (n / nprocs);
	int pstartrow = pnrows * procid;
	int pendrow = pstartrow + pnrows;
	int pstartpage = pstartrow / pgdoubles;
	int pendpage = pendrow / pgdoubles;
	int inflight = 0, wmax_inflight = MAX_INFLIGHT / nprocs;

	for (int i = pstartpage; i < pendpage; i++) {
		if (inflight == (wmax_inflight - 1)) {
			rmem_wait_resp(iomem, 1);
			rmem_get_resp(iomem);
			inflight--;
		}

		rmem_wait_req(iomem, 1);
		rmem_write_issue(iomem, vpaddr[i], startpage + i);
		inflight++;
	}

	rmem_wait_resp(iomem, inflight);
	rmem_complete(iomem, inflight);
}

static inline void pull_result(void *iomem, long startpage,
		double *v, unsigned long *vpaddr, int n)
{
	int npages = n / pgdoubles;
	int pnrows = (n / nprocs);
	int pstartrow = pnrows * procid;
	int pendrow = pstartrow + pnrows;
	int pstartpage = pstartrow / pgdoubles;
	int pendpage = pendrow / pgdoubles;
	int inflight = 0;

	for (int i = 0; i < pstartpage; i++) {
		if (inflight == (MAX_INFLIGHT - 1)) {
			rmem_wait_resp(iomem, 1);
			rmem_get_resp(iomem);
			inflight--;
		}

		rmem_wait_req(iomem, 1);
		rmem_read_issue(iomem, vpaddr[i], startpage + i);
		inflight++;
	}

	for (int i = pendpage; i < npages; i++) {
		if (inflight == (MAX_INFLIGHT - 1)) {
			rmem_wait_resp(iomem, 1);
			rmem_get_resp(iomem);
			inflight--;
		}

		rmem_wait_req(iomem, 1);
		rmem_read_issue(iomem, vpaddr[i], startpage + i);
		inflight++;
	}

	rmem_wait_resp(iomem, inflight);
	rmem_complete(iomem, inflight);
}

#endif
