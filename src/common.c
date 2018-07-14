#include <stdio.h>
#include <stdlib.h>

size_t mt_get_file_size(FILE *handle) {
    size_t file_size, old_position;

    old_position = ftell(handle);
    fseek(handle, 0, SEEK_END);
    file_size = ftell(handle);
    fseek(handle, 0, old_position);
    return file_size;
}

char *mt_read_entire_file(char *filename) {
    FILE *handle = fopen(filename, "r");
    if (!handle) return NULL;

    size_t file_size = mt_get_file_size(handle);
    char *outbuf = malloc(sizeof(char) * (file_size + 1));
    if (!outbuf) {
        fclose(handle);
        return NULL;
    }

    size_t bytes_read = fread(outbuf, sizeof(char), file_size, handle);
    if (bytes_read < file_size) {
        free(outbuf);
        fclose(handle);
        return NULL;
    }

    outbuf[file_size] = '\0';
    fclose(handle);
    return outbuf;
}
