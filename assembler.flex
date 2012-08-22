/*
flasm, command line assembler & disassembler of flash actionscript bytecode
Copyright (c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan
All rights reserved. See LICENSE.TXT for terms of use.
*/

%option nounput
%option warn
%{

#include <math.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "flasm.h"
#include "assembler.tab.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

int yyEOF = 0;

char *lexBuffer = NULL;
int lexBufferLen = 0;

#define MAX_INCLUDE_DEPTH 10
#define MAX_LINE_LENGTH 65535

extern char *inputName;

YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
static int include_stack_ptr = 0;
static char *includenames[MAX_INCLUDE_DEPTH];
static int  sLineNumber[MAX_INCLUDE_DEPTH];
static int  column[MAX_INCLUDE_DEPTH];
static char szLine[MAX_LINE_LENGTH];


extern int yylex(void);
int  yywrap(void);
void yyerror(char *msg);
void warning(char *msg);
char *FileName(void);
int LineNumber(void);
int ColumnNumber(void);

inline void count(void);
inline void newLine(void);

static char sbuf[MAX_LINE_LENGTH];
static char *s;

int numActions = 0;

%}

DIGIT    [0-9]
LABELD   [a-zA-Z_][a-zA-Z0-9_]*:
ID       [a-zA-Z_][a-zA-Z0-9_]*
%x MOVIEDECL
%x STRINGSINGLEQ
%x STRINGDOUBLEQ
%x INCL
%x BLOCKCOMMENT

%%

0x[0-9a-fA-F]+                  {
                                    count();
                                    yylval.str = mstrdup(yytext+2);
                                    return HEX;
                                }

-{DIGIT}+                       {
                                    count();
                                    yylval.num = atol(yytext);
                                    return INTEGER;
                                }

{DIGIT}+                        {
                                    count();
                                    yylval.num = atol(yytext);
                                    return INTEGER;
                                }

-?{DIGIT}+"."{DIGIT}*(e("+"|"-"){DIGIT}{1,3})?f {
                                    count();
                                    yylval.str = mstrdup(yytext);
                                    return FLOAT;
                                }

-?{DIGIT}+"."{DIGIT}*(e("+"|"-"){DIGIT}{1,3})? {
                                    count();
                                    yylval.str = mstrdup(yytext);
                                    return DOUBLE;
                                }

c\:{DIGIT}+                     {
                                    count();
                                    yylval.num = atoi(yytext+2);
                                    return CONSTANT; 
                                }

"r:"                            {
                                    count();
                                    return REGISTER;
                                }

#INCLUDE[ \"\'\t]*              {
                                    count();
                                    BEGIN(INCL);
                                }
<INCL>[^\"\'\t\r\n]+            {
                                    /* got the include file name */
                                    count();
                                    if (include_stack_ptr >= MAX_INCLUDE_DEPTH) 
                                        yyerror("Includes nested too deeply");
                                    if (access(yytext, R_OK)!=0)
                                        yyerror("Cannot open include file");
                                    includenames[include_stack_ptr+1] = mstrdup(yytext);
                                }
<INCL>[ \"\'\t\r]*\n            {
                                    /* eat EOL and possible end quote */
                                    newLine();
                                    include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;
                                    sLineNumber[include_stack_ptr] = 0;
                                    yyin = fopen(includenames[include_stack_ptr], "r");     
                                    if (!yyin) {
                                        include_stack_ptr--;
                                        sLineNumber[include_stack_ptr]--;
                                        yyerror("Problem opening include file");
                                    }
                                    yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
                                    BEGIN(INITIAL);
                                }

{LABELD}                        {
                                    count();
                                    yylval.str = mstrdup(yytext);
                                    yylval.str[yyleng-1]=0;
                                    return LABEL;
                                }

{ID}                            {
                                    char buf[MAX_KEYWORD_LEN];
                                    struct keyword *k;

                                    lowercase(yytext, buf);
                                    k = in_word_set(buf, yyleng);
                                    count(); 
                                    
                                    if (k != NULL) {
                                        /* count numActions for ifFrameLoaded/IfFrameLoadedExpr */
                                        if (k->numActions > 0)
                                            numActions += k->numActions;
                                        else if (k->numActions < 0)
                                            numActions = 0;
                                        return k->token;
                                    }
                                    else {
                                        yylval.str = mstrdup(yytext);
                                        return STRING;
                                    }
                                }

\'                              {
                                    count();
                                    BEGIN STRINGSINGLEQ;
                                    s = sbuf;
                                }
<STRINGSINGLEQ>\n               {
                                    column[include_stack_ptr]++;
                                    yyerror("Unterminated string");
                                }
<STRINGSINGLEQ>\\b              {   count(); *s++ = '\b';  }
<STRINGSINGLEQ>\\t              {   count(); *s++ = '\t';  }
<STRINGSINGLEQ>\\n              {   count(); *s++ = '\n';  }
<STRINGSINGLEQ>\\f              {   count(); *s++ = '\f';  }
<STRINGSINGLEQ>\\r              {   count(); *s++ = '\r';  }
<STRINGSINGLEQ>\\\\             {   count(); *s++ = '\\';  }
<STRINGSINGLEQ>\\\'             {   count(); *s++ = '\'';  }
<STRINGSINGLEQ>\'               {
                                    count();
                                    *s = 0;
                                    yylval.str = mstrdup(sbuf);
                                    BEGIN(INITIAL);
                                    return STRING;
                                }
<STRINGSINGLEQ>.                {   
                                    count();
                                    *s++ = *yytext;
                                }

\"                              {
                                    count();
                                    BEGIN STRINGDOUBLEQ;
                                    s = sbuf;
                                }
<STRINGDOUBLEQ>\n               {
                                    column[include_stack_ptr]++;
                                    yyerror("Unterminated string");
                                }
<STRINGDOUBLEQ>\\b              {   count(); *s++ = '\b';  }
<STRINGDOUBLEQ>\\t              {   count(); *s++ = '\t';  }
<STRINGDOUBLEQ>\\n              {   count(); *s++ = '\n';  }
<STRINGDOUBLEQ>\\f              {   count(); *s++ = '\f';  }
<STRINGDOUBLEQ>\\r              {   count(); *s++ = '\r';  }
<STRINGDOUBLEQ>\\\\             {   count(); *s++ = '\\';  }
<STRINGDOUBLEQ>\\\"             {   count(); *s++ = '\"';  }
<STRINGDOUBLEQ>\"               {
                                    count(); 
                                    *s = 0;
                                    yylval.str = mstrdup(sbuf);
                                    BEGIN(INITIAL);
                                    return STRING;
                                }
<STRINGDOUBLEQ>.                {   
                                    count();
                                    *s++ = *yytext;
                                }

"movie"[ \t\v\f]+[\'\"]         {
                                    count();
                                    BEGIN MOVIEDECL;
                                    s = sbuf;
                                    return MOVIE;
                                }
<MOVIEDECL>\n                   {
                                    count();
                                    yyerror("Unterminated movie name");
                                }
<MOVIEDECL>[\'\"]               {
                                    count();
                                    *s = 0;
                                    yylval.str = mstrdup(sbuf);
                                    BEGIN(INITIAL);
                                    return MOVIENAME;
                                }
<MOVIEDECL>.                    {
                                    count();
                                    *s++ = *yytext;
                                }
"movie"[ \t\v\f]+[^\n\\'\\"]    {
                                    count();
                                    yyerror("Movie name must be included in quotes");
                                }

"/*"                            {   count(); BEGIN(BLOCKCOMMENT);  }
<BLOCKCOMMENT>[^*\n]*           {   count();                       }
<BLOCKCOMMENT>[^*\n]*\n         {   newLine();                      }
<BLOCKCOMMENT>"*"+[^*/\n]*      {   count();                       }
<BLOCKCOMMENT>"*"+[^*/\n]*\n    {   newLine();                      }
<BLOCKCOMMENT>"*"+"/"           {   count(); BEGIN(INITIAL);       }

"//"                            {
                                    int c;
                                    do
                                       c = input();
                                    while (c != '\n' && c != 0 && c != EOF);

                                    newLine();
                                }

":"                             {   count(); return ':';       }
"("                             {   count(); return '(';       }
")"                             {   count(); return ')';       }
"["                             {   count(); return '[';       }
"]"                             {   count(); return ']';       }
","                             {   count(); return ',';       }
"."                             {   count(); return '.';       }
"="                             {   count(); return '=';       }

[ \t\v\f]                       {   column[include_stack_ptr]++;   }
\n                              {   newLine();  }
\r                              {               }

.                               {   yyerror("Unrecognized character");  }

<<EOF>>                         {
                                    yy_delete_buffer(YY_CURRENT_BUFFER);
                                    if (--include_stack_ptr < 0) {
                                        yyEOF = 1;
                                        yyterminate();
                                    }
                                    else {
                                        yy_switch_to_buffer(include_stack[include_stack_ptr]);
                                    }
                                }


%%


int yywrap()
{
    return(1);
}

char *FileName(void)
{  
    if (include_stack_ptr==0)
        return inputName;
    else
        return includenames[include_stack_ptr];
}

int LineNumber(void)
{
    return sLineNumber[include_stack_ptr];
}

int ColumnNumber(void)
{
    return column[include_stack_ptr];
}

inline void newLine(void)
{
    column[include_stack_ptr] = 0;
    sLineNumber[include_stack_ptr]++;
}

inline void count(void)
{
    /* Count the characters to maintain the current column position */
    column[include_stack_ptr] += yyleng;
}

void warning(char *msg)
{
    strcpy(szLine, yytext + yyleng - ColumnNumber());
    tellUser(0, "\n%s", szLine);
    if (yytext[0] != '\n') tellUser(0, "\n");
    tellUser(0, "%*s", ColumnNumber(), "^");
    tellUser(0, "\nLine %4.4d of %s:\nWarning: %s \n\n", LineNumber() + 1, FileName(), msg);
}

void yyerror(char *msg)
{
    if (!yyEOF && strlen(yytext)) {
        strcpy(szLine, yytext + yyleng - ColumnNumber());
        sLineNumber[include_stack_ptr]++;
        tellUser(0, "\n%s", szLine);
        if (yytext[0] != '\n') tellUser(0, "\n");
        tellUser(0, "%*s", ColumnNumber(), "^");
        tellUser(1, "\nLine %4.4d of %s:\n%s \n", LineNumber(), FileName(), msg);
    }
    else
        tellUser(1, "Unexpected EOF found in %s while looking for input.\n",inputName);
}
