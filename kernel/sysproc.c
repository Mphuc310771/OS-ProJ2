#include "defs.h"
#include "memlayout.h"
#include "param.h"
#include "proc.h"
#include "riscv.h"
#include "spinlock.h"
#include "sysinfo.h"
#include "types.h"

uint64 sys_exit(void) {
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64 sys_getpid(void) { return myproc()->pid; }

uint64 sys_fork(void) { return fork(); }

uint64 sys_wait(void) {
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64 sys_sbrk(void) {
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64 sys_sleep(void) {
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64 sys_kill(void) {
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64 sys_uptime(void) {
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_trace(void) {
  int mask;
  argint(0, &mask);
  myproc()->trace_mask = mask;
  return 0;
}

// Extern declarations for functions defined in other files
extern uint64 freemem(void);
extern uint64 nproc(void);

uint64 sys_sysinfo(void) {
  uint64 addr;
  struct sysinfo info;
  struct proc *p = myproc();

  argaddr(0, &addr);

  info.freemem = freemem();
  info.nproc = nproc();

  if (copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1;
  return 0;
}

#define PGACCESS_MAX 32 // Maximum number of pages to check

uint64 sys_pgaccess(void) {
  uint64 base;      // Starting virtual address
  int len;          // Number of pages to check
  uint64 mask_addr; // User address to store the result bitmask

  struct proc *p = myproc();

  // Get arguments
  argaddr(0, &base);
  argint(1, &len);
  argaddr(2, &mask_addr);

  // Validate len
  if (len < 0 || len > PGACCESS_MAX)
    return -1;

  uint32 mask = 0;

  // Check each page
  for (int i = 0; i < len; i++) {
    uint64 va = base + i * PGSIZE;

    // Get the PTE for this virtual address
    pte_t *pte = walk(p->pagetable, va, 0);

    if (pte == 0)
      continue; // Page not mapped

    if (!(*pte & PTE_V))
      continue; // Page not valid

    // Check if the Access bit is set
    if (*pte & PTE_A) {
      mask |= (1 << i); // Set corresponding bit in mask
      *pte &= ~PTE_A;   // Clear the Access bit
    }
  }

  // Copy the mask to user space
  if (copyout(p->pagetable, mask_addr, (char *)&mask, sizeof(mask)) < 0)
    return -1;

  return 0;
}
