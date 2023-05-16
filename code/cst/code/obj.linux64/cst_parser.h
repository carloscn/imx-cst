#define NUMBER 257
#define WORD 258
#define FILENAME 259
#define EQUALS 260
#define OR 261
#define EOL 262
#define LBRACK 263
#define RBRACK 264
#define COMMA 265
#define DOT 266
#define exp 267
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union
{
    char *str;
    uint32_t  num;
    command_t *command;
    argument_t *argument;
    value_t value;
    pair_t *pair;
    block_t *block;
    number_t *number;
    keyword_t *keyword;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;
