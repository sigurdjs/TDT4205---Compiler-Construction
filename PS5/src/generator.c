#include "vslc.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};
static const char *scratch_regs[4] = {
    "%rax", "%rbx", "%rcx", "%rdx"
};

size_t n_parms;
size_t loop_cntr = 0;
size_t func_cntr = 0;
size_t cond_cntr = 0;

static void generate_statement(node_t *node);
static void generate_function_call(node_t *node);
static void generate_block_stmnt(node_t *node);

static void generate_stringtable ( void )
{
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */
    puts ( ".section .rodata" );
    puts ( "intout: .string \"\%ld \"" );
    puts ( "strout: .string \"\%s \"" );
    puts ( "errout: .string \"Wrong number of arguments\"" );

    /* TODO:  handle the strings from the program */
    for (size_t i = 0; i < stringc; i++) {
        printf("STR%zu:\n \t.string %s\n",i,string_list[i]);
    }

}

static void generate_globals ( void )
{
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values(global_names, (void **)&global_list);
    //  Label globals
    puts(".section .data");
    for (size_t i = 0; i < n_globals; i++) {
        if (global_list[i]->type == SYM_GLOBAL_VAR) {
            printf("_%s:\t.zero 8\n",global_list[i]->name);
        }
    }
}

static void allocate_stack_space(symbol_t *func) {
    size_t n_locals = tlhash_size(func->locals);
    /* Allocate space on stack for paramateres
    + local variables + ensure 16-bit alignment */
    if (n_locals % 2 == 0)
        printf("\tsubq $%zu, %%rsp\n",8*n_locals);
    else
        printf("\tsubq $%zu, %%rsp\n",(size_t) 8*(n_locals+1));

}

static void save_parm_to_stack(symbol_t *func) {
    for (size_t i = 0; i < n_parms; i++){
        printf("\tmovq %s, -%zu(%%rbp)\n",record[i],8*(i+1));
    }
}

static void mov_variable(node_t *node, char* dest_reg) {
    if (node->type == IDENTIFIER_DATA){
        switch (node->entry->type) {
            case SYM_GLOBAL_VAR:
                printf("\tpushq _%s\n",node->entry->name);
                printf("\tpopq %s\n",dest_reg);
                break;
            case SYM_PARAMETER:
                printf("\tpushq -%zu(%%rbp)\n",(size_t)8*(node->entry->seq+1));
                printf("\tpopq %s\n",dest_reg);
                break;
            case SYM_LOCAL_VAR:
                printf("\tpushq -%zu(%%rbp)\n",(size_t)8*(node->entry->seq+n_parms+1));
                printf("\tpopq %s\n",dest_reg);
                break;
        }
    } else if (node->type == NUMBER_DATA) {
        printf("\tmovq $%ld, %s\n",*((int64_t*)node->data),dest_reg);

    }
}

static void get_var_register(node_t *node, char** reg) {
    if (node->type == IDENTIFIER_DATA){
        switch (node->entry->type) {
            case SYM_GLOBAL_VAR:
                sprintf(*reg,"_%s",node->entry->name);
                break;
            case SYM_PARAMETER:
                sprintf(*reg,"-%zu(%%rbp)",(size_t)8*(node->entry->seq+1));
                break;
            case SYM_LOCAL_VAR:
                sprintf(*reg,"-%zu(%%rbp)",(size_t)8*(node->entry->seq+n_parms+1));
                break;
        }
    }
}

static void generate_expression(node_t *node) {
    if (node->data == NULL) {
        generate_function_call(node);
        return;
    }
    /* Recursively generates expression and leaves answer in %rax */
    for (size_t i = 0; i < node->n_children; i++) {
        if (node->children[i] != NULL && node->type == EXPRESSION)
            generate_expression(node->children[i]);
        else
            return;
    }
    if (node->n_children == 2){
        if (node->children[0]->type != EXPRESSION && node->children[1]->type != EXPRESSION) {
            mov_variable(node->children[0],"%rax");
            mov_variable(node->children[1],"%rbx");
        } else if (node->children[0]->type != EXPRESSION) {
            mov_variable(node->children[0],"%rax");
            puts("\tpopq %rbx");
        } else if (node->children[1]->type != EXPRESSION) {
            puts("\tpopq %rax");
            mov_variable(node->children[1],"%rbx");
        } else {
            puts("\tpopq %rbx");
            puts("\tpopq %rax");
        }
        switch (*((char*)node->data)){
            case '+':
                puts("\taddq %rbx, %rax");
                break;
            case '-':
                puts("\tsubq %rbx, %rax");
                break;
            case '/':
                puts("\tcqo");
                puts("\tidivq %rbx");
                break;
            case '*':
                puts("\tcqo");
                puts("\timulq %rbx");
                break;
        }
        puts("\tpushq %rax\n");
    } else if (node->n_children == 1 && *((char*)node->data) == '-') {
        mov_variable(node->children[0],"%rax");
        puts("\tmovq $0, %rbx");
        puts("\tsubq %rax, %rbx");
        puts("\tpushq %rbx\n");
    }
}

static void generate_assignment_stmnt(node_t *node) {
    node_t *lhs = node->children[0];
    node_t *rhs = node->children[1];
    char *src = malloc(256);
    char *dst = malloc(256);
    switch (rhs->type) {
        case NUMBER_DATA:
            get_var_register(lhs,&dst);
            printf("\tmovq $%ld, %s\n",*((int64_t*)rhs->data),dst);
            break;
        case IDENTIFIER_DATA:
            get_var_register(rhs,&src);
            printf("\tpushq %s\n",src);
            get_var_register(lhs,&dst);
            printf("\tpopq %s\n",dst);
            break;
        case EXPRESSION:
            generate_expression(rhs);
            get_var_register(lhs,&dst);
            printf("\tpopq %s\n",dst);
    }
    free(src);  free(dst);
}

static void generate_print_stmnt(node_t *node) {
    node_t *child;
    char *reg = malloc(256);
    for (size_t i = 0; i < node->n_children; i++){
        child = node->children[i];
        switch (child->type) {
            case STRING_DATA:
                puts("\tmovq $strout, %rdi");
                printf("\tmovq $STR%zu, %%rsi\n",*((size_t*)child->data));
                puts("\tcall printf");
                break;
            case EXPRESSION:
                generate_expression(child);
                puts("\tmovq $intout, %rdi");
                puts("\tpopq %rsi");
                puts("\tcall printf");
                break;
            case NUMBER_DATA:
                puts("\tmovq $intout, %rdi");
                printf("\tmovq $%ld, %%rsi\n",*((int64_t*)child->data));
                puts("\tcall printf");
                break;
            case IDENTIFIER_DATA:
                puts("\tmovq $intout, %rdi");
                get_var_register(child,&reg);
                printf("\tmovq %s, %%rsi\n",reg);
                puts("\tcall printf");
                break;
        }
    }
    free(reg);
}

static void generate_return_stmnt(node_t *node) {
    char *reg = malloc(256);
    node_t *ret_val = node->children[0];
    switch (ret_val->type) {
        case IDENTIFIER_DATA:
            get_var_register(ret_val,&reg);
            printf("\tmovq %s, %%rax\n",reg);
            break;
        case NUMBER_DATA:
            printf("\tmovq $%ld, %%rax\n",*((int64_t*)ret_val->data));
            break;
        case EXPRESSION:
            generate_expression(ret_val);
            puts("\tpopq %rax");
            break;
    }
    free(reg);
}

static void generate_relation(node_t *node) {
    char *reg = malloc(256);
    /* N_children should never be more than 2 anyways,
    so the comparison is always between rax (lhs) and rbx (rhs) */
    for (size_t i = 0; i < node->n_children; i++) {
        node_t* child = node->children[i];
        switch (child->type) {
            case IDENTIFIER_DATA:
                get_var_register(child,&reg);
                printf("\tmovq %s, %s\n",reg,scratch_regs[i]);
                break;
            case NUMBER_DATA:
                printf("\tmovq $%ld, %s\n",*((int64_t*)child->data),scratch_regs[i]);
                break;
            case EXPRESSION:
                generate_expression(child);
                printf("\tpopq %s\n",scratch_regs[i]);
                break;
        }
    }
    puts("\tcmpq %rbx, %rax");
    free(reg);
}

static void generate_if_stmnt(node_t *node) {
    size_t cur_cond = cond_cntr++;
    node_t *relation = node->children[0];
    generate_relation(relation);
    switch (*((char*)relation->data)){
        case '=':
            printf("\tje _if%zu\n",cur_cond);
            printf("\tjne _else%zu\n",cur_cond);
            break;
        case '<':
            printf("\tjl _if%zu\n",cur_cond);
            printf("\tjge _else%zu\n",cur_cond);
            break;
        case '>':
            printf("\tjg _if%zu\n",cur_cond);
            printf("\tjle _else%zu\n",cur_cond);
            break;
    }
    printf("_if%zu:\n",cur_cond);
    generate_statement(node->children[1]);
    printf("\tjmp _continue%zu\n",cur_cond);
    printf("_else%zu:\n",cur_cond);
    if (node->n_children == 3)
        generate_statement(node->children[2]);
    printf("\tjmp _continue%zu\n",cur_cond);
    printf("_continue%zu:\n",cur_cond);
}

static void generate_while_stmnt(node_t *node) {
    size_t cur_loop = loop_cntr++;
    printf("_loop%zu:\n",cur_loop);
    generate_statement(node->children[1]);
    node_t *relation = node->children[0];
    generate_relation(relation);
    switch (*((char*)relation->data)){
        case '=':
            printf("\tje _loop%zu\n",cur_loop);
            printf("\tjne _loop_finished%zu\n",cur_loop);
            break;
        case '<':
            printf("\tjl _loop%zu\n",cur_loop);
            printf("\tjge _loop_finished%zu\n",cur_loop);
            break;
        case '>':
            printf("\tjg _loop%zu\n",cur_loop);
            printf("\tjle _loop_finished%zu\n",cur_loop);
            break;
    }
    printf("_loop_finished%zu:\n",cur_loop);
}

static void generate_statement(node_t *node) {
    switch (node->type) {
        case ASSIGNMENT_STATEMENT:
            generate_assignment_stmnt(node);
            break;
        case PRINT_STATEMENT:
            generate_print_stmnt(node);
            break;
        case RETURN_STATEMENT:
            generate_return_stmnt(node);
            break;
        case IF_STATEMENT:
            generate_if_stmnt(node);
            break;
        case WHILE_STATEMENT:
            generate_while_stmnt(node);
            break;
        case BLOCK:
            generate_block_stmnt(node);
            break;
    }
}

static void generate_statement_list(node_t *node) {
    for (size_t i = 0; i < node->n_children; i++) {
        node_t *stmnt = node->children[i];
        generate_statement(stmnt);
    }
}

static void generate_block_stmnt(node_t *node) {
    for (size_t i = 0; i < node->n_children; i++) {
        if (node->children[i]->type == STATEMENT_LIST) {
            generate_statement_list(node->children[i]);
        }
    }
}

static void traverse_tree(node_t *node) {
    if(node == NULL)
        return;
    switch (node->type) {
        case EXPRESSION:
            generate_expression(node);
            return;
        case STATEMENT_LIST:
            generate_statement_list(node);
            return;
        case BLOCK:
            generate_block_stmnt(node);
            return;
    }
    for (size_t i = 0; i < node->n_children; i++) {
        traverse_tree(node->children[i]);
    }
}

static void generate_function_call(node_t *node) {
    char *reg = malloc(256);
    node_t *func = node->children[0];
    node_t *expr_list = node->children[1];
    for (size_t i = 0; i < expr_list->n_children; i++) {
        node_t *child = expr_list->children[i];
        switch (expr_list->children[i]->type) {
            case NUMBER_DATA:
                printf("\tmovq $%ld, %s\n",*((int64_t*)child->data),record[i]);
                break;
            case IDENTIFIER_DATA:
                get_var_register(child,&reg);
                printf("\tmovq %s, %s\n",reg,record[i]);
                break;
            case EXPRESSION:
                generate_expression(child);
                printf("\tpopq %s\n",record[i]);
                break;
        }
    }
    printf("\tcall _%s\n",func->entry->name);
    puts("\tpushq %rax");
}

static void generate_function (symbol_t *function) {
    printf("_%s:\n", function->name);
    puts ( "\tpushq %rbp" );
    puts ( "\tmovq %rsp, %rbp" );
    n_parms = function->nparms;
    allocate_stack_space(function);
    save_parm_to_stack(function);
    traverse_tree(function->node);
    func_cntr++;
    puts("\tleave");
    puts("\tret");
}

static void generate_main ( symbol_t *first )
{
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );
    puts ( "\tpushq %rbp" );
    puts ( "\tmovq %rsp, %rbp" );

    puts ( "\tsubq $1, %rdi" );
    printf ( "\tcmpq\t$%zu,%%rdi\n", first->nparms );
    puts ( "\tjne ABORT" );
    puts ( "\tcmpq $0, %rdi" );
    puts ( "\tjz SKIP_ARGS" );

    puts ( "\tmovq %rdi, %rcx" );
    printf ( "\taddq $%zu, %%rsi\n", 8*first->nparms );
    puts ( "PARSE_ARGV:" );
    puts ( "\tpushq %rcx" );
    puts ( "\tpushq %rsi" );

    puts ( "\tmovq (%rsi), %rdi" );
    puts ( "\tmovq $0, %rsi" );
    puts ( "\tmovq $10, %rdx" );
    puts ( "\tcall strtol" );

    /*  Now a new argument is an integer in rax */
    puts ( "\tpopq %rsi" );
    puts ( "\tpopq %rcx" );
    puts ( "\tpushq %rax" );
    puts ( "\tsubq $8, %rsi" );
    puts ( "\tloop PARSE_ARGV" );

    /* Now the arguments are in order on stack */
    for ( int arg=0; arg<MIN(6,first->nparms); arg++ )
        printf ( "\tpopq\t%s\n", record[arg] );

    puts ( "SKIP_ARGS:" );
    printf ( "\tcall\t_%s\n", first->name );
    puts ( "\tjmp END" );
    puts ( "ABORT:" );
    puts ( "\tmovq $errout, %rdi" );
    puts ( "\tcall puts" );

    puts ( "END:" );
    puts ( "\tmovq %rax, %rdi" );
    puts ( "\tcall exit" );
}

void generate_program ( void )
{
    generate_stringtable();
    generate_globals();
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values(global_names, (void **)&global_list);
    for (size_t i = 0; i < n_globals; i++) {
        if (global_list[i]->type == SYM_FUNCTION) {
            if (global_list[i]->seq == 0)
                generate_main(global_list[i]); // Generate main code first
        }
    }
    for (size_t i = 0; i < n_globals; i++) {
        if (global_list[i]->type == SYM_FUNCTION)
            generate_function(global_list[i]);
    }
}
