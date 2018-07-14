#ifndef mt_common_h
#define mt_common_h

#include <stdio.h>

#define mt_VERSION "0.0.0"

/// Get the size of a file in bytes.
size_t mt_get_file_size(FILE *);

/// Read stdin until EOF and return the resulting character buffer.
char *mt_read_entire_stdin(void);

/// Read an entire file into a character buffer.  The caller is
/// expected to free the buffer.  Returns NULL on error.
char *mt_read_entire_file(char *);

#endif
