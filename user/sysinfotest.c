#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sinfo(struct sysinfo *info) {
  if (sysinfo(info) < 0) {
    printf("FAIL: sysinfo failed\n");
    exit(1);
  }
}

void testmem(void) {
  struct sysinfo info;
  uint64 n = (uint64)1024 * 1024;
  char *p;

  printf("sysinfo freemem test: ");
  sinfo(&info);
  printf("free memory before malloc: %d bytes\n", (int)info.freemem);

  p = malloc(n);
  if (p == 0) {
    printf("FAIL: malloc failed\n");
    exit(1);
  }

  sinfo(&info);
  printf("free memory after malloc: %d bytes\n", (int)info.freemem);

  free(p);
  sinfo(&info);
  printf("free memory after free: %d bytes\n", (int)info.freemem);

  printf("sysinfo freemem test: OK\n");
}

void testproc(void) {
  struct sysinfo info;
  int pid;
  int status;

  printf("sysinfo nproc test: ");
  sinfo(&info);
  printf("nproc before fork: %d\n", (int)info.nproc);

  pid = fork();
  if (pid < 0) {
    printf("FAIL: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // Child process
    sinfo(&info);
    printf("nproc in child: %d\n", (int)info.nproc);
    exit(0);
  }

  // Parent process
  wait(&status);
  sinfo(&info);
  printf("nproc after child exit: %d\n", (int)info.nproc);

  printf("sysinfo nproc test: OK\n");
}

int main(int argc, char *argv[]) {
  printf("sysinfotest starting\n");

  testmem();
  testproc();

  printf("sysinfotest: OK\n");
  exit(0);
}
