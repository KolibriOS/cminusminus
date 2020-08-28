#include <sys/stat.h>
#include "tok.h"

long filelength(int fd) {
  struct stat sb;
  if (fstat(fd, &sb) != 0)
    return -1;
  return sb.st_size;
}

void strupr(char *string) {
  char ch;
  while ((ch = *string)) {
    if (ch >= 'a' && ch <= 'z')
      *string = ch - 0x20;
    string++;
  }
}

void strlwr(char *string) {
  char ch;
  while ((ch = *string)) {
    if (ch >= 'A' && ch <= 'Z')
      *string = ch + 0x20;
    string++;
  }
}

int strcmpi(char *s1, char *s2) {
  int i;
  for (i = 0; s1[i] && s2[i]; i++) {
	if (s1[i] == s2[i] || (s1[i] ^ 32) == s2[i]) continue;
	else break;
  }
  if (s1[i] == s2[i]) return 0;
  if ((s1[i] | 32) < (s2[i] | 32)) return -1;
  return 1;
}
