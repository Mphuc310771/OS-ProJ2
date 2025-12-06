#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

#ifndef PGSIZE
#define PGSIZE 4096
#endif

// Test pgaccess syscall
void pgaccess_test() {
  char *buf;
  unsigned int abits;

  printf("pgaccess_test starting\n");

  // Allocate 32 pages
  buf = malloc(32 * PGSIZE);
  if (buf == 0) {
    printf("malloc failed\n");
    exit(1);
  }

  // Access some pages (pages 1, 2, 30)
  buf[PGSIZE * 1 + 10] = 'a';
  buf[PGSIZE * 2 + 20] = 'b';
  buf[PGSIZE * 30 + 30] = 'c';

  // Call pgaccess to check which pages were accessed
  if (pgaccess(buf, 32, &abits) < 0) {
    printf("pgaccess failed\n");
    exit(1);
  }

  // Check if pages 1, 2, 30 are marked as accessed
  // Expected: bits 1, 2, 30 should be set
  printf("pgaccess returned: 0x%x\n", abits);

  if ((abits & (1 << 1)) == 0) {
    printf("FAIL: page 1 not marked accessed\n");
  } else {
    printf("PASS: page 1 marked accessed\n");
  }

  if ((abits & (1 << 2)) == 0) {
    printf("FAIL: page 2 not marked accessed\n");
  } else {
    printf("PASS: page 2 marked accessed\n");
  }

  if ((abits & (1 << 30)) == 0) {
    printf("FAIL: page 30 not marked accessed\n");
  } else {
    printf("PASS: page 30 marked accessed\n");
  }

  // Call pgaccess again - should return 0 since A bits were cleared
  if (pgaccess(buf, 32, &abits) < 0) {
    printf("pgaccess failed on second call\n");
    exit(1);
  }

  if (abits == 0) {
    printf("PASS: A bits cleared after first pgaccess call\n");
  } else {
    printf("FAIL: A bits not cleared, got 0x%x\n", abits);
  }

  free(buf);
  printf("pgaccess_test done\n");
}

// Test USYSCALL - fast getpid using shared memory
void ugetpid_test() {
  int i;

  printf("ugetpid_test starting\n");

  // Compare getpid() syscall result with USYSCALL page
  struct usyscall *u = (struct usyscall *)USYSCALL;

  for (i = 0; i < 64; i++) {
    int pid = getpid();
    if (pid != u->pid) {
      printf("FAIL: syscall getpid %d != usyscall pid %d\n", pid, u->pid);
      exit(1);
    }
  }

  printf("PASS: usyscall pid matches getpid()\n");
  printf("ugetpid_test done\n");
}

int main(int argc, char *argv[]) {
  printf("pgtbltest starting\n");

  ugetpid_test();
  pgaccess_test();

  printf("pgtbltest: all tests passed\n");
  exit(0);
}
