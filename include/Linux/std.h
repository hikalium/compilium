#define NULL 0
#define EXIT_FAILURE 1

typedef struct __sFILE FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define va_start __builtin_va_start
#define va_end __builtin_va_end
//#define va_arg __builtin_va_arg
//#define va_copy(d, s) __builtin_va_copy((d), (s))
#define va_list __builtin_va_list
int vfprintf(FILE *stream, const char *format, va_list ap);

void assert();
typedef unsigned long size_t;
void *calloc(unsigned long, unsigned long);
int printf(const char *, ...);
int putchar(int);
char *strchr(const char *s, int c);
int fclose(FILE *stream);
size_t fread(void *, size_t, size_t, FILE *);
void *malloc(size_t size);
int strcmp(const char *s1, const char *s2);
char *strncat(char *s1, const char *s2, size_t n);
char *strcat(char *s1, const char *s2);
FILE *fopen(const char *path, const char *mode);
size_t strlen(const char *);
char *strncpy(char *dst, const char *src, size_t len);
long strtol(const char *str, char **endptr, int base);
int puts(const char *s);
_Noreturn void exit(int);
void free(void *);
FILE *fdopen(int fildes, const char *mode);
int fflush(FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int fputs(const char *s, FILE *stream);
int fputc(int, FILE *);
