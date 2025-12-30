#include "stdio.h"
#include "string.h"
#include "sys/calls/fd.h"

static FILE _stdin;
static FILE _stdout;
static FILE _stderr;

FILE *stdin  = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;

void InitStdio(void)
{
    memset(&_stdin, 0, sizeof(FILE));
    _stdin.fd                    = kFdStdIn;
    _stdin.flags.open_mode       = kFdFlagRead;
    _stdin.flags.buffer_mode     = kFdBufferLine;
    _stdin.flags.eof             = false;
    _stdin.flags.error           = false;
    _stdin.flags.closed          = false;
    _stdin.flags.is_buffer_owner = false;
    _stdin.buffer                = NULL;
    _stdin.buffer_size           = 0;
    _stdin.buffer_pos            = 0;
    _stdin.buffer_level          = 0;
    _stdin.file_pos              = 0;

    memset(&_stdout, 0, sizeof(FILE));
    _stdout.fd                    = kFdStdOut;
    _stdout.flags.open_mode       = kFdFlagWrite;
    _stdout.flags.buffer_mode     = kFdBufferLine;
    _stdout.flags.eof             = false;
    _stdout.flags.error           = false;
    _stdout.flags.closed          = false;
    _stdout.flags.is_buffer_owner = false;
    _stdout.buffer                = NULL;
    _stdout.buffer_size           = 0;
    _stdout.buffer_pos            = 0;
    _stdout.buffer_level          = 0;
    _stdout.file_pos              = 0;

    memset(&_stderr, 0, sizeof(FILE));
    _stderr.fd                    = kFdStdErr;
    _stderr.flags.open_mode       = kFdFlagWrite;
    _stderr.flags.buffer_mode     = kFdBufferNone;
    _stderr.flags.eof             = false;
    _stderr.flags.error           = false;
    _stderr.flags.closed          = false;
    _stderr.flags.is_buffer_owner = false;
    _stderr.buffer                = NULL;
    _stderr.buffer_size           = 0;
    _stderr.buffer_pos            = 0;
    _stderr.buffer_level          = 0;
    _stderr.file_pos              = 0;
}
