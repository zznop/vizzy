#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/time.h>

#define TMP_BUFFER_SZ 8192
#define LOG_BUF_SZ 256
#define LOAD_SYM(sym, type, name) {         \
    if (!sym) {                             \
        sym = (type)dlsym(RTLD_NEXT, name); \
    }                                       \
}

typedef void *(*malloc_t)(size_t size);
static malloc_t m_malloc_real = NULL;

typedef void *(*calloc_t)(size_t num, size_t size);
static calloc_t m_calloc_real = NULL;

typedef void *(*realloc_t)(void *ptr, size_t new_size);
static realloc_t m_realloc_real = NULL;

typedef void (*free_t)(void *ptr);
static free_t m_free_real = NULL;

typedef void *(*mmap_t)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
static mmap_t m_mmap_real = NULL;

typedef int (*munmap_t)(void *addr, size_t length);
static munmap_t m_munmap_real = NULL;

static void _heap_info_log_line(const char *line)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    char buf[LOG_BUF_SZ];
    int n = snprintf(buf, sizeof(buf), "%ld,%ld,%s\n", tv.tv_sec, tv.tv_usec, line);
    if ((unsigned)n >= sizeof(buf))
        return; // truncated

    int fd = open("/tmp/heapinfo.txt", O_RDWR|O_APPEND|O_CREAT, 0644);
    if (!fd)
        return;

    int size = strlen(buf);
    for (int total = 0; total < size;) {
        int n = write(fd, buf, size);
        total += n;
    }

    close(fd);
}

static void _load_real_symbols(void)
{
    static int loaded = false;
    if (loaded)
        return;
    else
        loaded = true;

    LOAD_SYM(m_malloc_real, malloc_t, "malloc");
    LOAD_SYM(m_calloc_real, calloc_t, "calloc");
    LOAD_SYM(m_realloc_real, realloc_t, "realloc");
    LOAD_SYM(m_free_real, free_t, "free");
    LOAD_SYM(m_mmap_real, mmap_t, "mmap");
    LOAD_SYM(m_munmap_real, munmap_t, "munmap");
}

void __attribute__((constructor)) init(void)
{
    _load_real_symbols();
}

void *malloc(size_t size)
{
    _load_real_symbols();

    char buf[LOG_BUF_SZ];
    char *ptr = m_malloc_real(size);
    int n = snprintf(buf, sizeof(buf), "%s,%p,%ld", __func__, ptr, size);
    if ((unsigned)n >= sizeof(buf))
        return ptr; // truncated

    _heap_info_log_line(buf);
    return ptr;
}

void *calloc(size_t num, size_t size)
{
    // The dynamic linker uses calloc so we must mimic the first call
    static uint8_t tmpbuf[TMP_BUFFER_SZ] = {0};
    if (!m_calloc_real)
        return tmpbuf;

    _load_real_symbols();
    void * ptr = m_calloc_real(num, size);

    char buf[LOG_BUF_SZ];
    int n = snprintf(buf, sizeof(buf), "%s,%p,%ld", __func__, ptr, size*num);
    if ((unsigned)n >= sizeof(buf))
        return ptr; // truncated

    _heap_info_log_line(buf);
    return ptr;
}

void *realloc(void *ptr, size_t new_size)
{
    _load_real_symbols();
    void *newptr = m_realloc_real(ptr, new_size);

    char buf[LOG_BUF_SZ];
    int n = snprintf(buf, sizeof(buf), "%s,%p,%ld", __func__, newptr, new_size);
    if ((unsigned)n >= sizeof(buf))
        return newptr; // truncated

    return newptr;
}

void free(void *ptr)
{
    _load_real_symbols();
    if (!ptr)
        return;

    m_free_real(ptr);

    char buf[LOG_BUF_SZ];
    int n = snprintf(buf, sizeof(buf), "%s,%p,", __func__, ptr);
    if ((unsigned)n >= sizeof(buf))
        return; // truncated

    _heap_info_log_line(buf);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    _load_real_symbols();
    void *ptr = m_mmap_real(addr, length, prot, flags, fd, offset);

    char buf[LOG_BUF_SZ];
    int n = snprintf(buf, sizeof(buf), "%s,%p,%ld", __func__, ptr, length);
    if ((unsigned)n >= sizeof(buf))
        return ptr; // truncated

    _heap_info_log_line(buf);
    return ptr;
}

int munmap(void *addr, size_t length)
{
    _load_real_symbols();
    int rc = m_munmap_real(addr, length);

    char buf[LOG_BUF_SZ];
    int n = snprintf(buf, sizeof(buf), "%s,%p,%ld", __func__, addr, length);
    if ((unsigned)n >= sizeof(buf))
        return rc; // truncated

    _heap_info_log_line(buf);
    return rc;
}
