void* malloc(size_t size);
void* calloc(size_t count, size_t size);
void* realloc(void* ptr, size_t size);
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
void exit(int status);
long strtol(const char* str, char** endptr, int base);
