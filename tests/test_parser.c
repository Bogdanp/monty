#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "parser.h"

#include "minunit.h"

int tests_run = 0;
static mt_Parser *parser;
static mt_Node *tree;

static void teardown() {
    mt_parser_free(parser);
}

static char *test_parser_can_parse_empty_files() {
    parser = mt_parser_init("[stdin]", "");
    tree = mt_parser_parse(parser);
    mu_assert("expected a tree", tree);
    mu_assert("expected a MODULE node", tree->type == mt_NODE_MODULE);
    mu_assert("expected no children", tree->value.as_node_list == NULL);
    return 0;
}

static char *test_parser_can_parse_basic_expressions() {
    char *filename = "tests/fixtures/test_parser_basics.mt";
    char *source = mt_read_entire_file(filename);
    mu_assert("expected source to contain data", source);

    parser = mt_parser_init(filename, source);
    tree = mt_parser_parse(parser);
    mu_assert("expected a tree", tree);

    printf("\n\n");
    mt_node_dump(tree, 0);

    free(source);
    return 0;
}

static char *run_suite() {
    //mu_run_test(test_parser_can_parse_empty_files);
    mu_run_test(test_parser_can_parse_basic_expressions);
    return 0;
}

int main(void) {
    char *message = run_suite();
    if (message) {
        fprintf(stderr, "ERROR[%d]: %s\n", tests_run, message);
    } else {
        printf("%d/%d TESTS PASSED\n", tests_run, tests_run);
    }

    return 0;
}
