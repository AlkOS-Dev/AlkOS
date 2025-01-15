#include <errno.h>

/* Errno should be thread local */
TODO_THREADING
error_t g_errno = NO_ERROR;

error_t *__access_errno() { return &g_errno; }
