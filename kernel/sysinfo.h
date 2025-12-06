#ifndef SYSINFO_H
#define SYSINFO_H

#include "types.h"

struct sysinfo {
  uint64 freemem; // amount of free memory (bytes)
  uint64 nproc;   // number of process
};

#endif
