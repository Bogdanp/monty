#include "scanner.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) ((a < b) ? a : b)

static char *TOKEN_DEBUG_NAMES[] = {
    "mt_TOKEN_EOF",
    "mt_TOKEN_ERROR",

    "mt_TOKEN_DOT",
    "mt_TOKEN_COMMA",
    "mt_TOKEN_LPAREN",
    "mt_TOKEN_RPAREN",
    "mt_TOKEN_LBRACKET",
    "mt_TOKEN_RBRACKET",
    "mt_TOKEN_PLUS",
    "mt_TOKEN_MINUS",
    "mt_TOKEN_STAR",
    "mt_TOKEN_SLASH",

    "mt_TOKEN_EQUAL",
    "mt_TOKEN_EQUAL_EQUAL",
    "mt_TOKEN_BANG_EQUAL",
    "mt_TOKEN_LESS",
    "mt_TOKEN_LESS_EQUAL",
    "mt_TOKEN_GREATER",
    "mt_TOKEN_GREATER_EQUAL",
    "mt_TOKEN_COLON",
    "mt_TOKEN_COLON_EQUAL",

    "mt_TOKEN_CAP_NAME",
    "mt_TOKEN_NAME",
    "mt_TOKEN_STRING",
    "mt_TOKEN_NUMBER",

    "mt_TOKEN_AND",
    "mt_TOKEN_DEF",
    "mt_TOKEN_ELSE",
    "mt_TOKEN_END",
    "mt_TOKEN_EXTEND",
    "mt_TOKEN_FOR",
    "mt_TOKEN_IF",
    "mt_TOKEN_MATCH",
    "mt_TOKEN_NOT",
    "mt_TOKEN_OR",
    "mt_TOKEN_PROTOCOL",
    "mt_TOKEN_RECORD",
    "mt_TOKEN_WHILE",

    "mt_TOKEN_COMMENT",
};

void mt_token_debug(mt_Token *token, char *buf, size_t bufsz) {
    char value[255] = "";
    memcpy(value, token->start, token->length);

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

static void error_token(mt_Scanner *scanner, mt_Token *token, char *message) {
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

static bool is_low_alpha(char c) {
    return 'a' <= c && c <= 'z';
}

static bool is_high_alpha(char c) {
    return 'A' <= c && c <= 'Z';
}

static bool is_alpha(char c) {
    return is_low_alpha(c) || is_high_alpha(c);
}

static bool is_keyword(mt_Scanner *scanner, const char *keyword) {
    size_t length = scanner->current - scanner->start;
    if (length != strlen(keyword)) {
        return false;
    }

    return memcmp(scanner->start, keyword, length) == 0;
}

static void chomp_comment(mt_Scanner *scanner, mt_Token *token) {
    while (*scanner->current != '\n' && *scanner->current != '\0') {
        scanner->current += 1;
    }

    load_token(scanner, token, mt_TOKEN_COMMENT);
}

static void chomp_cap_name(mt_Scanner *scanner, mt_Token *token) {
    while (is_alpha(*scanner->current) || is_digit(*scanner->current) || *scanner->current == '_') {
        scanner->current += 1;
    }

    load_token(scanner, token, mt_TOKEN_CAP_NAME);
    scanner->column += scanner->current - scanner->start - 1;
}

static void chomp_name(mt_Scanner *scanner, mt_Token *token) {
    while (is_alpha(*scanner->current) || is_digit(*scanner->current) || *scanner->current == '_') {
        scanner->current += 1;
    }

    if (is_keyword(scanner, "and")) load_token(scanner, token, mt_TOKEN_AND);
    else if (is_keyword(scanner, "def")) load_token(scanner, token, mt_TOKEN_DEF);
    else if (is_keyword(scanner, "else")) load_token(scanner, token, mt_TOKEN_ELSE);
    else if (is_keyword(scanner, "end")) load_token(scanner, token, mt_TOKEN_END);
    else if (is_keyword(scanner, "extend")) load_token(scanner, token, mt_TOKEN_EXTEND);
    else if (is_keyword(scanner, "for")) load_token(scanner, token, mt_TOKEN_FOR);
    else if (is_keyword(scanner, "if")) load_token(scanner, token, mt_TOKEN_IF);
    else if (is_keyword(scanner, "match")) load_token(scanner, token, mt_TOKEN_MATCH);
    else if (is_keyword(scanner, "not")) load_token(scanner, token, mt_TOKEN_NOT);
    else if (is_keyword(scanner, "or")) load_token(scanner, token, mt_TOKEN_OR);
    else if (is_keyword(scanner, "protocol")) load_token(scanner, token, mt_TOKEN_PROTOCOL);
    else if (is_keyword(scanner, "record")) load_token(scanner, token, mt_TOKEN_RECORD);
    else if (is_keyword(scanner, "while")) load_token(scanner, token, mt_TOKEN_WHILE);
    else load_token(scanner, token, mt_TOKEN_NAME);

    scanner->column += scanner->current - scanner->start - 1;
}

static void chomp_number(mt_Scanner *scanner, mt_Token *token) {
    char *error;
    bool failed = false;
    bool point_found = false;
    while (is_digit(*scanner->current) || *scanner->current == '.') {
        if (*scanner->current == '.') {
            if (point_found) {
                failed = true;
                error = "multiple periods in number";
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
        error_token(scanner, token, error);
    } else {
        load_token(scanner, token, mt_TOKEN_NUMBER);
    }

    scanner->column += scanner->current - scanner->start - 1;
}

static void chomp_string(mt_Scanner *scanner, mt_Token *token) {
    char pc;
    while (*scanner->current != '"' || pc == '\\') {
        pc = *scanner->current;
        scanner->current += 1;
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
    static char buf[255];
    char c = advance(scanner);

    if (c == '_' || is_low_alpha(c)) {
        chomp_name(scanner, token);
        return;
    }

    if (is_high_alpha(c)) {
        chomp_cap_name(scanner, token);
        return;
    }

    if (is_digit(c)) {
        chomp_number(scanner, token);
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
            snprintf(buf, 255, "expected '=' after '!' but found '%c'", peek(scanner));
            error_token(scanner, token, buf);
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

    case '"': chomp_string(scanner, token); break;
    case '#': chomp_comment(scanner, token); break;

    default:
        snprintf(buf, 255, "unexpected token '%c'", c);
        error_token(scanner, token, buf);
        break;
    }
}
