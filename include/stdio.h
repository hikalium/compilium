#define NULL 0
#define EOF (-1)

typedef unsigned long size_t;
typedef struct FILE FILE;

struct FILE;

#ifdef __APPLE__
#define stdout __stdoutp
extern FILE *__stdoutp;
#define stderr __stderrp
extern FILE *__stderrp;
#else
extern FILE *stdout;
extern FILE *stderr;

#endif

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int snprintf(char *, unsigned long, const char *format, ...);
int vfprintf(struct FILE *, const char *, va_list);

int fputc(int c, FILE *);
int putchar(int c);
int getchar(void);
int fflush(FILE *);

