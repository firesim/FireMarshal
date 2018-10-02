#ifndef __MEMBLADE_H__
#define __MEMBLADE_H__

#include <stdint.h>
#include <sys/ioctl.h>

#define MB_REQ_ETH_TYPE 0x0408
#define MB_RESP_ETH_TYPE 0x0508
#define MB_DRAFT_VERSION 1

#define MB_OC_PAGE_READ 0
#define MB_OC_PAGE_WRITE 1
#define MB_OC_WORD_READ 2
#define MB_OC_WORD_WRITE 3
#define MB_OC_ATOMIC_ADD 4
#define MB_OC_COMP_SWAP 5

#define MB_RC_PAGE_OK 0x80
#define MB_RC_NODATA_OK 0x81
#define MB_RC_WORD_OK 0x82
#define MB_RC_ERROR 0x83

#define RMEM_CLIENT_SRC_ADDR 0x00
#define RMEM_CLIENT_DST_ADDR 0x08
#define RMEM_CLIENT_DSTMAC   0x10
#define RMEM_CLIENT_OPCODE   0x16
#define RMEM_CLIENT_PAGENO   0x18
#define RMEM_CLIENT_REQ      0x20
#define RMEM_CLIENT_RESP     0x24
#define RMEM_CLIENT_NREQ     0x28
#define RMEM_CLIENT_NRESP    0x2C

static inline uint64_t memblade_make_exthead(int offset, int size)
{
	return ((offset & 0xfff) << 4) | (size & 0x3);
}

static inline uint32_t rmem_nreq(void *iomem)
{
	volatile uint32_t *ptr = iomem + RMEM_CLIENT_NREQ;
	return *ptr;
}

static inline uint32_t rmem_nresp(void *iomem)
{
	volatile uint32_t *ptr = iomem + RMEM_CLIENT_NRESP;
	return *ptr;
}

static inline void rmem_set_src_addr(void *iomem, uint64_t addr)
{
	volatile uint64_t *ptr = iomem + RMEM_CLIENT_SRC_ADDR;
	*ptr = addr;
}

static inline void rmem_set_dst_addr(void *iomem, uint64_t addr)
{
	volatile uint64_t *ptr = iomem + RMEM_CLIENT_DST_ADDR;
	*ptr = addr;
}

static inline void rmem_set_opcode(void *iomem, uint8_t opcode)
{
	volatile uint8_t *ptr = iomem + RMEM_CLIENT_OPCODE;
	*ptr = opcode;
}

static inline void rmem_set_dstmac(void *iomem, uint64_t dstmac)
{
	volatile uint64_t *ptr = iomem + RMEM_CLIENT_DSTMAC;
	*ptr = dstmac;
}

static inline void rmem_set_pageno(void *iomem, uint64_t pageno)
{
	volatile uint64_t *ptr = iomem + RMEM_CLIENT_PAGENO;
	*ptr = pageno;
}

static inline uint32_t rmem_send_req(void *iomem)
{
	volatile uint32_t *ptr = iomem + RMEM_CLIENT_REQ;
	return *ptr;
}

static inline uint32_t rmem_get_resp(void *iomem)
{
	volatile uint32_t *ptr = iomem + RMEM_CLIENT_RESP;
	return *ptr;
}

static inline long get_pfn(int fd, void *vaddr)
{
	return ioctl(fd, 0, (unsigned long) vaddr);
}

static inline void rmem_send_sync(void *iomem)
{
	while (rmem_nreq(iomem) == 0) {}
	rmem_send_req(iomem);

	while (rmem_nresp(iomem) == 0) {}
	rmem_get_resp(iomem);
}

static inline void rmem_wait_req(void *iomem, int n)
{
	while (rmem_nreq(iomem) < n) {}
}

static inline void rmem_wait_resp(void *iomem, int n)
{
	while (rmem_nresp(iomem) < n) {}
}

static inline void rmem_read_issue(
		void *iomem, unsigned long dst_addr, long pageno)
{
	rmem_set_dst_addr(iomem, dst_addr);
	rmem_set_opcode(iomem, MB_OC_PAGE_READ);
	rmem_set_pageno(iomem, pageno);
	rmem_send_req(iomem);
}

static inline void rmem_write_issue(
		void *iomem, unsigned long src_addr, long pageno)
{
	rmem_set_src_addr(iomem, src_addr);
	rmem_set_opcode(iomem, MB_OC_PAGE_WRITE);
	rmem_set_pageno(iomem, pageno);
	rmem_send_req(iomem);
}

static inline void rmem_complete(void *iomem, int n)
{
	for (int i = 0; i < n; i++)
		rmem_get_resp(iomem);
}

#endif
