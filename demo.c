#include <stdio.h>

#include "scanner.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: demo FILENAME\n");
        return 1;
    }

    const char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = malloc(sizeof(char) * size + 1);
    size_t read = fread(buf, sizeof(char), size, fp);
    buf[read] = '\0';
    fclose(fp);

    mt_Scanner *scanner = malloc(sizeof(mt_Scanner *));
    mt_scanner_init(scanner, buf);

    char debug_buf[255];
    mt_Token *token = malloc(sizeof(mt_Token *));
    do {
        mt_scanner_scan(scanner, token);
        mt_token_debug(token, debug_buf, 255);
        printf("token: %s\n", debug_buf);
    } while (token->type != mt_TOKEN_EOF);

    free(scanner);
    free(token);

    return 0;
}
