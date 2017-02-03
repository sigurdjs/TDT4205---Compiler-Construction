%{
#include <vslc.h>
%}

%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR NUMBER IDENTIFIER STRING

%%
/*program :
      FUNC {
        root = (node_t *) malloc ( sizeof(node_t) );
        node_init ( root, PROGRAM, NULL, 0 );
      }*/
    
expr :
     NUMBER                     { node_init($$,NUMBER_DATA,$1,0); } 
     | expr '+' expr            { node_init($$,EXPRESSION,$2,2,$1,$3); }
     | expr '-' expr            { node_init($$,EXPRESSION,$2,2,$1,$3); }
     ;
%%

int yyerror ( const char *error ) {
    fprintf ( stderr, "%s on line %d\n", error, yylineno );
    exit ( EXIT_FAILURE );
}
