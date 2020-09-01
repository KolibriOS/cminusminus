#include <sys/stat.h>
#include "tok.h"

long cmm_filelength(int fd) {
  struct stat sb;
  if (fstat(fd, &sb) != 0)
    return -1;
  return sb.st_size;
}

void cmm_strupr(char *string) {
  char ch;
  while ((ch = *string)) {
    if (ch >= 'a' && ch <= 'z')
      *string = ch - 0x20;
    string++;
  }
}

void cmm_strlwr(char *string) {
  char ch;
  while ((ch = *string)) {
    if (ch >= 'A' && ch <= 'Z')
      *string = ch + 0x20;
    string++;
  }
}

int cmm_strcmpi(char *s1, char *s2) {
  int i;
  for (i = 0; s1[i] && s2[i]; i++) {
	if (s1[i] == s2[i] || (s1[i] ^ 32) == s2[i]) continue;
	else break;
  }
  if (s1[i] == s2[i]) return 0;
  if ((s1[i] | 32) < (s2[i] | 32)) return -1;
  return 1;
}

