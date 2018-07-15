#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "parser.h"
#include "scanner.h"

/// --dump-ast
static bool dump_ast = false;

/// --dump-tokens
static bool dump_tokens = false;

/// -
static bool source_from_stdin = false;

/// -c SOURCE
static char *source_from_cli = NULL;

/// FILENAME
static char *source_from_filename = NULL;

#define print_error(msg, ...) do { fprintf(stderr, "error: " msg "\n", ##__VA_ARGS__); exit(1); } while (0)

static void print_usage(char *program_name) {
    fprintf(
        stderr,
        "usage: %s [OPTION [OPTION ...]] [- | -c SOURCE | FILENAME] [ARG [ARG ...]]\n"
        "\n"
        "options:\n"
        "  -h, --help     : print this message and exit\n"
        "  -v, --version  : print the current version and exit\n"
        "  --dump-ast     : print all the AST nodes in the source code without interpreting it\n"
        "  --dump-tokens  : print all the tokens in the source code without interpreting it\n"
        "  -              : read source from stdin\n"
        "  -c SOURCE      : read source from string\n"
        "  FILENAME       : read source from file\n"
        "  ARG            : argument passed to program via 'std.cli.args'\n",
        program_name
    );
    exit(1);
}

static void print_version() {
    printf("monty %s\n", mt_VERSION);
    exit(0);
}

#define MS NULL
static bool match(char *arg, ...) {
    va_list args;
    va_start(args, arg);

    while (true) {
        char *variant = va_arg(args, char *);
        if (variant == MS) {
            return false;
        }

        if (strcmp(arg, variant) == 0) {
            return true;
        }
    }

    va_end(args);
}

static void parse_args(int *argc, char *argv[]) {
    if (*argc <= 1) {
        print_usage(argv[0]);
        return;
    }

    for (int i = 1; i < *argc; i++) {
        char *arg = argv[i];

        if (match(arg, "-h", "--help", MS)) {
            print_usage(argv[0]);
            return;
        }

        if (match(arg, "-v", "--version", MS)) {
            print_version();
            return;
        }

        if (match(arg, "--dump-ast", MS)) {
            dump_ast = true;
            continue;
        }

        if (match(arg, "--dump-tokens", MS)) {
            dump_tokens = true;
            continue;
        }

        if (match(arg, "-c", MS)) {
            if (++i >= *argc) {
                print_error("-c flag expects an argument");
                return;
            }

            source_from_cli = argv[i];
            return;
        }

        if (match(arg, "-", MS)) {
            source_from_stdin = true;
            return;
        }

        if (arg[0] == '-') {
            print_error("unrecognized command line argument '%s'", arg);
            return;
        }

        source_from_filename = arg;
        return;
    }
}

static void do_dump_ast(char *filename, char *source) {
    mt_Parser *parser = mt_parser_init(filename, source);
    mt_Node *tree = mt_parser_parse(parser);
    mt_node_dump(tree, stdout);
    mt_parser_free(parser);
}

static void do_dump_tokens(char *source) {
    mt_Scanner *scanner = mt_scanner_init(source);
    mt_Token *token = mt_token_init();
    char debug_buf[255];

    do {
        mt_scanner_scan(scanner, token);
        mt_token_debug(token, debug_buf, sizeof(debug_buf));

        printf("%s\n", debug_buf);
    } while (token->type != mt_TOKEN_EOF);

    mt_token_free(token);
    mt_scanner_free(scanner);
}

int main(int argc, char *argv[]) {
    parse_args(&argc, argv);

    char *error = NULL;
    char *source = NULL;
    if (source_from_cli) {
        source = source_from_cli;
    } else if (source_from_filename) {
        source = mt_read_entire_file(source_from_filename);
        if (!source) {
            error = "could not read file";
            goto fail;
        }
    } else if (source_from_stdin) {
        source = mt_read_entire_stdin();
    } else {
        error = "interpreter not implemented";
        goto fail;
    }

    if (dump_ast) {
        char *filename = source_from_filename ? source_from_filename : "[stdin]";
        do_dump_ast(filename, source);
    } else if (dump_tokens) {
        do_dump_tokens(source);
    } else {
        error = "interpreter not implemented";
        goto fail;
    }

    if (source && !source_from_cli) free(source);
    return 0;

fail:
    if (source && !source_from_cli) free(source);
    print_error("%s", error);
    return 1;
}
