#include "kernel/param.h"
#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int i;
  char *nargv[MAXARG];

  // Kiểm tra tham số đầu vào
  if (argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')) {
    fprintf(2, "Usage: %s mask command\n", argv[0]);
    exit(1);
  }

  // Gọi system call trace
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);
    exit(1);
  }

  // Chuẩn bị tham số cho lệnh tiếp theo (ví dụ: grep hello README)
  for (i = 2; i < argc && i < MAXARG; i++) {
    nargv[i - 2] = argv[i];
  }
  nargv[i - 2] = 0;

  // Thực thi lệnh đó
  exec(nargv[0], nargv);
  fprintf(2, "exec %s failed\n", nargv[0]);
  exit(1);
}