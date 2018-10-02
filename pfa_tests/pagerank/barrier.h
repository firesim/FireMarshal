#ifndef __BARRIER_H__
#define __BARRIER_H__

#include "memblade.h"
#include "util.h"

void init_barrier(void *iomem, uint64_t *src, uint64_t src_paddr, long barrier_page)
{
	rmem_set_src_addr(iomem, src_paddr);
	rmem_set_pageno(iomem, barrier_page);
	rmem_set_opcode(iomem, MB_OC_WORD_WRITE);

	src[0] = memblade_make_exthead(0, 3);
	src[1] = 0;
	mb();
	rmem_send_sync(iomem);

	src[0] = memblade_make_exthead(8, 3);
	src[1] = 0xDEADBEEF;
	mb();
	rmem_send_sync(iomem);
}

void destroy_barrier(void *iomem, uint64_t *src, uint64_t src_paddr, long barrier_page)
{
	rmem_set_src_addr(iomem, src_paddr);
	rmem_set_pageno(iomem, barrier_page);
	rmem_set_opcode(iomem, MB_OC_WORD_WRITE);

	src[0] = memblade_make_exthead(8, 3);
	src[1] = 0;
	mb();
	rmem_send_sync(iomem);
}

void wait_barrier_init(void *iomem, uint64_t *src, uint64_t src_paddr,
		uint64_t *dst, uint64_t dst_paddr, long barrier_page)
{
	rmem_set_src_addr(iomem, src_paddr);
	rmem_set_dst_addr(iomem, dst_paddr);
	rmem_set_pageno(iomem, barrier_page);
	rmem_set_opcode(iomem, MB_OC_WORD_READ);

	src[0] = memblade_make_exthead(8, 3);
	mb();
	do {
		rmem_send_sync(iomem);
		mb();
	} while (dst[0] != 0xDEADBEEF);
}

void barrier(void *iomem, int nprocs,
		uint64_t *src, uint64_t src_paddr,
		uint64_t *dst, uint64_t dst_paddr,
		long barrier_page)
{
	static int barrier_run = 1;
	uint64_t final = barrier_run * nprocs;

	rmem_set_src_addr(iomem, src_paddr);
	rmem_set_dst_addr(iomem, dst_paddr);
	rmem_set_pageno(iomem, barrier_page);

	rmem_set_opcode(iomem, MB_OC_ATOMIC_ADD);
	src[0] = memblade_make_exthead(0, 3);
	src[1] = 1;
	mb();
	rmem_send_sync(iomem);

	rmem_set_opcode(iomem, MB_OC_WORD_READ);

	do {
		dst[0] = 0;
		rmem_send_sync(iomem);
		mb();
	} while (dst[0] < final);

	barrier_run++;
}

#endif
