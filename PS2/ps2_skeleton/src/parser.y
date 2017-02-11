%{
#include <vslc.h>
node_t* constr_nd(); 
void copy_int(node_t* node, char* text);
%}

%left '+' '-'
%left '*' '/'
%left '.'
%nonassoc UMINUS


%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR NUMBER IDENTIFIER STRING



%%
program         : global_list   

global_list     : global
                | global_list global
                ;

global          : function
                | declaration
                ;

statement_list  : statement
                | statement_list statement
                ;

print_list      : print_item     {printf("Found item to print \n");} 
                | print_list ',' print_item
                ;

expression_list : expression
                | expression_list ',' expression
                ;

variable_list   : identifier
                | variable_list ',' identifier
                ;

argument_list   : expression_list
                | /* epsilon */
                ;

parameter_list  : variable_list
                | /* epsilon */
                ;

declaration_list: declaration
                | declaration_list declaration
                ;

function        : FUNC identifier '(' parameter_list ')' statement
                ;
                
statement       :
                assignment_statement
                | return_statement
                | print_statement
                | if_statement
                | while_statement
                | null_statement
                | block
                ;

block           : OPENBLOCK declaration_list statement_list CLOSEBLOCK
                | OPENBLOCK statement_list CLOSEBLOCK
                ;

assignment_statement: identifier ':' '=' expression
                    ;

return_statement: RETURN expression
                ;
                
print_statement : PRINT print_list 
                {printf("Found syntax to print something \n");}
                ;

null_statement  : CONTINUE
                ;

if_statement    : IF relation THEN statement
                | IF relation THEN statement ELSE statement
                ;

while_statement : WHILE relation DO statement
                ;

relation        :
                expression '=' expression
                | expression '<' expression
                | expression '>' expression
                ;

expression      :
                expression '+' expression    /*{$$ = constr_nd();
                node_init($$,EXPRESSION,$2,2,$1,$3); }*/
                | expression '-' expression    /*{$$ = constr_nd();
                node_init($$,EXPRESSION,$2,2,$1,$3); } */               
                | expression '*' expression    /*{$$ = constr_nd();
                node_init($$,EXPRESSION,$2,2,$1,$3); }*/               
                | expression '/' expression    /*{$$ = constr_nd();
                node_init($$,EXPRESSION,$2,2,$1,$3); }*/               
                | '-' expression %prec UMINUS
                | '(' expression ')'           /*{$$ = constr_nd();
                node_init($$,EXPRESSION,NULL,1,$2); } */
                | number
                | identifier
                | identifier '(' argument_list ')'
                ;

declaration     : VAR variable_list
                ;
                
print_item      : expression
                | string  {printf("Found string to print \n");}
                ;

identifier      : IDENTIFIER
                ;

number          : NUMBER                         {$$ = constr_nd(); node_init($$,NUMBER_DATA,NULL,0); copy_int($$,yytext);} 
                ;
                
string          : STRING
                {printf("String expressiong parsedi");}
                ;
%%

node_t* constr_nd() {
    node_t* ptr = (node_t *)malloc(sizeof(node_t));
    return ptr; 
}

void copy_int(node_t* node, char* text) {
    node->data = strtol(text,NULL,10);
}
    

int yyerror ( const char *error ) {
    fprintf ( stderr, "%s on line %d\n", error, yylineno );
    exit ( EXIT_FAILURE );
}
