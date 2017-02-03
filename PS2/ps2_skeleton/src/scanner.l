%{
#include <vslc.h>
%}
%option noyywrap
%option array
%option yylineno

%state STRING

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

    /*
[a-zA-Z0-9_]*           {yylval = strdup(yytext);
                        return IDENTIFIER;}
                        */


\/\/.*                  ;
[0-9]+                  {printf("Found NUMBER: %s\n",yytext); yylval = atoi(yytext);
                        return NUMBER;}
[-+]                    {printf("Found OPERATOR: %s\n",yytext); return yytext;}                        

    /*
<INITIAL>\"             {BEGIN(STRING);}
<STRING>\"              {BEGIN(INITIAL);}
<STRING>.               {yylval = strdup(yytext);
                        return STRING;}
                        */
[\t\r\v\n ]             ; 
.                       {printf("Rest of text:%s",yytext); return yytext;}
%%
