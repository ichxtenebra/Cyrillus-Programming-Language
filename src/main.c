#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOK_AZ,
    TOK_GLAGOL,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMI,
    TOK_STRING
} TokenType;

typedef struct {
    TokenType type;
    int start;
    int len;
} Token;

typedef struct {
    int start;
    int len;
} Statement;

static inline int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline int is_alnum(char c) {
    return is_alpha(c) || (c >= '0' && c <= '9');
}

static Token* tokenize(const char *src, int *count) {
    Token *tokens = malloc(sizeof(Token) * 256);
    int cap = 256;
    *count = 0;
    int pos = 0;
    int len = strlen(src);
    
    while (pos < len) {
        char c = src[pos];
        
        if (is_alpha(c)) {
            int start = pos;
            while (pos < len && is_alnum(src[pos])) pos++;
            int wlen = pos - start;
            
            TokenType type;
            if (wlen == 2 && src[start] == 'a' && src[start+1] == 'z') {
                type = TOK_AZ;
            } else if (wlen == 6 && memcmp(src + start, "glagol", 6) == 0) {
                type = TOK_GLAGOL;
            } else {
                continue;
            }
            
            if (*count >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(Token) * cap);
            }
            tokens[*count].type = type;
            tokens[*count].start = start;
            tokens[*count].len = wlen;
            (*count)++;
        }
        else if (c == '"') {
            pos++;
            int start = pos;
            while (pos < len && src[pos] != '"') pos++;
            int slen = pos - start;
            pos++;
            
            if (*count >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(Token) * cap);
            }
            tokens[*count].type = TOK_STRING;
            tokens[*count].start = start;
            tokens[*count].len = slen;
            (*count)++;
        }
        else if (c == '(') {
            if (*count >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(Token) * cap);
            }
            tokens[*count].type = TOK_LPAREN;
            tokens[*count].start = pos;
            tokens[*count].len = 1;
            (*count)++;
            pos++;
        }
        else if (c == ')') {
            if (*count >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(Token) * cap);
            }
            tokens[*count].type = TOK_RPAREN;
            tokens[*count].start = pos;
            tokens[*count].len = 1;
            (*count)++;
            pos++;
        }
        else if (c == '{') {
            if (*count >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(Token) * cap);
            }
            tokens[*count].type = TOK_LBRACE;
            tokens[*count].start = pos;
            tokens[*count].len = 1;
            (*count)++;
            pos++;
        }
        else if (c == '}') {
            if (*count >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(Token) * cap);
            }
            tokens[*count].type = TOK_RBRACE;
            tokens[*count].start = pos;
            tokens[*count].len = 1;
            (*count)++;
            pos++;
        }
        else if (c == ';') {
            if (*count >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(Token) * cap);
            }
            tokens[*count].type = TOK_SEMI;
            tokens[*count].start = pos;
            tokens[*count].len = 1;
            (*count)++;
            pos++;
        }
        else {
            pos++;
        }
    }
    
    return tokens;
}

static Statement* parse(Token *tokens, int tok_count, int *stmt_count) {
    Statement *stmts = malloc(sizeof(Statement) * 128);
    int cap = 128;
    *stmt_count = 0;
    int pos = 0;
    
    while (pos < tok_count) {
        if (tokens[pos].type == TOK_AZ) {
            pos += 3;
            while (pos < tok_count) {
                if (tokens[pos].type == TOK_RBRACE) {
                    pos++;
                    break;
                }
                if (tokens[pos].type == TOK_GLAGOL) {
                    pos += 2;
                    if (*stmt_count >= cap) {
                        cap *= 2;
                        stmts = realloc(stmts, sizeof(Statement) * cap);
                    }
                    stmts[*stmt_count].start = tokens[pos].start;
                    stmts[*stmt_count].len = tokens[pos].len;
                    (*stmt_count)++;
                    pos += 3;
                } else {
                    pos++;
                }
            }
        } else {
            pos++;
        }
    }
    
    return stmts;
}

static char* generate(const char *src, Statement *stmts, int stmt_count) {
    int cap = 4096;
    char *out = malloc(cap);
    int pos = 0;
    
    const char *header = "#include <stdio.h>\nint main() {\n";
    int hlen = strlen(header);
    memcpy(out, header, hlen);
    pos += hlen;
    
    for (int i = 0; i < stmt_count; i++) {
        const char *prefix = "    printf(\"";
        int plen = strlen(prefix);
        
        if (pos + plen + stmts[i].len + 10 >= cap) {
            cap *= 2;
            out = realloc(out, cap);
        }
        
        memcpy(out + pos, prefix, plen);
        pos += plen;
        
        memcpy(out + pos, src + stmts[i].start, stmts[i].len);
        pos += stmts[i].len;
        
        const char *suffix = "\");\n";
        int slen = strlen(suffix);
        memcpy(out + pos, suffix, slen);
        pos += slen;
    }
    
    const char *footer = "    return 0;\n}\n";
    int flen = strlen(footer);
    if (pos + flen >= cap) {
        cap *= 2;
        out = realloc(out, cap);
    }
    memcpy(out + pos, footer, flen);
    pos += flen;
    
    out[pos] = '\0';
    return out;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.cyr>\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *src = malloc(fsize + 1);
    fread(src, 1, fsize, f);
    src[fsize] = '\0';
    fclose(f);
    
    int tok_count;
    Token *tokens = tokenize(src, &tok_count);
    
    int stmt_count;
    Statement *stmts = parse(tokens, tok_count, &stmt_count);
    
    char *c_code = generate(src, stmts, stmt_count);
    
    f = fopen("output.c", "w");
    fprintf(f, "%s", c_code);
    fclose(f);
    
    system("gcc -O3 output.c -o output");
    remove("output.c");
    
    free(src);
    free(tokens);
    free(stmts);
    free(c_code);
    
    return 0;
}
