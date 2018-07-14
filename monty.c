#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

/// --tokenize
static bool tokenize = false;

/// -
static bool source_from_stdin = false;

/// -c SOURCE
static char *source_from_cli = NULL;

/// FILENAME
static char *source_from_filename = NULL;

#define print_error(msg, ...) do { fprintf(stderr, "error: " msg "\n", ##__VA_ARGS__); exit(1); } while (0)

void print_usage(char *program_name) {
    fprintf(
        stderr,
        "usage: %s [OPTION [OPTION ...]] [- | -c SOURCE | FILENAME] [ARG [ARG ...]]\n"
        "\n"
        "options:\n"
        "  -h, --help     : print this message and exit\n"
        "  -v, --version  : print the current version and exit\n"
        "  -t, --tokenize : print all the tokens in the source code without interpreting it\n"
        "  -              : read source from stdin\n"
        "  -c SOURCE      : read source from string\n"
        "  FILENAME       : read source from file\n"
        "  ARG            : argument passed to program via 'std.cli.args'\n",
        program_name
    );
    exit(1);
}

void print_version() {
    printf("monty %s\n", mt_VERSION);
    exit(0);
}

void parse_args(int *argc, char *argv[]) {
    if (*argc <= 1) {
        print_usage(argv[0]);
        return;
    }

    for (int i = 1; i < *argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return;
        }

        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return;
        }

        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tokenize") == 0) {
            tokenize = true;
            continue;
        }

        if (strcmp(argv[i], "-c") == 0) {
            if (++i >= *argc) {
                print_error("-c flag expects an argument");
                return;
            }

            source_from_cli = argv[i];
            return;
        }

        if (strcmp(argv[i], "-") == 0) {
            source_from_stdin = true;
            return;
        }

        if (argv[i][0] == '-') {
            print_error("unrecognized command line argument '%s'", argv[i]);
            return;
        }

        source_from_filename = argv[i];
        return;
    }
}

int main(int argc, char *argv[]) {
    parse_args(&argc, argv);

    mt_Scanner *scanner = malloc(sizeof(mt_Scanner *));
    mt_Token *token = malloc(sizeof(mt_Token *));

    char *error = NULL;
    char *source = NULL;
    if (source_from_cli) {
        mt_scanner_init(scanner, source_from_cli);
    } else if (source_from_filename) {
        source = mt_read_entire_file(source_from_filename);
        if (!source) {
            error = "could not read file";
            goto fail;
        }

        mt_scanner_init(scanner, source);
    } else if (source_from_stdin) {
        source = mt_read_entire_stdin();
        mt_scanner_init(scanner, source);
    } else {
        error = "interpreter not implemented";
        goto fail;
    }

    if (tokenize) {
        char debug_buf[255];
        do {
            mt_scanner_scan(scanner, token);
            mt_token_debug(token, debug_buf, sizeof(debug_buf));
            printf("token: %s\n", debug_buf);
        } while (token->type != mt_TOKEN_EOF);
    } else {
        error = "interpreter not implemented";
        goto fail;
    }

    if (source) free(source);
    free(scanner);
    free(token);
    return 0;

fail:
    if (source) free(source);
    if (scanner) free(scanner);
    if (token) free(token);
    print_error("%s", error);
    return 1;
}
