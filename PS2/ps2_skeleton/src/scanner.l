%{
#include <vslc.h>
%}
%option noyywrap
%option array
%option yylineno

%state STRING

%%
\/\/                    {printf("%s\n",yytext);}
FUNC                    ECHO;
BEGIN                   ECHO;
END                     ECHO;
RETURN                  ECHO;
PRINT                   ECHO;
CONTINUE                ECHO;
IF                      ECHO;
THEN                    ECHO;
ELSE                    ECHO;
WHILE                   ECHO;
DO                      ECHO;
VAR                     ECHO;

<INITIAL>\"             {BEGIN(STRING);}
<STRING>\"              {BEGIN(INITIAL);}
<STRING>.               {ECHO;}
.                       { ECHO; }
%%
