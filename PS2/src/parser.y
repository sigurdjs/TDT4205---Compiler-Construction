%{
#include <vslc.h>
node_t* constr_nd(); 
void copy_int(node_t* node, char* text);
void copy_string(node_t* node, char* text);
%}


%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%expect 1

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR NUMBER IDENTIFIER STRING



%%
program         : global_list   
                {$$ = constr_nd();
                node_init($$,PROGRAM,NULL,1,$1); 
                root = $$; }
                ;

global_list     : global
                {$$ = constr_nd();
                node_init($$,GLOBAL_LIST,NULL,1,$1); }
                | global_list global
                {$$ = constr_nd();
                node_init($$,GLOBAL_LIST,NULL,2,$1,$2); }
                ;

global          : function
                {$$ = constr_nd();
                node_init($$,GLOBAL,NULL,1,$1); }
                | declaration 
                {$$ = constr_nd();
                node_init($$,GLOBAL,NULL,1,$1); }
                ;

statement_list  : statement
                {$$ = constr_nd();
                node_init($$,STATEMENT_LIST,NULL,1,$1); }
                | statement_list statement
                {$$ = constr_nd();
                node_init($$,STATEMENT_LIST,NULL,2,$1,$2); }
                ;

print_list      : print_item      
                {$$ = constr_nd();
                node_init($$,PRINT_LIST,NULL,1,$1); }
                | print_list ',' print_item
                {$$ = constr_nd();
                node_init($$,PRINT_LIST,NULL,2,$1,$3); }
                ;

expression_list : expression
                {$$ = constr_nd();
                node_init($$,EXPRESSION_LIST,NULL,1,$1); }
                | expression_list ',' expression
                {$$ = constr_nd();
                node_init($$,EXPRESSION_LIST,NULL,2,$1,$3); }
                ;

variable_list   : identifier 
                {$$ = constr_nd();
                node_init($$,VARIABLE_LIST,NULL,1,$1); }
                | variable_list ',' identifier 
                {$$ = constr_nd();
                node_init($$,VARIABLE_LIST,NULL,2,$1,$3); }
                ;

argument_list   : expression_list
                {$$ = constr_nd();
                node_init($$,ARGUMENT_LIST,NULL,1,$1); }
                | /* epsilon */
                {$$ = constr_nd();
                node_init($$,ARGUMENT_LIST,NULL,0); }
                ;

parameter_list  : variable_list
                {$$ = constr_nd();
                node_init($$,PARAMETER_LIST,NULL,1,$1); }
                | /* epsilon */
                {$$ = constr_nd();
                node_init($$,PARAMETER_LIST,NULL,0); }
                ;

declaration_list: declaration
                 {$$ = constr_nd();
                node_init($$,DECLARATION_LIST,NULL,1,$1); }
                | declaration_list declaration
                {$$ = constr_nd();
                node_init($$,DECLARATION_LIST,NULL,2,$1,$2); }
                ;

function        : FUNC identifier '(' parameter_list ')' statement
                {$$ = constr_nd();
                node_init($$,FUNCTION,NULL,3,$2,$4,$6); }
                ;
                
statement       :
                assignment_statement
                {$$ = constr_nd();
                node_init($$,STATEMENT,NULL,1,$1);}
                | return_statement 
                {$$ = constr_nd();
                node_init($$,STATEMENT,NULL,1,$1);}
                | print_statement
                {$$ = constr_nd();
                node_init($$,STATEMENT,NULL,1,$1);}
                | if_statement
                {$$ = constr_nd();
                node_init($$,STATEMENT,NULL,1,$1);}
                | while_statement
                {$$ = constr_nd();
                node_init($$,STATEMENT,NULL,1,$1);}
                | null_statement
                {$$ = constr_nd();
                node_init($$,STATEMENT,NULL,1,$1);}
                | block
                {$$ = constr_nd();
                node_init($$,STATEMENT,NULL,1,$1);}
                ;

block           : OPENBLOCK declaration_list statement_list CLOSEBLOCK
                {$$ = constr_nd();
                node_init($$,BLOCK,NULL,2,$2,$3); }
                | OPENBLOCK statement_list CLOSEBLOCK
                {$$ = constr_nd();
                node_init($$,BLOCK,NULL,1,$2); }
                ;

assignment_statement: identifier ':' '=' expression
                    {$$ = constr_nd();
                    node_init($$,ASSIGNMENT_STATEMENT,NULL,2,$1,$4); }
                    ;

return_statement: RETURN expression
                {$$ = constr_nd();
                node_init($$,RETURN_STATEMENT,NULL,1,$2); }
                ;
                
print_statement : PRINT print_list 
                {$$ = constr_nd();
                node_init($$,PRINT_STATEMENT,NULL,1,$2); }
                ;

null_statement  : CONTINUE
                {$$ = constr_nd();
                node_init($$,NULL_STATEMENT,NULL,1,$1); }
                ;

if_statement    : IF relation THEN statement
                {$$ = constr_nd();
                node_init($$,IF_STATEMENT,NULL,2,$2,$4); }
                | IF relation THEN statement ELSE statement
                {$$ = constr_nd();
                node_init($$,IF_STATEMENT,NULL,3,$2,$4,$6); }
                ;

while_statement : WHILE relation DO statement
                {$$ = constr_nd();
                node_init($$,WHILE_STATEMENT,NULL,2,$2,$4); }
                ;

relation        :
                expression '=' expression
                {$$ = constr_nd();
                node_init($$,RELATION, strdup("="),2,$1,$3); }
                | expression '<' expression
                {$$ = constr_nd();
                node_init($$,RELATION, strdup("<"),2,$1,$3); }
                | expression '>' expression
                {$$ = constr_nd();
                node_init($$,RELATION, strdup(">"),2,$1,$3); }
                ;

expression      :
                expression '+' expression    
                {$$ = constr_nd();
                node_init($$,EXPRESSION,strdup("+"),2,$1,$3); } 
                | expression '-' expression    
                {$$ = constr_nd();
                node_init($$,EXPRESSION,strdup("-"),2,$1,$3); } 
                | expression '*' expression    
                {$$ = constr_nd();
                node_init($$,EXPRESSION,strdup("*"),2,$1,$3); }              
                | expression '/' expression    
                {$$ = constr_nd();
                node_init($$,EXPRESSION,strdup("/"),2,$1,$3); }              
                | '-' expression %prec UMINUS
                {$$ = constr_nd();
                node_init($$,EXPRESSION,strdup("-"),1,$2); }                
                | '(' expression ')'           
                {$$ = $2;}
                | number
                {$$ = constr_nd();
                node_init($$,EXPRESSION,NULL,1,$1); }
                | identifier
                {$$ = constr_nd();
                node_init($$,EXPRESSION,NULL,1,$1); }
                | identifier '(' argument_list ')'
                {$$ = constr_nd();
                node_init($$,EXPRESSION,NULL,2,$1,$3); }
                ;

declaration     : VAR variable_list
                {$$ = constr_nd();
                node_init($$,DECLARATION,NULL,1,$2); }
                ;
                
print_item      : expression 
                {$$ = constr_nd();
                node_init($$,PRINT_ITEM,NULL,1,$1); }
                | string                  
                {$$ = constr_nd();
                node_init($$,PRINT_ITEM,NULL,1,$1); }
                ;

identifier      : IDENTIFIER 
                {$$ = constr_nd();
                node_init($$,IDENTIFIER_DATA,NULL,0);
                copy_string($$,yytext); }
                ;

number          : NUMBER {}                        
                {$$ = constr_nd();
                node_init($$,NUMBER_DATA,NULL,0); 
                copy_int($$,yytext);} 
                ;
                
string          : STRING
                {$$ = constr_nd();
                node_init($$,STRING_DATA,NULL,0);
                copy_string($$,yytext); }
                ;
%%

node_t* constr_nd() {
    node_t* ptr = (node_t *)malloc(sizeof(node_t));
    return ptr; 
}

void copy_int(node_t* node, char* text) {
    int64_t* number = malloc(sizeof(int64_t));
    *number = strtol(text,NULL,10);
    node->data = number;
}

void copy_string(node_t* node, char* text) {
    node->data = strdup(text);
}   

int yyerror ( const char *error ) {
    fprintf ( stderr, "%s on line %d\n", error, yylineno );
    exit ( EXIT_FAILURE );
}
