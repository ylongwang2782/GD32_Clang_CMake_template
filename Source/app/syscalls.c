#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int _close(int fd) { return -1; }

int _fstat(int fd, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd) { return 1; }

int _lseek(int fd, int ptr, int dir) { return 0; }

int _read(int fd, char *ptr, int len) { return 0; }

int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

int _getpid(void) {
    return 1;
}
