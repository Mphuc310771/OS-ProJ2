#ifndef types_definitions
#define types_definitions
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long uint64;
#endif

typedef uint64 pte_t;
typedef uint64 *pagetable_t; // 512 PTEs

#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "kernel/stat.h"
#include "user/user.h"

#ifndef PGSIZE
#define PGSIZE 4096
#endif

// Macros if missing
#ifndef PTE_V
#define PTE_V (1L << 0)
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4)
#endif

#ifndef PGROUNDUP
#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#endif

#ifndef PTE_FLAGS
#define PTE_FLAGS(pte) ((pte) & 0x3FF)
#endif

#ifndef PTE2PA
#define PTE2PA(pte) (((pte) >> 10) << 12)
#endif

#ifndef SUPERPGROUNDUP
#define SUPERPGSIZE (2 * (1 << 20))
#define SUPERPGROUNDUP(sz) (((sz) + SUPERPGSIZE - 1) & ~(SUPERPGSIZE - 1))
#endif

#define N (8 * (1 << 20))

void print_pgtbl();
void print_kpgtbl();
void ugetpid_test();
void superpg_test();
void pgaccess_test();

char *testname = "???";

void err(char *why) {
  printf("pgtbltest: %s failed: %s, pid=%d\n", testname, why, getpid());
  exit(1);
}

void pgaccess_test() {
  char *buf;
  unsigned int abits;
  printf("pgaccess_test starting\n");
  testname = "pgaccess_test";
  buf = malloc(32 * PGSIZE);
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
  buf[PGSIZE] = 1;
  buf[2 * PGSIZE] = 2;
  buf[30 * PGSIZE] = 30;
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
  if (abits != ((1 << 1) | (1 << 2) | (1 << 30)))
    err("incorrect access bits set");
  free(buf);
  printf("pgaccess_test: OK\n");
}

void print_pte(uint64 va) {
  pte_t pte = (pte_t)pgpte((void *)va);
  printf("va 0x%lx pte 0x%lx pa 0x%lx perm 0x%lx\n", (uint64)va, (uint64)pte,
         (uint64)PTE2PA(pte), (uint64)PTE_FLAGS(pte));
}

void print_pgtbl() {
  printf("print_pgtbl starting\n");
  for (uint64 i = 0; i < 10; i++) {
    print_pte(i * PGSIZE);
  }
  uint64 top = MAXVA / PGSIZE;
  for (uint64 i = top - 10; i < top; i++) {
    print_pte(i * PGSIZE);
  }
  printf("print_pgtbl: OK\n");
}

void ugetpid_test() {
  int i;

  printf("ugetpid_test starting\n");
  testname = "ugetpid_test";

  for (i = 0; i < 64; i++) {
    int ret = fork();
    if (ret != 0) {
      wait(&ret);
      if (ret != 0)
        exit(1);
      continue;
    }
    if (getpid() != ugetpid())
      err("missmatched PID");
    exit(0);
  }
  printf("ugetpid_test: OK\n");
}

void print_kpgtbl() {
  printf("print_kpgtbl starting\n");
  kpgtbl();
  printf("print_kpgtbl: OK\n");
}

void supercheck(uint64 s) {
  pte_t last_pte = 0;

  for (uint64 p = s; p < s + 512 * PGSIZE; p += PGSIZE) {
    pte_t pte = (pte_t)pgpte((void *)p);
    if (pte == 0)
      err("no pte");
    if ((uint64)last_pte != 0 && pte != last_pte) {
      err("pte different");
    }
    if ((pte & PTE_V) == 0 || (pte & PTE_R) == 0 || (pte & PTE_W) == 0) {
      err("pte wrong");
    }
    last_pte = pte;
  }

  for (int i = 0; i < 512; i += PGSIZE) {
    *(int *)(s + i) = i;
  }

  for (int i = 0; i < 512; i += PGSIZE) {
    if (*(int *)(s + i) != i)
      err("wrong value");
  }
}

void superpg_test() {
  int pid;

  printf("superpg_test starting\n");
  testname = "superpg_test";

  char *end = sbrk(N);
  if (end == 0 || end == (char *)0xffffffffffffffff)
    err("sbrk failed");

  uint64 s = SUPERPGROUNDUP((uint64)end);
  supercheck(s);
  if ((pid = fork()) < 0) {
    err("fork");
  } else if (pid == 0) {
    supercheck(s);
    exit(0);
  } else {
    int status;
    wait(&status);
    if (status != 0) {
      exit(0);
    }
  }
  printf("superpg_test: OK\n");
}

int main(int argc, char *argv[]) {
  print_pgtbl();
  ugetpid_test();
  print_kpgtbl();
  // Call both to satisfy requirements and repo check
  pgaccess_test();
  superpg_test();
  printf("pgtbltest: all tests succeeded\n");
  exit(0);
}
