#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "utils.h"

static char *TOKEN_DEBUG_NAMES[] = {
    "TOKEN_EOF",
    "TOKEN_ERROR",
    "TOKEN_COMMENT",

    "TOKEN_DOT",
    "TOKEN_COMMA",
    "TOKEN_LPAREN",
    "TOKEN_RPAREN",
    "TOKEN_LBRACKET",
    "TOKEN_RBRACKET",
    "TOKEN_PLUS",
    "TOKEN_MINUS",
    "TOKEN_STAR",
    "TOKEN_SLASH",

    "TOKEN_EQUAL",
    "TOKEN_EQUAL_EQUAL",
    "TOKEN_BANG_EQUAL",
    "TOKEN_LESS",
    "TOKEN_LESS_EQUAL",
    "TOKEN_GREATER",
    "TOKEN_GREATER_EQUAL",
    "TOKEN_COLON",
    "TOKEN_COLON_EQUAL",

    "TOKEN_CAP_NAME",
    "TOKEN_NAME",
    "TOKEN_STRING",
    "TOKEN_NUMBER",

    "TOKEN_AND",
    "TOKEN_DEF",
    "TOKEN_ELSE",
    "TOKEN_END",
    "TOKEN_EXTEND",
    "TOKEN_FOR",
    "TOKEN_IF",
    "TOKEN_MATCH",
    "TOKEN_NOT",
    "TOKEN_OR",
    "TOKEN_PROTOCOL",
    "TOKEN_RECORD",
    "TOKEN_WHILE",
};

void mt_token_debug(mt_Token *token, char *buf, size_t bufsz) {
    char value[255] = "";
    memcpy(value, token->start, MIN(token->length, sizeof(value)));

    snprintf(
        buf,
        bufsz,
        "Token(type='%s', value='%s', line=%d, column=%d)",
        TOKEN_DEBUG_NAMES[token->type],
        value,
        token->line,
        token->column
    );
}

static void load_token(mt_Scanner *scanner, mt_Token *token, mt_TokenType type) {
    token->type = type;
    token->start = scanner->start;
    token->length = (size_t)(scanner->current - scanner->start);
    token->line = scanner->line;
    token->column = scanner->column;
}

static void fail_token(mt_Scanner *scanner, mt_Token *token, char *message) {
    token->type = mt_TOKEN_ERROR;
    token->start = message;
    token->length = strlen(message);
    token->line = scanner->line;
    token->column = scanner->column;
}


static char advance(mt_Scanner *scanner) {
    char c;

    do  {
        c = *scanner->current;
        scanner->start = scanner->current;
        scanner->current += 1;
        scanner->column += 1;

        if (c == '\n') {
            scanner->line += 1;
            scanner->column = 0;
        }
    } while (c == ' ' || c == '\t' || c == '\r' || c == '\n');

    return c;
}

static char peek(mt_Scanner *scanner) {
    return *scanner->current;
}

static bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

static bool is_lo_alpha(char c) {
    return 'a' <= c && c <= 'z';
}

static bool is_hi_alpha(char c) {
    return 'A' <= c && c <= 'Z';
}

static bool is_alpha(char c) {
    return is_lo_alpha(c) || is_hi_alpha(c);
}

static bool match_keyword(mt_Scanner *scanner, const char *keyword) {
    size_t length = scanner->current - scanner->start;
    return length == strlen(keyword) && memcmp(scanner->start, keyword, length) == 0;
}

static void load_comment(mt_Scanner *scanner, mt_Token *token) {
    while (*scanner->current != '\n' && *scanner->current != '\0') {
        scanner->current += 1;
    }

    load_token(scanner, token, mt_TOKEN_COMMENT);
}

static void load_name(mt_Scanner *scanner, mt_Token *token) {
    while (is_alpha(*scanner->current) || is_digit(*scanner->current) || *scanner->current == '_') {
        scanner->current += 1;
    }

    if (match_keyword(scanner, "and"))           load_token(scanner, token, mt_TOKEN_AND);
    else if (match_keyword(scanner, "def"))      load_token(scanner, token, mt_TOKEN_DEF);
    else if (match_keyword(scanner, "else"))     load_token(scanner, token, mt_TOKEN_ELSE);
    else if (match_keyword(scanner, "end"))      load_token(scanner, token, mt_TOKEN_END);
    else if (match_keyword(scanner, "extend"))   load_token(scanner, token, mt_TOKEN_EXTEND);
    else if (match_keyword(scanner, "for"))      load_token(scanner, token, mt_TOKEN_FOR);
    else if (match_keyword(scanner, "if"))       load_token(scanner, token, mt_TOKEN_IF);
    else if (match_keyword(scanner, "match"))    load_token(scanner, token, mt_TOKEN_MATCH);
    else if (match_keyword(scanner, "not"))      load_token(scanner, token, mt_TOKEN_NOT);
    else if (match_keyword(scanner, "or"))       load_token(scanner, token, mt_TOKEN_OR);
    else if (match_keyword(scanner, "protocol")) load_token(scanner, token, mt_TOKEN_PROTOCOL);
    else if (match_keyword(scanner, "record"))   load_token(scanner, token, mt_TOKEN_RECORD);
    else if (match_keyword(scanner, "while"))    load_token(scanner, token, mt_TOKEN_WHILE);
    else load_token(scanner, token, mt_TOKEN_NAME);

    scanner->column += scanner->current - scanner->start - 1;
}

static void load_cap_name(mt_Scanner *scanner, mt_Token *token) {
    while (is_alpha(*scanner->current) || is_digit(*scanner->current) || *scanner->current == '_') {
        scanner->current += 1;
    }

    load_token(scanner, token, mt_TOKEN_CAP_NAME);
    scanner->column += scanner->current - scanner->start - 1;
}

static void load_number(mt_Scanner *scanner, mt_Token *token) {
    char *error;
    bool failed = false;
    bool point_found = false;
    while (is_digit(*scanner->current) || *scanner->current == '.') {
        if (*scanner->current == '.') {
            if (point_found) {
                failed = true;
                error = "multiple points in number";
            }

            point_found = true;
        }

        scanner->current += 1;
    }

    if (*scanner->start == '0' && scanner->current - scanner->start > 1) {
        failed = true;
        error = "numbers cannot start with 0";
    }

    if (failed) {
        fail_token(scanner, token, error);
    } else {
        load_token(scanner, token, mt_TOKEN_NUMBER);
    }

    scanner->column += scanner->current - scanner->start - 1;
}

static void load_string(mt_Scanner *scanner, mt_Token *token) {
    char pc;
    bool failed = false;
    while (*scanner->current != '"' || pc == '\\') {
        if (*scanner->current == '\0') {
            failed = true;
            break;
        }

        pc = *scanner->current;
        scanner->current += 1;
    }

    if (failed) {
        fail_token(scanner, token, "unexpected end of file while parsing string literal");
        return;
    }

    scanner->current += 1;

    load_token(scanner, token, mt_TOKEN_STRING);
    scanner->column += scanner->current - scanner->start - 1;
}

void mt_scanner_init(mt_Scanner *scanner, char *buffer) {
    scanner->start = buffer;
    scanner->current = buffer;
    scanner->line = 1;
    scanner->column = 0;
}

void mt_scanner_scan(mt_Scanner *scanner, mt_Token *token) {
    char c = advance(scanner);

    if (c == '_' || is_lo_alpha(c)) {
        load_name(scanner, token);
        return;
    }

    if (is_hi_alpha(c)) {
        load_cap_name(scanner, token);
        return;
    }

    if (is_digit(c)) {
        load_number(scanner, token);
        return;
    }

    switch (c) {
    case '\0': load_token(scanner, token, mt_TOKEN_EOF); break;

    case '.': load_token(scanner, token, mt_TOKEN_DOT); break;
    case ',': load_token(scanner, token, mt_TOKEN_COMMA); break;
    case '(': load_token(scanner, token, mt_TOKEN_LPAREN); break;
    case ')': load_token(scanner, token, mt_TOKEN_RPAREN); break;
    case '[': load_token(scanner, token, mt_TOKEN_LBRACKET); break;
    case ']': load_token(scanner, token, mt_TOKEN_RBRACKET); break;
    case '+': load_token(scanner, token, mt_TOKEN_PLUS); break;
    case '-': load_token(scanner, token, mt_TOKEN_MINUS); break;
    case '*': load_token(scanner, token, mt_TOKEN_STAR); break;
    case '/': load_token(scanner, token, mt_TOKEN_SLASH); break;

    case '=':
        if (peek(scanner) == '=') {
            load_token(scanner, token, mt_TOKEN_EQUAL_EQUAL);
            advance(scanner);
        } else {
            load_token(scanner, token, mt_TOKEN_EQUAL);
        }

        break;

    case '!':
        if (peek(scanner) == '=') {
            load_token(scanner, token, mt_TOKEN_BANG_EQUAL);
            advance(scanner);
        } else {
            snprintf(scanner->error, sizeof(scanner->error) / sizeof(char), "expected '=' after '!' but found '%c'", peek(scanner));
            fail_token(scanner, token, scanner->error);
            advance(scanner);
        }

        break;

    case '<':
        if (peek(scanner) == '=') {
            load_token(scanner, token, mt_TOKEN_LESS_EQUAL);
            advance(scanner);
        } else {
            load_token(scanner, token, mt_TOKEN_LESS);
        }

        break;

    case '>':
        if (peek(scanner) == '=') {
            load_token(scanner, token, mt_TOKEN_GREATER_EQUAL);
            advance(scanner);
        } else {
            load_token(scanner, token, mt_TOKEN_GREATER);
        }

        break;

    case ':':
        if (peek(scanner) == '=') {
            load_token(scanner, token, mt_TOKEN_COLON_EQUAL);
            advance(scanner);
        } else {
            load_token(scanner, token, mt_TOKEN_COLON);
        }

        break;

    case '"': load_string(scanner, token); break;
    case '#': load_comment(scanner, token); break;

    default:
        snprintf(scanner->error, sizeof(scanner->error) / sizeof(char), "unexpected token '%c'", c);
        fail_token(scanner, token, scanner->error);
        break;
    }
}
