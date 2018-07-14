#ifndef mt_common_h
#define mt_common_h

#include <stdio.h>

/// Get the size of a file in bytes.
size_t mt_get_file_size(FILE *);

/// Read an entire file into a character buffer.  The caller is
/// expected to free the buffer.  Returns NULL on error.
char *mt_read_entire_file(char *);

#endif
