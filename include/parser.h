#ifndef mt_parser_h
#define mt_parser_h

#include <stdio.h>
#include <stdint.h>

#include "scanner.h"

typedef enum {
    mt_NODE_MODULE,
    mt_NODE_STRING,
    mt_NODE_TYPE,
    mt_NODE_NAME,
} mt_NodeType;

struct Node;

typedef struct NodeList {
    struct Node *value;
    struct NodeList *next;
} mt_NodeList;

typedef union {
    double as_double;
    int64_t as_integer;
    char *as_string;
    mt_NodeList *as_node_list;
} mt_NodeValue;

typedef struct Node {
    mt_NodeType type;
    mt_NodeValue value;

    uint32_t line;
    uint32_t column;
} mt_Node;

/// Create a Node.  Returns NULL if there isn't enough free memory.
mt_Node *mt_node_init(mt_NodeType type, uint32_t, uint32_t);

/// Dump a Node to a stream.
void mt_node_dump(mt_Node *, FILE *);

/// Free a Node.
void mt_node_free(mt_Node *);

#define PARSER_ERROR_LENGTH 1024

/// Parsers turn source code into ASTs.
typedef struct {
    char *filename;  ///< the name of the file being parsed -- doesn't have to point to a real file as it's only used for error reporting
    char *source;   ///< the source code to parse, this data must outlive the parser

    mt_Scanner *scanner;  ///< the Scanner/lexer
    mt_Token *previous_token;
    mt_Token *current_token;
    mt_Node *tree;  ///< the root AST node

    char error[PARSER_ERROR_LENGTH];
    uint32_t error_line;
    uint32_t error_column;
} mt_Parser;

/// Create a Parser.  It is up to the caller to manage filename and
/// source, but they must outlive the parser.  Returns NULL if there
/// isn't enough memory.
mt_Parser *mt_parser_init(char *filename, char *source);

/// Perform a parse on the Parser's input.  Returns an AST on success.
///
/// When the return value is NULL, the "error", "error_line" and
/// "error_column" fields will be populated with information about the
/// error.
///
/// The returned value will be freed when you call "mt_parser_free".
mt_Node *mt_parser_parse(mt_Parser *);

/// Free a Parser.
void mt_parser_free(mt_Parser *);

#endif
