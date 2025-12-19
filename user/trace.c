#include "kernel/param.h"
#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

// Maximum length of mask string to prevent overflow (10 digits for 32-bit int)
#define MAX_MASK_LEN 10

// Check if a string is a valid non-negative integer
// Returns 1 if valid, 0 if invalid
int is_valid_number(char *s) {
  int i = 0;
  int len = 0;

  // Empty string is invalid
  if (s == 0 || s[0] == '\0') {
    return 0;
  }

  // Check for negative sign (not allowed)
  if (s[0] == '-') {
    return 0;
  }

  // Check each character is a digit
  while (s[i] != '\0') {
    if (s[i] < '0' || s[i] > '9') {
      return 0; // Non-digit character found
    }
    len++;
    i++;
  }

  // Check for overflow (too many digits)
  if (len > MAX_MASK_LEN) {
    return 0;
  }

  return 1;
}

int main(int argc, char *argv[]) {
  int i;
  char *nargv[MAXARG];

  // Check minimum number of arguments
  if (argc < 3) {
    fprintf(2, "Error: Not enough arguments\n");
    fprintf(2, "Usage: %s mask command [args...]\n", argv[0]);
    fprintf(2, "  mask: non-negative integer (bitmask of syscalls to trace)\n");
    fprintf(2, "  command: program to run\n");
    exit(1);
  }

  // Validate mask is a valid non-negative integer
  if (!is_valid_number(argv[1])) {
    fprintf(2, "Error: Invalid mask '%s'\n", argv[1]);
    fprintf(
        2, "Mask must be a non-negative integer (digits only, max %d digits)\n",
        MAX_MASK_LEN);
    fprintf(2, "Examples:\n");
    fprintf(2, "  trace 32 grep hello README    (trace read syscall)\n");
    fprintf(2, "  trace 2147483647 ls           (trace all syscalls)\n");
    exit(1);
  }

  // Call trace system call
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "Error: trace syscall failed\n");
    exit(1);
  }

  // Prepare arguments for the command to execute
  for (i = 2; i < argc && i < MAXARG; i++) {
    nargv[i - 2] = argv[i];
  }
  nargv[i - 2] = 0;

  // Execute the command
  exec(nargv[0], nargv);
  fprintf(2, "Error: exec '%s' failed\n", nargv[0]);
  exit(1);
}
