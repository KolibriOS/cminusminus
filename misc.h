#ifndef O_BINARY
#define O_BINARY 0
#define O_TEXT 0
#endif

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#ifdef _MSC_VER
#define PATH_SEP_C '\\'
#define PATH_SEP "\\"
#else
#define PATH_SEP_C '/'
#define PATH_SEP "/"
#endif

long cmm_filelength(int fd);
void cmm_strupr(char *string);
void cmm_strlwr(char *string);
int cmm_strcmpi(char *s1, char *s2);
