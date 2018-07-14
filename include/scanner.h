#ifndef mt_scanner_h
#define mt_scanner_h

#include <stdint.h>
#include <stdlib.h>

typedef enum {
    mt_TOKEN_EOF,
    mt_TOKEN_ERROR,

    mt_TOKEN_DOT,
    mt_TOKEN_COMMA,
    mt_TOKEN_LPAREN,
    mt_TOKEN_RPAREN,
    mt_TOKEN_LBRACKET,
    mt_TOKEN_RBRACKET,
    mt_TOKEN_PLUS,
    mt_TOKEN_MINUS,
    mt_TOKEN_STAR,
    mt_TOKEN_SLASH,

    mt_TOKEN_EQUAL,
    mt_TOKEN_EQUAL_EQUAL,
    mt_TOKEN_BANG_EQUAL,
    mt_TOKEN_LESS,
    mt_TOKEN_LESS_EQUAL,
    mt_TOKEN_GREATER,
    mt_TOKEN_GREATER_EQUAL,
    mt_TOKEN_COLON,
    mt_TOKEN_COLON_EQUAL,

    mt_TOKEN_CAP_NAME,
    mt_TOKEN_NAME,
    mt_TOKEN_STRING,
    mt_TOKEN_NUMBER,

    mt_TOKEN_AND,
    mt_TOKEN_DEF,
    mt_TOKEN_ELSE,
    mt_TOKEN_END,
    mt_TOKEN_EXTEND,
    mt_TOKEN_FOR,
    mt_TOKEN_IF,
    mt_TOKEN_MATCH,
    mt_TOKEN_NOT,
    mt_TOKEN_OR,
    mt_TOKEN_PROTOCOL,
    mt_TOKEN_RECORD,
    mt_TOKEN_WHILE,

    mt_TOKEN_COMMENT,
} mt_TokenType;


typedef struct {
    mt_TokenType type;

    char *start;
    size_t length;

    uint32_t line;
    uint32_t column;
} mt_Token;

/** Print a token to a buffer. */
void mt_token_debug(mt_Token *, char *, size_t);


/**
 * Scanners are views on top of character buffers that yield tokens.
 */
typedef struct {
    char *start;
    char *current;

    uint32_t line;
    uint32_t column;
} mt_Scanner;


void mt_scanner_init(mt_Scanner *, char *);
void mt_scanner_scan(mt_Scanner *, mt_Token *);

#endif
