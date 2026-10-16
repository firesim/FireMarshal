// prefetch-smoke: Linux userspace smoke test for the BOOM ld-x0 software
// prefetch (WithSoftwarePrefetchRoCC). Run this before heavier SPEC workloads
// to validate the prefetch path under a real kernel: demand paging, trap
// boundaries (the drain window that used to reach MMIO), hostile addresses,
// and an mcf-style pointer chase.
//
// Any SIGSEGV/SIGBUS/SIGILL is caught and reported with the faulting PC and
// address: a fault whose PC is not a load (or that lands mid-phase where no
// wild access exists) evidences prefetch fault misattribution. Exit 0 + the
// PREFETCH-SMOKE PASS line means the machine survived everything.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ucontext.h>

#if defined(__riscv)
#define SW_PREFETCH(addr) \
  __asm__ __volatile__("ld x0, 0(%0)" ::"r"(addr) : "memory")
#else
#define SW_PREFETCH(addr) ((void)(addr))
#endif

#define PAGE   4096UL
#define BUFSZ  (8UL << 20) // 8 MiB, larger than any cache level
#define NPAGES (BUFSZ / PAGE)

static volatile const char *current_phase = "startup";

static void fault_handler(int sig, siginfo_t *si, void *ucv)
{
  uintptr_t pc = 0;
#if defined(__riscv)
  ucontext_t *uc = (ucontext_t *)ucv;
  pc = uc->uc_mcontext.__gregs[0]; // REG_PC
#endif
  printf("FATAL: signal %d in phase '%s': si_addr=%p pc=0x%lx\n", sig,
         current_phase, si->si_addr, (unsigned long)pc);
  printf("(a fault here likely means prefetch exceptions are escaping)\n");
  fflush(stdout);
  _exit(1);
}

static uint64_t now_us(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000ull + ts.tv_nsec / 1000;
}

static uint64_t pat(size_t page, size_t off)
{
  return 0x5AA5000000000000ull ^ ((uint64_t)page << 16) ^ (uint64_t)off;
}

static volatile uint64_t *bufp(volatile uint8_t *b, size_t page, size_t off)
{
  return (volatile uint64_t *)&b[page * PAGE + off];
}

static void check(volatile uint8_t *b, size_t page, size_t off, int code)
{
  uint64_t v = *bufp(b, page, off);
  if (v != pat(page, off)) {
    printf("MISMATCH page %zu off %zu: got 0x%llx want 0x%llx (code %d)\n",
           page, off, (unsigned long long)v, (unsigned long long)pat(page, off),
           code);
    exit(1);
  }
}

// mcf-style pointer chase over a permuted ring, PF_DIST-ahead prefetch
typedef struct node {
  struct node *next;
  uint64_t val;
  uint64_t pad[6]; // one cache line per node
} node_t;

static uint64_t chase(node_t *start, size_t steps, int prefetch)
{
  uint64_t sum = 0;
  node_t *p = start;
  for (size_t i = 0; i < steps; i++) {
    if (prefetch && p->next && p->next->next)
      SW_PREFETCH(p->next->next); // PF_DIST = 2 pointers ahead
    sum += p->val;
    p = p->next;
  }
  return sum;
}

int main(void)
{
  struct sigaction sa = {0};
  sa.sa_sigaction = fault_handler;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
  sigaction(SIGILL, &sa, NULL);

  printf("prefetch-smoke: ld-x0 software prefetch under Linux\n");

  volatile uint8_t *buf = mmap(NULL, BUFSZ, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (buf == MAP_FAILED) {
    perror("mmap");
    return 1;
  }

  // ---- phase 1: prefetch resident pages -----------------------------------
  current_phase = "1-resident";
  printf("phase 1: resident pages\n");
  for (size_t p = 0; p < NPAGES; p++) {
    *bufp(buf, p, 0) = pat(p, 0);
    *bufp(buf, p, 2048) = pat(p, 2048);
  }
  for (size_t p = 0; p < 256; p++)
    SW_PREFETCH(bufp(buf, p, 2048));
  for (size_t p = 0; p < 256; p++)
    check(buf, p, 2048, 10);
  printf("phase 1 OK\n");

  // ---- phase 2: prefetch demand-paged (untouched) pages -------------------
  current_phase = "2-demand-paged";
  printf("phase 2: not-yet-faulted mmap pages (prefetch must be dropped)\n");
  volatile uint8_t *fresh = mmap(NULL, 64 * PAGE, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (fresh == MAP_FAILED) {
    perror("mmap fresh");
    return 1;
  }
  for (size_t p = 0; p < 64; p++)
    SW_PREFETCH(bufp(fresh, p, 0)); // PTE invalid: swallowed, no fault-in
  for (size_t p = 0; p < 64; p++) { // now demand-fault them for real
    *bufp(fresh, p, 0) = pat(p, 0);
    check(fresh, p, 0, 20);
  }
  printf("phase 2 OK\n");

  // ---- phase 3: prefetch bursts across trap boundaries --------------------
  // Queued prefetches drain during kernel/OpenSBI windows (this is the path
  // that turned heap vaddrs into MMIO writes before the LSU/IOMSHR fixes).
  current_phase = "3-trap-drain";
  printf("phase 3: prefetch bursts across %d syscall boundaries\n", 20000);
  for (int it = 0; it < 20000; it++) {
    size_t base = ((size_t)it * 8) % NPAGES;
    size_t off = 64 * ((it % 61) + 1); // rotate lines so they stay cold
    for (size_t k = 0; k < 8; k++)
      SW_PREFETCH(bufp(buf, (base + k) % NPAGES, off));
    (void)getpid(); // trap now, with the queue likely non-empty
    for (size_t k = 0; k < 4; k++)
      check(buf, (base + k) % NPAGES, 2048, 30);
    if (it % 5000 == 4999)
      printf("  iter %d OK\n", it + 1);
  }
  printf("phase 3 OK\n");

  // ---- phase 4: hostile prefetch targets ----------------------------------
  current_phase = "4-hostile";
  printf("phase 4: hostile prefetches (NULL/unmapped/non-canonical/kernel)\n");
  SW_PREFETCH((void *)0);                    // NULL
  volatile uint8_t *hole = mmap(NULL, 4 * PAGE, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  munmap((void *)hole, 4 * PAGE);
  SW_PREFETCH(hole);                         // just-unmapped page
  SW_PREFETCH((void *)0x4000000000ull);      // beyond sv39 user canonical
  SW_PREFETCH((void *)0xffffffc000000000ull); // kernel half
  SW_PREFETCH((void *)0x2004000ull);         // numerically CLINT mtimecmp
  (void)getpid();                            // flush queue across a trap
  check(buf, 1, 2048, 40);                   // still alive and coherent
  printf("phase 4 OK\n");

  // ---- phase 5: mcf-style pointer chase, timed ----------------------------
  current_phase = "5-pointer-chase";
  size_t nnodes = 1 << 15;
  node_t *nodes = calloc(nnodes, sizeof(node_t));
  // permuted ring (LCG step coprime with nnodes)
  size_t idx = 0, step = 40503;
  for (size_t i = 0; i < nnodes; i++) {
    size_t nxt = (idx + step) % nnodes;
    nodes[idx].next = &nodes[nxt];
    nodes[idx].val = idx;
    idx = nxt;
  }
  size_t steps = 4 * nnodes;
  uint64_t t0 = now_us();
  uint64_t s1 = chase(&nodes[0], steps, 0);
  uint64_t t1 = now_us();
  uint64_t s2 = chase(&nodes[0], steps, 1);
  uint64_t t2 = now_us();
  if (s1 != s2) {
    printf("MISMATCH: chase sums differ (0x%llx vs 0x%llx)\n",
           (unsigned long long)s1, (unsigned long long)s2);
    return 1;
  }
  printf("phase 5 OK: chase %zu steps: no-pf %llu us, pf %llu us\n", steps,
         (unsigned long long)(t1 - t0), (unsigned long long)(t2 - t1));

  printf("PREFETCH-SMOKE PASS\n");
  return 0;
}
