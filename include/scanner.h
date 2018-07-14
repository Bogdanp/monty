#ifndef mt_scanner_h
#define mt_scanner_h

#include <stdint.h>
#include <stdlib.h>

typedef enum {
    // Meta
    mt_TOKEN_EOF,
    mt_TOKEN_ERROR,
    mt_TOKEN_COMMENT,

    // Single-character tokens
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

    // Single and double-character tokens
    mt_TOKEN_EQUAL,
    mt_TOKEN_EQUAL_EQUAL,
    mt_TOKEN_BANG_EQUAL,
    mt_TOKEN_LESS,
    mt_TOKEN_LESS_EQUAL,
    mt_TOKEN_GREATER,
    mt_TOKEN_GREATER_EQUAL,
    mt_TOKEN_COLON,
    mt_TOKEN_COLON_EQUAL,

    // Literals
    mt_TOKEN_CAP_NAME,
    mt_TOKEN_NAME,
    mt_TOKEN_STRING,
    mt_TOKEN_NUMBER,

    // Keywords -- keep these sorted
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
} mt_TokenType;


/// Tokens contain positional information along with a type.
typedef struct {
    mt_TokenType type;

    char *start;  ///< the start position in the source buffer for this token's value
    size_t length;  ///< the length of this token's value

    uint32_t line;
    uint32_t column;
} mt_Token;

/// Stringify a token into a buffer for debugging.
void mt_token_debug(mt_Token *, char *, size_t);


/// Scanners are views on top of character buffers that yield tokens.
typedef struct {
    char error[255];  ///< holds the error message of the last error token

    char *start;  ///< the start position of the current token
    char *current;  ///< the end position of the current token

    uint32_t line;  ///< the current line number (1 indexed)
    uint32_t column; ///< the current column number (1 indexed)
} mt_Scanner;

/// Initialize a scanner's internals.  The scanner must be allocated
/// and freed by the user.
void mt_scanner_init(mt_Scanner *, char *);

/// Extract the next token from a scanner.
void mt_scanner_scan(mt_Scanner *, mt_Token *);

#endif
