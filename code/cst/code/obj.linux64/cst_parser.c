/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20140715

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#define YYPREFIX "yy"

#define YYPURE 0

#line 20 "../../code/front_end/src/cst_parser.y"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <csf.h>

extern int32_t yylineno;

void yyerror(const char *str)
{
    fprintf(stderr,"error: line %d: %s\n",yylineno,str);
}

int32_t yywrap()
{
        return 1;
}

char buf[100];

#line 43 "../../code/front_end/src/cst_parser.y"
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
#line 63 "cst_parser.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

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
#define YYERRCODE 256
typedef short YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    8,    7,    7,    6,    6,    6,    6,    3,
    3,    2,    2,    5,    5,    4,    9,    9,   10,   11,
   11,   12,   12,    1,    1,    1,   13,   13,
};
static const YYINT yylen[] = {                            2,
    0,    2,    5,    0,    2,    4,    4,    4,    4,    1,
    3,    2,    3,    1,    3,    4,    1,    3,    1,    1,
    3,    1,    1,    1,    2,    2,    1,    2,
};
static const YYINT yydefred[] = {                         1,
    0,    0,    2,   24,    0,   26,   25,    0,   27,    0,
   28,    0,    0,    5,    0,    0,   23,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   13,    0,   11,
    0,   15,   19,   18,   21,   16,   12,    0,
};
static const YYINT yydgoto[] = {                          1,
   18,   19,   20,   21,   22,   14,   12,    3,   23,   24,
   25,   26,   10,
};
static const YYINT yysindex[] = {                         0,
 -254, -227,    0,    0, -246,    0,    0, -225,    0, -222,
    0, -227, -232,    0, -244, -250,    0, -224, -226, -225,
 -221, -225, -225, -220, -225, -219, -216, -215, -214, -222,
 -210, -222, -222, -209, -222, -223, -208,    0, -249,    0,
 -207,    0,    0,    0,    0,    0,    0, -216,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,
    0,    6,    0,    0,    0, -242,    0, -235, -213,    0,
 -206,    0,    0, -205,    0, -204, -233,    0,    0,    2,
    0,    3,    4,    0,    5,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yygindex[] = {                         0,
   -2,    0,    9,    0,   21,    0,    0,    0,   19,    0,
   18,    0,   -1,
};
#define YYTABLESIZE 269
static const YYINT yytable[] = {                          5,
    4,    6,    7,    8,    9,    3,   27,   47,    2,   13,
    6,    7,   16,    4,   17,   28,   28,    8,   30,   19,
   32,   33,   19,   35,    6,    7,   22,   15,   12,   22,
    4,   12,    6,    7,    4,   17,    9,   40,   29,   11,
   37,   38,   39,   31,   34,   36,   41,   43,   10,   48,
   46,   42,   44,   45,    0,   14,   17,   20,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    4,    6,
    7,    8,    9,    4,    6,    7,    8,    9,    3,
};
static const YYINT yycheck[] = {                          2,
    0,    0,    0,    0,    0,    0,  257,  257,  263,   12,
  257,  258,  257,  258,  259,  266,  266,  264,   20,  262,
   22,   23,  265,   25,  257,  258,  262,  260,  262,  265,
  258,  265,  257,  258,  258,  259,  262,   29,  265,  262,
  257,  257,  257,  265,  265,  265,  257,  257,  262,  257,
  259,   31,   34,   36,   -1,  262,  262,  262,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  258,  258,
  258,  258,  258,  263,  263,  263,  263,  263,  263,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 267
#define YYUNDFTOKEN 283
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"NUMBER","WORD","FILENAME",
"EQUALS","OR","EOL","LBRACK","RBRACK","COMMA","DOT","exp",0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : commands",
"commands :",
"commands : commands command",
"command : LBRACK label RBRACK eol arguments",
"arguments :",
"arguments : arguments argument",
"argument : label EQUALS pairs eol",
"argument : label EQUALS blocks eol",
"argument : label EQUALS numbers eol",
"argument : label EQUALS keywords eol",
"pairs : pair",
"pairs : pair COMMA pairs",
"pair : NUMBER NUMBER",
"pair : NUMBER DOT NUMBER",
"blocks : block",
"blocks : block COMMA blocks",
"block : NUMBER NUMBER NUMBER FILENAME",
"numbers : number",
"numbers : number COMMA numbers",
"number : NUMBER",
"keywords : keyword",
"keywords : keyword COMMA keywords",
"keyword : label",
"keyword : FILENAME",
"label : WORD",
"label : label WORD",
"label : label NUMBER",
"eol : EOL",
"eol : eol EOL",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 3:
#line 82 "../../code/front_end/src/cst_parser.y"
	{
            /* command record allocated when parsing arguments */
            yyval.command = yystack.l_mark[0].command;

            /* save the head */
            if(g_cmd_head == NULL)
            {
                g_cmd_head = yyval.command;
            }

            /* maintain the cmd list using g_cmd_current */
            if(g_cmd_current == NULL)
            {
                g_cmd_current = yyval.command;
                g_cmd_current->next = NULL;
            }
            else
            {
                g_cmd_current->next = yyval.command;
                g_cmd_current = g_cmd_current->next;
                g_cmd_current->next = NULL;
            }

            yyval.command->name = yystack.l_mark[-3].str;              /* add name */
            yyval.command->start_offset_cert_sig = 0;
            yyval.command->size_cert_sig = 0;
            yyval.command->cert_sig_data = NULL;
            if ((g_error_code = handle_command(yyval.command)) < SUCCESS) YYERROR;
        }
break;
case 4:
#line 115 "../../code/front_end/src/cst_parser.y"
	{
            yyval.command = malloc(sizeof(command_t));
            yyval.command->argument_count = 0;
            yyval.command->argument = NULL;
        }
break;
case 5:
#line 121 "../../code/front_end/src/cst_parser.y"
	{
            yyval.command = yystack.l_mark[-1].command;
            yyval.command->argument_count++;
            yystack.l_mark[0].argument->next = yyval.command->argument;    /* insert new argument at head */
            yyval.command->argument = yystack.l_mark[0].argument;
        }
break;
case 6:
#line 131 "../../code/front_end/src/cst_parser.y"
	{
            /* argument record allocated when parsing pairs */
            yyval.argument = yystack.l_mark[-1].argument;
            yyval.argument->name = yystack.l_mark[-3].str;               /* add name */
            if((g_error_code = set_argument_type(yyval.argument)) != SUCCESS) YYERROR;
        }
break;
case 7:
#line 138 "../../code/front_end/src/cst_parser.y"
	{
            /* argument record allocated when parsing blocks */
            yyval.argument = yystack.l_mark[-1].argument;
            yyval.argument->name = yystack.l_mark[-3].str;               /* add name */
            if((g_error_code = set_argument_type(yyval.argument)) != SUCCESS) YYERROR;
        }
break;
case 8:
#line 145 "../../code/front_end/src/cst_parser.y"
	{
            /* argument record allocated when parsing numbers */
            yyval.argument = yystack.l_mark[-1].argument;
            yyval.argument->name = yystack.l_mark[-3].str;               /* add name */
            if((g_error_code = set_argument_type(yyval.argument)) != SUCCESS) YYERROR;
        }
break;
case 9:
#line 152 "../../code/front_end/src/cst_parser.y"
	{
            /* argument record allocated when parsing numbers */
            yyval.argument = yystack.l_mark[-1].argument;
            yyval.argument->name = yystack.l_mark[-3].str;               /* add name */
            if((g_error_code = set_argument_type(yyval.argument)) != SUCCESS) YYERROR;
        }
break;
case 10:
#line 159 "../../code/front_end/src/cst_parser.y"
	{
            yyval.argument = malloc(sizeof(argument_t));
            yyval.argument->next = NULL;
            yyval.argument->value_count = 1;
            yyval.argument->value_type = PAIR_TYPE;
            yyval.argument->value.pair = yystack.l_mark[0].pair;
        }
break;
case 11:
#line 168 "../../code/front_end/src/cst_parser.y"
	{
            yyval.argument = yystack.l_mark[0].argument;
            yyval.argument->value_count++;
            yystack.l_mark[-2].pair->next = yyval.argument->value.pair;    /* insert new pair at head */
            yyval.argument->value.pair = yystack.l_mark[-2].pair;
        }
break;
case 12:
#line 177 "../../code/front_end/src/cst_parser.y"
	{
            yyval.pair = malloc(sizeof(pair_t));
            yyval.pair->next = NULL;
            yyval.pair->first = yystack.l_mark[-1].num;
            yyval.pair->second = yystack.l_mark[0].num;
        }
break;
case 13:
#line 184 "../../code/front_end/src/cst_parser.y"
	{
            yyval.pair = malloc(sizeof(pair_t));
            yyval.pair->next = NULL;
            yyval.pair->first = yystack.l_mark[-2].num;
            yyval.pair->second = yystack.l_mark[0].num;
        }
break;
case 14:
#line 193 "../../code/front_end/src/cst_parser.y"
	{
            yyval.argument = malloc(sizeof(argument_t));
            yyval.argument->next = NULL;
            yyval.argument->value_count = 1;
            yyval.argument->value_type = BLOCK_TYPE;
            yyval.argument->value.block = yystack.l_mark[0].block;
        }
break;
case 15:
#line 202 "../../code/front_end/src/cst_parser.y"
	{
            yyval.argument = yystack.l_mark[0].argument;
            yyval.argument->value_count++;
            yystack.l_mark[-2].block->next = yyval.argument->value.block;    /* insert new block at head */
            yyval.argument->value.block = yystack.l_mark[-2].block;
        }
break;
case 16:
#line 211 "../../code/front_end/src/cst_parser.y"
	{
            yyval.block = malloc(sizeof(block_t));
            yyval.block->next = NULL;
            yyval.block->base_address = yystack.l_mark[-3].num;
            yyval.block->start = yystack.l_mark[-2].num;
            yyval.block->length = yystack.l_mark[-1].num;
            yyval.block->block_filename = yystack.l_mark[0].str;
        }
break;
case 17:
#line 222 "../../code/front_end/src/cst_parser.y"
	{
            yyval.argument = malloc(sizeof(argument_t));
            yyval.argument->next = NULL;
            yyval.argument->value_count = 1;
            yyval.argument->value_type = NUMBER_TYPE;
            yyval.argument->value.number = yystack.l_mark[0].number;
        }
break;
case 18:
#line 231 "../../code/front_end/src/cst_parser.y"
	{
            yyval.argument = yystack.l_mark[0].argument;
            yyval.argument->value_count++;
            yystack.l_mark[-2].number->next = yyval.argument->value.number;    /* insert new block at head */
            yyval.argument->value.number = yystack.l_mark[-2].number;
        }
break;
case 19:
#line 240 "../../code/front_end/src/cst_parser.y"
	{
            yyval.number = malloc(sizeof(number_t));
            yyval.number->next = NULL;
            yyval.number->num_value = yystack.l_mark[0].num;
        }
break;
case 20:
#line 248 "../../code/front_end/src/cst_parser.y"
	{
            yyval.argument = malloc(sizeof(argument_t));
            yyval.argument->next = NULL;
            yyval.argument->value_count = 1;
            yyval.argument->value_type = KEYWORD_TYPE;
            yyval.argument->value.keyword = yystack.l_mark[0].keyword;
        }
break;
case 21:
#line 257 "../../code/front_end/src/cst_parser.y"
	{
            yyval.argument = yystack.l_mark[0].argument;
            yyval.argument->value_count++;
            yystack.l_mark[-2].keyword->next = yyval.argument->value.keyword;    /* insert new block at head */
            yyval.argument->value.keyword = yystack.l_mark[-2].keyword;
        }
break;
case 22:
#line 266 "../../code/front_end/src/cst_parser.y"
	{
            yyval.keyword = malloc(sizeof(keyword_t));
            yyval.keyword->next = NULL;
            yyval.keyword->string_value = yystack.l_mark[0].str;
            if((g_error_code = set_label(yyval.keyword)) != SUCCESS) YYERROR;
        }
break;
case 23:
#line 273 "../../code/front_end/src/cst_parser.y"
	{
            yyval.keyword = malloc(sizeof(keyword_t));
            yyval.keyword->next = NULL;
            yyval.keyword->string_value = yystack.l_mark[0].str;
            yyval.keyword->unsigned_value = 0xFFFFFFFF;
        }
break;
case 24:
#line 282 "../../code/front_end/src/cst_parser.y"
	{
            yyval.str = yystack.l_mark[0].str;
        }
break;
case 25:
#line 286 "../../code/front_end/src/cst_parser.y"
	{
            yyval.str = realloc(yyval.str, strlen(yyval.str) + strlen(yystack.l_mark[0].str) + 1);
            yyval.str = strcat(yyval.str,yystack.l_mark[0].str);
        }
break;
case 26:
#line 291 "../../code/front_end/src/cst_parser.y"
	{
	    /* Concat label with number into buf */
	    sprintf(buf, "%s%d", yyval.str, yystack.l_mark[0].num);
	    /* Return buf to top of stack */
	    yyval.str = realloc(yyval.str, strlen(buf) + 1);
            yyval.str = buf;
        }
break;
#line 734 "cst_parser.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                yys = yyname[YYTRANSLATE(yychar)];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
