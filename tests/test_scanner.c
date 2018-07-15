#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

#include "minunit.h"

int tests_run = 0;

static mt_Scanner *scanner;
static mt_Token *token;
static char buf[255] = "";

#define DEBUG_TOKEN do { mt_token_debug(token, buf, 255); printf("token: %s\n", buf); } while (0);
#define MIN(a, b) ((a < b) ? a : b)

static char *test_scanner_can_scan_empty_buffers() {
    mt_scanner_init(scanner, "");
    mt_scanner_scan(scanner, token);
    mu_assert("expected mt_TOKEN_EOF", token->type == mt_TOKEN_EOF);
    return 0;
}

static char *test_scanner_leaves_buffers_intact() {
    char original[] = "record Human\n  String name\nend";
    memcpy(buf, original, sizeof(original));

    mt_scanner_init(scanner, buf);
    do { mt_scanner_scan(scanner, token); } while (token->type != mt_TOKEN_EOF);

    mu_assert("expected the buffer to remain intact", memcmp(original, buf, sizeof(original)) == 0);
    return 0;
}

typedef struct {
    mt_TokenType type;
    char *expected;
    uint32_t line;
    uint32_t column;
} TableTest;

static char *run_table_tests(TableTest tests[], size_t ntests, char *source) {
    char value[255] = "";

    mt_token_init(token);
    mt_scanner_init(scanner, source);

    for (size_t i = 0; i < ntests; i++) {
        mt_scanner_scan(scanner, token);

        sprintf(buf, "expected token type %d got %d (test %ld)", tests[i].type, token->type, i);
        mu_assert(buf, token->type == tests[i].type);

        sprintf(buf, "expected token line %d got %d (test %ld)", tests[i].line, token->line, i);
        mu_assert(buf, token->line == tests[i].line);

        sprintf(buf, "expected token column %d got %d (test %ld)", tests[i].column, token->column, i);
        mu_assert(buf, token->column == tests[i].column);

        memcpy(value, token->start, MIN(token->length, 254));
        sprintf(buf, "expected token value %s got %s (test %ld)", tests[i].expected, value, i);
        mu_assert(buf, memcmp(tests[i].expected, value, strlen(tests[i].expected)) == 0);
    }

    return 0;
}

static char *test_scanner_can_scan_single_character_tokens() {
    TableTest tests[] = {
        { mt_TOKEN_LPAREN, "(", 1, 1 },
        { mt_TOKEN_RPAREN, ")", 1, 2 },
        { mt_TOKEN_LBRACKET, "[", 1, 3 },
        { mt_TOKEN_RBRACKET, "]", 1, 4 },
        { mt_TOKEN_DOT, ".", 2, 1 },
        { mt_TOKEN_COMMA, ",", 2, 2 },
        { mt_TOKEN_PLUS, "+", 3, 1 },
        { mt_TOKEN_MINUS, "-", 3, 2 },
        { mt_TOKEN_STAR, "*", 3, 3 },
        { mt_TOKEN_SLASH, "/", 3, 4 },
    };

    return run_table_tests(tests, sizeof(tests) / sizeof(tests[0]), "()[]\n.,\n+-*/");
}

static char *test_scanner_can_scan_multi_character_tokens() {
    TableTest tests[] = {
        { mt_TOKEN_COLON, ":", 1, 1 },
        { mt_TOKEN_COLON_EQUAL, ":=", 1, 3 },
        { mt_TOKEN_BANG_EQUAL, "!=", 1, 6 },
        { mt_TOKEN_EQUAL, "=", 2, 1 },
        { mt_TOKEN_EQUAL_EQUAL, "==", 2, 3 },
        { mt_TOKEN_LESS, "<", 3, 1 },
        { mt_TOKEN_LESS_EQUAL, "<=", 3, 3 },
        { mt_TOKEN_GREATER, ">", 4, 1 },
        { mt_TOKEN_GREATER_EQUAL, ">=", 4, 3 },
        { mt_TOKEN_ERROR, "expected '=' after '!' but found '2'", 5, 1 },
    };

    return run_table_tests(tests, sizeof(tests) / sizeof(tests[0]), ": := !=\n= ==\n< <=\n> >=\n!2");
}

static char *test_scanner_can_scan_identifiers() {
    TableTest tests[] = {
        { mt_TOKEN_NAME, "a", 1, 1 },
        { mt_TOKEN_NAME, "b", 1, 3 },
        { mt_TOKEN_CAP_NAME, "Integer", 1, 5 },
        { mt_TOKEN_NAME, "current_token", 1, 13 },
        { mt_TOKEN_CAP_NAME, "Some_Class", 1, 27 },
        { mt_TOKEN_NAME, "___", 1, 38 },
    };

    return run_table_tests(tests, sizeof(tests) / sizeof(tests[0]), "a b Integer current_token Some_Class ___");
}

static char *test_scanner_can_scan_keywords() {
    TableTest tests[] = {
        { mt_TOKEN_PROTOCOL, "protocol", 1, 1 },
        { mt_TOKEN_RECORD, "record", 1, 10 },
        { mt_TOKEN_EXTEND, "extend", 1, 17 },
        { mt_TOKEN_DEF, "def", 1, 24 },
        { mt_TOKEN_IF, "if", 1, 28 },
        { mt_TOKEN_ELSE, "else", 1, 31 },
        { mt_TOKEN_FOR, "for", 1, 36 },
        { mt_TOKEN_WHILE, "while", 1, 40 },
        { mt_TOKEN_MATCH, "match", 1, 46 },
        { mt_TOKEN_END, "end", 1, 52 },
    };

    return run_table_tests(tests, sizeof(tests) / sizeof(tests[0]), "protocol record extend def if else for while match end");
}

static char *test_scanner_can_scan_strings() {
    TableTest tests[] = {
        { mt_TOKEN_STRING, "\"hello\"", 1, 1 },
        { mt_TOKEN_STRING, "\"\"", 1, 9 },
        { mt_TOKEN_STRING, "\"hello\\\"there\"", 1, 12 },
        { mt_TOKEN_ERROR, "unexpected end of file while parsing string literal", 1, 27 },
    };

    return run_table_tests(tests, sizeof(tests) / sizeof(tests[0]), "\"hello\" \"\" \"hello\\\"there\" \"never closed");
}

static char *test_scanner_can_scan_multiline_strings() {
    TableTest tests[] = {
        { mt_TOKEN_NAME, "print", 1, 1 },
        { mt_TOKEN_LPAREN, "(", 1, 6 },
        { mt_TOKEN_STRING, "\"testing\n  multiline\n  strings here,\n  get outta the way!!\"", 2, 3 },
        { mt_TOKEN_COMMA, ",", 5, 23 },
        { mt_TOKEN_NUMBER, "1", 5, 25 },
    };

    char *source = mt_read_entire_file("tests/fixtures/test_scanner_multiline_strings.mt");
    mu_assert("expected source to contain data", source);

    char *res = run_table_tests(tests, sizeof(tests) / sizeof(tests[0]), source);
    free(source);
    return res;
}

static char *test_scanner_can_scan_numbers() {
    TableTest tests[] = {
        { mt_TOKEN_NUMBER, "0", 1, 1 },
        { mt_TOKEN_NUMBER, "1234", 1, 3 },
        { mt_TOKEN_NUMBER, "12.05", 1, 8 },
        { mt_TOKEN_ERROR, "multiple points in number", 1, 14 },
        { mt_TOKEN_ERROR, "numbers cannot start with 0", 1, 19 },
    };

    return run_table_tests(tests, sizeof(tests) / sizeof(tests[0]), "0 1234 12.05 1..3 0123");
}

static char *test_scanner_can_scan_comments() {
    TableTest tests[] = {
        { mt_TOKEN_COLON, ":", 1, 1 },
        { mt_TOKEN_COMMENT, "# some comment", 1, 3 },
        { mt_TOKEN_COLON_EQUAL, ":=", 2, 1 },
        { mt_TOKEN_COMMENT, "# another comment", 2, 4 },
    };

    return run_table_tests(tests, sizeof(tests) / sizeof(tests[0]), ": # some comment\n:= # another comment");
}

static char *run_suite() {
    mu_run_test(test_scanner_can_scan_empty_buffers);
    mu_run_test(test_scanner_leaves_buffers_intact);
    mu_run_test(test_scanner_can_scan_single_character_tokens);
    mu_run_test(test_scanner_can_scan_multi_character_tokens);
    mu_run_test(test_scanner_can_scan_identifiers);
    mu_run_test(test_scanner_can_scan_keywords);
    mu_run_test(test_scanner_can_scan_strings);
    mu_run_test(test_scanner_can_scan_multiline_strings);
    mu_run_test(test_scanner_can_scan_numbers);
    mu_run_test(test_scanner_can_scan_comments);
    return 0;
}

int main(void) {
    scanner = malloc(sizeof(mt_Scanner));
    token = malloc(sizeof(mt_Token));
    mt_token_init(token);

    char *message = run_suite();
    if (message) {
        fprintf(stderr, "ERROR[%d]: %s\n", tests_run, message);
    } else {
        printf("%d/%d TESTS PASSED\n", tests_run, tests_run);
    }

    free(token);
    free(scanner);
    return 0;
}
