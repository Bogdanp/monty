#include <stdio.h>

#include "common.h"
#include "scanner.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: demo FILENAME\n");
        return 1;
    }

    char *source = mt_read_entire_file(argv[1]);
    if (!source) {
        fprintf(stderr, "error: failed to read %s\n", argv[1]);
        return 1;
    }

    mt_Scanner *scanner = malloc(sizeof(mt_Scanner *));
    mt_scanner_init(scanner, source);

    char debug_buf[255];
    mt_Token *token = malloc(sizeof(mt_Token *));
    do {
        mt_scanner_scan(scanner, token);
        mt_token_debug(token, debug_buf, sizeof(debug_buf));
        printf("token: %s\n", debug_buf);
    } while (token->type != mt_TOKEN_EOF);

    free(source);
    free(scanner);
    free(token);

    return 0;
}
