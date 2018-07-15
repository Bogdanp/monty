#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"

/// Node
/// ====

static void node_list_reverse(mt_NodeList **list) {
    mt_NodeList *head = *list;
    mt_NodeList *current = head;
    mt_NodeList *previous = NULL;
    while (current) {
        head = head->next;
        current->next = previous;
        previous = current;
        current = head;
    }

    *list = previous;
}

static mt_NodeList *node_list_cons(mt_NodeList *old_head, mt_Node *el) {
    mt_NodeList *head = malloc(sizeof(mt_NodeList));
    head->value = el;
    head->next = old_head;
    return head;
}

static void node_list_free(mt_NodeList *head) {
    mt_NodeList *current = head;
    while (head) {
        head = head->next;
        mt_node_free(current->value);
        free(current);
        current = head;
    }
}

mt_Node *mt_node_init(mt_NodeType type) {
    mt_Node *node = malloc(sizeof(mt_Node));
    if (!node) return NULL;

    node->type = type;
    node->value.as_node_list = NULL;
    return node;
}

void mt_node_dump(mt_Node *node, uint32_t depth) {
    mt_NodeList *head = NULL;

    for (uint32_t i = 0; i < depth; i++) {
        printf(" ");
    }

    switch (node->type) {
    case mt_NODE_NAME:   printf("NAME(%s)", node->value.as_string); break;
    case mt_NODE_STRING: printf("STRING(\"%s\")", node->value.as_string); break;

    case mt_NODE_MODULE:
        head = node->value.as_node_list;
        while (head) {
            mt_node_dump(head->value, depth);
            head = head->next;
        }
        break;
    }

    printf("\n");
}

void mt_node_free(mt_Node *node) {
    if (node->type == mt_NODE_MODULE)
        node_list_free(node->value.as_node_list);

    // TODO(bogdan): free value based on the node type.
    free(node);
}


/// Parser
/// ======

static mt_Token *advance(mt_Parser *parser) {
    mt_token_copy(parser->current_token, parser->previous_token);
    mt_scanner_scan(parser->scanner, parser->current_token);
    return parser->current_token;
}

static mt_Node *node_from_token(mt_Token *token) {
    mt_Node *node = NULL;

    switch (token->type) {
    case mt_TOKEN_CAP_NAME:
    case mt_TOKEN_NAME:
        node = mt_node_init(mt_NODE_NAME);
        node->value.as_string = malloc(sizeof(char) * token->length + 1);
        node->value.as_string[token->length] = 0;
        memcpy(node->value.as_string, token->start, token->length);
        return node;

    case mt_TOKEN_STRING:
        node = mt_node_init(mt_NODE_STRING);
        node->value.as_string = malloc(sizeof(char) * token->length - 1);
        node->value.as_string[token->length] = 0;
        memcpy(node->value.as_string, token->start + 1, token->length - 2);
        return node;

    default:
        return NULL;
    }
}

static mt_Node *parse_expression(mt_Parser *parser) {
    mt_Token *token = advance(parser);

    switch (token->type) {
    default:
        return node_from_token(parser->previous_token);
    }

    return NULL;
}

mt_Parser *mt_parser_init(char *filename, char *source) {
    mt_Parser *parser = malloc(sizeof(mt_Parser));
    if (!parser) return NULL;

    parser->filename = filename;
    parser->source = source;
    parser->scanner = NULL;
    parser->previous_token = NULL;
    parser->current_token = NULL;
    parser->tree = NULL;

    memset(parser->error, 0, PARSER_ERROR_LENGTH);
    parser->error_line = 0;
    parser->error_column = 0;

    parser->scanner = mt_scanner_init(source);
    if (!parser->scanner) goto fail;

    parser->previous_token = mt_token_init();
    if (!parser->previous_token) goto fail;

    parser->current_token = mt_token_init();
    if (!parser->current_token) goto fail;

    parser->tree = mt_node_init(mt_NODE_MODULE);
    if (!parser->tree) goto fail;

    return parser;

fail:
    mt_parser_free(parser);
    return NULL;
}

mt_Node *mt_parser_parse(mt_Parser *parser) {
    mt_Node *tree = parser->tree;
    mt_Node *node = NULL;

    advance(parser);
    while (true) {
        node = parse_expression(parser);
        if (!node) return NULL;

        tree->value.as_node_list = node_list_cons(tree->value.as_node_list, node);

        if (parser->current_token->type == mt_TOKEN_EOF) {
            break;
        }
    }

    node_list_reverse(&tree->value.as_node_list);
    return parser->tree;
}

void mt_parser_free(mt_Parser *parser) {
    if (parser->scanner) mt_scanner_free(parser->scanner);
    if (parser->previous_token) mt_token_free(parser->previous_token);
    if (parser->current_token) mt_token_free(parser->current_token);
    if (parser->tree) mt_node_free(parser->tree);

    free(parser);
}
