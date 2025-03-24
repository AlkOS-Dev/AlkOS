#ifndef ALKOS_LIBC_INCLUDE_ERRNO_H_
#define ALKOS_LIBC_INCLUDE_ERRNO_H_

#include <defines.h>
#include <todo.h>

// ------------------------------
// Error type
// ------------------------------

typedef int error_t;

typedef error_t errno_t;

// ------------------------------
// Errno access
// ------------------------------

/* Errno should be thread local */
TODO_THREADING

BEGIN_DECL_C
error_t *__access_errno(void);

END_DECL_C

#define errno (*__access_errno())

// ------------------------------
// Error defines
// ------------------------------

#define __cast_er(x) ((error_t)(x))

#define NO_ERROR __cast_er(0)

/* ISO C standard errors */
#define EDOM   __cast_er(1) /* Math argument out of domain of func */
#define EILSEQ __cast_er(2) /* Illegal byte sequence */
#define ERANGE __cast_er(3) /* Result too large */

/* POSIX errors */
/* source: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html */
#define E2BIG           __cast_er(4)  /* Argument list too long */
#define EACCES          __cast_er(5)  /* Permission denied */
#define EADDRINUSE      __cast_er(6)  /* Address in use */
#define EADDRNOTAVAIL   __cast_er(7)  /* Address not available */
#define EAFNOSUPPORT    __cast_er(8)  /* Address family not supported */
#define EAGAIN          __cast_er(9)  /* Resource unavailable, try again */
#define EALREADY        __cast_er(10) /* Connection already in progress */
#define EBADF           __cast_er(11) /* Bad file descriptor */
#define EBADMSG         __cast_er(12) /* Bad message */
#define EBUSY           __cast_er(13) /* Device or resource busy */
#define ECANCELED       __cast_er(14) /* Operation canceled */
#define ECHILD          __cast_er(15) /* No child processes */
#define ECONNABORTED    __cast_er(16) /* Connection aborted */
#define ECONNREFUSED    __cast_er(17) /* Connection refused */
#define ECONNRESET      __cast_er(18) /* Connection reset */
#define EDEADLK         __cast_er(19) /* Resource deadlock would occur */
#define EDESTADDRREQ    __cast_er(20) /* Destination address required */
#define EDQUOT          __cast_er(21) /* Reserved */
#define EEXIST          __cast_er(22) /* File exists */
#define EFAULT          __cast_er(23) /* Bad address */
#define EFBIG           __cast_er(24) /* File too large */
#define EHOSTUNREACH    __cast_er(25) /* Host is unreachable */
#define EIDRM           __cast_er(26) /* Identifier removed */
#define EINPROGRESS     __cast_er(27) /* Operation in progress */
#define EINTR           __cast_er(28) /* Interrupted function */
#define EINVAL          __cast_er(29) /* Invalid argument */
#define EIO             __cast_er(30) /* I/O error */
#define EISCONN         __cast_er(31) /* Socket is connected */
#define EISDIR          __cast_er(32) /* Is a directory */
#define ELOOP           __cast_er(33) /* Too many levels of symbolic links */
#define EMFILE          __cast_er(34) /* File descriptor value too large */
#define EMLINK          __cast_er(35) /* Too many links */
#define EMSGSIZE        __cast_er(36) /* Message too large */
#define EMULTIHOP       __cast_er(37) /* Reserved */
#define ENAMETOOLONG    __cast_er(38) /* Filename too long */
#define ENETDOWN        __cast_er(39) /* Network is down */
#define ENETRESET       __cast_er(40) /* Connection aborted by network */
#define ENETUNREACH     __cast_er(41) /* Network unreachable */
#define ENFILE          __cast_er(42) /* Too many files open in system */
#define ENOBUFS         __cast_er(43) /* No buffer space available */
#define ENODATA         __cast_er(44) /* No message available on STREAM head read queue */
#define ENODEV          __cast_er(45) /* No such device */
#define ENOENT          __cast_er(46) /* No such file or directory */
#define ENOEXEC         __cast_er(47) /* Executable file format error */
#define ENOLCK          __cast_er(48) /* No locks available */
#define ENOLINK         __cast_er(49) /* Reserved */
#define ENOMEM          __cast_er(50) /* Not enough space */
#define ENOMSG          __cast_er(51) /* No message of the desired type */
#define ENOPROTOOPT     __cast_er(52) /* Protocol not available */
#define ENOSPC          __cast_er(53) /* No space left on device */
#define ENOSR           __cast_er(54) /* No STREAM resources */
#define ENOSTR          __cast_er(55) /* Not a STREAM */
#define ENOSYS          __cast_er(56) /* Functionality not supported */
#define ENOTCONN        __cast_er(57) /* The socket is not connected */
#define ENOTDIR         __cast_er(58) /* Not a directory or symbolic link to directory */
#define ENOTEMPTY       __cast_er(59) /* Directory not empty */
#define ENOTRECOVERABLE __cast_er(60) /* State not recoverable */
#define ENOTSOCK        __cast_er(61) /* Not a socket */
#define ENOTSUP         __cast_er(62) /* Not supported */
#define ENOTTY          __cast_er(63) /* Inappropriate I/O control operation */
#define ENXIO           __cast_er(64) /* No such device or address */
#define EOPNOTSUPP      ENOTSUP       /* Operation not supported on socket */
#define EOVERFLOW       __cast_er(65) /* Value too large to be stored in data type */
#define EOWNERDEAD      __cast_er(66) /* Previous owner died */
#define EPERM           __cast_er(67) /* Operation not permitted */
#define EPIPE           __cast_er(68) /* Broken pipe */
#define EPROTO          __cast_er(69) /* Protocol error */
#define EPROTONOSUPPORT __cast_er(70) /* Protocol not supported */
#define EPROTOTYPE      __cast_er(71) /* Protocol wrong type for socket */
#define EROFS           __cast_er(72) /* Read-only file system */
#define ESPIPE          __cast_er(73) /* Invalid seek */
#define ESRCH           __cast_er(74) /* No such process */
#define ESTALE          __cast_er(75) /* Reserved */
#define ETIME           __cast_er(76) /* Stream ioctl timeout */
#define ETIMEDOUT       __cast_er(77) /* Connection timed out */
#define ETXTBSY         __cast_er(78) /* Text file busy */
#define EWOULDBLOCK     EAGAIN        /* Operation would block */
#define EXDEV           __cast_er(79) /* Cross-device link */

#endif  // ALKOS_LIBC_INCLUDE_ERRNO_H_
