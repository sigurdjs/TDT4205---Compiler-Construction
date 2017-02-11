%{
#include <vslc.h>
%}
%option noyywrap
%option array
%option yylineno

%state STRING

VARIABLE [a-zA-Z_][a-zA-Z0-9_]*
COMMENT \/\/.*\n
DIGIT [0-9]+
SPACE [\t\r\v\n ]+

%%
FUNC                    {printf("Found FUNC\n"); return FUNC;}
BEGIN                   {printf("Found BEGIN\n"); return OPENBLOCK;}
END                     {printf("Found END\n"); return CLOSEBLOCK;}
RETURN                  {printf("Found RETURN\n"); return RETURN;}
PRINT                   {printf("Found PRINT\n"); return PRINT;}
CONTINUE                {printf("Found CONTINUE\n"); return CONTINUE;}
IF                      {printf("Found IF\n"); return IF;}
THEN                    {printf("Found THEN\n"); return THEN;}
ELSE                    {printf("Found ELSE\n"); return ELSE;}
WHILE                   {printf("Found WHILE\n"); return WHILE;}
DO                      {printf("Found DO\n"); return DO;}
VAR                     {printf("Found VAR\n"); return VAR;}

<INITIAL>{VARIABLE}     {printf("Found idn: %s\n",yytext); return IDENTIFIER;}

<INITIAL>{COMMENT}      ;
<INITIAL>{DIGIT}        {printf("Found number: %s\n",yytext); return NUMBER;}
<INITIAL>{SPACE}        ; 
        
L?\"(\\.|[^\\"])*\" 	{printf("Found string: %s\n",yytext); return STRING;}

.                       {return (int) yytext[0];}

%%
