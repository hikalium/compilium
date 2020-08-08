#include <stdarg.h>

#define NULL 0
#define EOF (-1)

typedef unsigned long size_t;
typedef struct FILE FILE;

struct FILE;

#ifdef __APPLE__
#define stdin __stdinp
extern FILE *__stdinp;
#define stdout __stdoutp
extern FILE *__stdoutp;
#define stderr __stderrp
extern FILE *__stderrp;
#else
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#endif

FILE *fopen(const char *, const char *);
int fclose(FILE *);
int fflush(FILE *);
int fgetc(FILE *);
int fprintf(FILE *, const char *, ...);
int fputc(int c, FILE *);
int puts(char *s);
int fputs(const char *, FILE *);
int getchar(void);
int printf(const char *, ...);
int putchar(int c);
int snprintf(char *, unsigned long, const char *, ...);
int vfprintf(struct FILE *, const char *, va_list);
