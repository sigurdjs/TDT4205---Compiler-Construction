%{
#include <vslc.h>
%}
%option noyywrap
%option array
%option yylineno

VARIABLE [a-zA-Z_][a-zA-Z0-9_]*
COMMENT \/\/.*\n
DIGIT [0-9]+
SPACE [\t\r\v\n ]+

%%
FUNC                    { return FUNC;}
BEGIN                   { return OPENBLOCK;}
END                     { return CLOSEBLOCK;}
RETURN                  { return RETURN;}
PRINT                   { return PRINT;}
CONTINUE                { return CONTINUE;}
IF                      { return IF;}
THEN                    { return THEN;}
ELSE                    { return ELSE;}
WHILE                   { return WHILE;}
DO                      { return DO;}
VAR                     { return VAR;}

<INITIAL>{VARIABLE}     { return IDENTIFIER;}

<INITIAL>{COMMENT}      ;
<INITIAL>{DIGIT}        { return NUMBER;}
<INITIAL>{SPACE}        ; 
        
L?\"(\\.|[^\\"])*\" 	{ return STRING;}

.                       {return (int) yytext[0];}

%%
