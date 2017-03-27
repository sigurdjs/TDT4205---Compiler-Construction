#include <vslc.h>

tlhash_t *global_names;
char **string_list;
size_t n_string_list = 8, stringc = 0;

/* Helper function for inserting global variable into global symbol list */
static void insert_global_var(node_t* var_list);

/* Helper function for inserting function into global symbol list */
static void insert_function(node_t* func, int seq);

/* Helper function for inserting function parameters from variable_list */
static void insert_func_parms(tlhash_t *locals, node_t *list);

void find_globals ( void )
{
    global_names = malloc ( sizeof(tlhash_t) );
    tlhash_init ( global_names, 32 );
    node_t *global_list = root->children[0];
    int seq = 0;
    for(int i = 0; i < global_list->n_children; i++) {
        node_t *global = global_list->children[i];
        switch (global->type) {
            case FUNCTION:
                insert_function(global,seq);
                seq++;
                break;
            case DECLARATION:
                insert_global_var(global->children[0]);
                break;
            default:
                break;
        }
    }
}

static void insert_global_var(node_t* var_list)
{
    for(int i = 0; i < var_list->n_children; i++) {
        node_t *variable = var_list->children[i];
        symbol_t *el = malloc(sizeof(symbol_t));
        *el = (symbol_t) {
            .name = (char*) variable->data,
            .type = SYM_GLOBAL_VAR,
            .node = variable
        };
        tlhash_insert(global_names, el->name, strlen(el->name)+1, el);
    }
}

static void insert_function(node_t* func, int seq)
{
    symbol_t *el = malloc(sizeof(symbol_t));
    tlhash_t *locals = malloc(sizeof(tlhash_t));
    tlhash_init(locals,32);
    char* name; int nparms = 0;
    for(int i = 0; i < func->n_children; i++) {
        node_t *node = func->children[i];
        if (node != NULL) {
            switch (node->type) {
                case IDENTIFIER_DATA:
                    name = node->data;
                    break;
                case VARIABLE_LIST:
                    nparms = node->n_children;
                    insert_func_parms(locals,node);
                    break;
            }
        }
    }
    *el = (symbol_t) {
        .name = name,
        .type = SYM_FUNCTION,
        .node = func,
        .seq = (size_t) seq,
        .nparms = (size_t) nparms,
        .locals = locals
    };
    tlhash_insert(global_names, el->name, strlen(el->name)+1, el);
}

static void insert_func_parms(tlhash_t *locals, node_t *list)
{
    int seq = 0;
    for(int i = 0; i < list->n_children; i++) {
        node_t *list_el = list->children[i];
        symbol_t *el = malloc(sizeof(symbol_t));
        *el = (symbol_t) {
            .name = strdup(list_el->data),
            .node = list_el,
            .type = SYM_PARAMETER,
            .seq = (size_t) seq,
        };
        tlhash_insert(locals, el->name, strlen(el->name)+1, el);
        seq++;
    }
}

static void scope_level_down(stack *s);
static void scope_level_up(stack *s, tlhash_t *locals);
static void insert_local_var(node_t* var_list, tlhash_t *cur_table, int *seq);
static void traverse_tree(node_t *node,tlhash_t *locals, stack *scope_stack, int *seq);
static void link_variable(node_t *node, stack *scope_stack);
static void add_string_data(node_t *node);

void find_locals(void)
{
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );

    /* Call bind_names on all those which are functions */
    for ( size_t i=0; i<n_globals; i++ ) {
        if ( global_list[i]->type == SYM_FUNCTION ) {
            bind_names ( global_list[i], global_list[i]->node );
        }
    }
}

void bind_names ( symbol_t *function, node_t *root )
{
    stack s;
    stack_init(&s);
    stack_push(&s,function->locals);
    tlhash_t *locals = malloc(sizeof(tlhash_t));
    tlhash_init(locals,32);
    int seq = 0;
    for (int i = 0; i < root->n_children; i++) {
        if (root->children[i] != NULL) {
            traverse_tree(root->children[i],locals,&s,&seq);
        }
    }
    int sz = tlhash_size(locals);
    symbol_t *variables[sz];
    char* keys[sz];
    tlhash_values(locals,(void**)variables);
    tlhash_keys(locals,(void**)keys);
    for(int i = 0; i < sz; i++) {
        tlhash_insert(function->locals,keys[i],strlen(keys[i])+1,variables[i]);
    }
    tlhash_finalize(locals);
    free(locals);
    stack_destroy(&s);
    free(s);
}

static void scope_level_up(stack *s, tlhash_t *locals)
{
    tlhash_t *tab = stack_pop(s);
    int sz = tlhash_size(tab);
    symbol_t *variables[sz];
    tlhash_values(tab,(void**)variables);
    for(int i = 0; i < sz; i++) {
        char key[256];
        sprintf(key,"%s%zu",variables[i]->name,variables[i]->seq);
        tlhash_insert(locals,key,strlen(key)+1,variables[i]);
    }
    tlhash_finalize(tab);
    free(tab);
}

static void scope_level_down(stack *s)
{
    tlhash_t *new_table = malloc(sizeof(tlhash_t));
    tlhash_init(new_table,32);
    stack_push(s, new_table);
}

static void traverse_tree(node_t *node, tlhash_t *locals, stack *scope_stack, int* seq)
{
    switch (node->type) {
        case BLOCK:
            scope_level_down(scope_stack);
            break;
        case VARIABLE_LIST:
            if (stack_getsize(scope_stack) > 1) { // If size of stack is 1 we are in parameter list
                insert_local_var(node, (tlhash_t*) stack_peek(scope_stack), seq);
            }
            return; //Not interested in examining further, should not link declaration to anything..
        case IDENTIFIER_DATA:
            if (stack_getsize(scope_stack) > 1) { // If size of stack is 1 we look at function name
                link_variable(node, scope_stack);
            } else {
                return;
            }
            break;
        case STRING_DATA:
            add_string_data(node);
            break;
    }
    for (int i = 0; i < node->n_children; i++) {
        if (node->children[i] != NULL) {
            traverse_tree(node->children[i],locals,scope_stack,seq);
        } else {
            return;
        }
    }
    if (node->type == BLOCK)  {
        scope_level_up(scope_stack,locals);
        return;
    }
}

static void add_string_data(node_t *node)
{
    string_list = realloc(string_list,sizeof(*string_list)*(stringc+1));
    string_list[stringc] = node->data;
    size_t *i = malloc(sizeof(size_t)); //Free in node_finalize should remove this pointer
    *i = stringc;
    node->data = i;
    stringc++;
}

static void insert_local_var(node_t* var_list, tlhash_t *cur_table, int *seq)
{
    for(int i = 0; i < var_list->n_children; i++) {
        node_t *variable = var_list->children[i];
        symbol_t *el = malloc(sizeof(symbol_t));
        *el = (symbol_t) {
            .name = (char*) variable->data,
            .type = SYM_LOCAL_VAR,
            .seq = *seq,
            .node = variable
        };
        tlhash_insert(cur_table, el->name, strlen(el->name)+1, el);
        (*seq)++;
    }
}

static void link_variable(node_t *node, stack *scope_stack)
{
    symbol_t *linked_el = NULL;
    int stacksize = stack_getsize(scope_stack);
    tlhash_t *table[stacksize];
    for (int i = 0; i < stacksize; i++) {
        table[i] = (tlhash_t*) stack_pop(scope_stack);
        tlhash_lookup(table[i], (char*) node->data, strlen(node->data)+1,(void**) &linked_el);
        if (linked_el != NULL) {
            node->entry = linked_el;
            break;
        }
    }
    if (linked_el == NULL) {
        tlhash_lookup(global_names,(char*) node->data, strlen(node->data)+1,(void**) &linked_el);
        if (linked_el != NULL) {
            node->entry = linked_el;
        } else {
            printf("Syntax error, variable has not been declared \n");
        }
    }

    for(int i = stacksize - stack_getsize(scope_stack); i > 0; i--) {
        stack_push(scope_stack,table[i-1]);
    }
}

void destroy_symtab ( void )
{
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values (global_names, (void **)&global_list);
    size_t sz;
    for ( size_t i=0; i<n_globals; i++ ) {
        if ( global_list[i]->type == SYM_FUNCTION ) {
            sz = tlhash_size(global_list[i]->locals);
            symbol_t *symbols[sz];
            tlhash_values(global_list[i]->locals,(void**)symbols);
            for(int j = 0; j < sz; j++) {
                free(symbols[j]->locals);
                free(symbols[j]);
            }
            tlhash_finalize(global_list[i]->locals);
            free(global_list[i]->locals);
        }
    }
    tlhash_finalize (global_names);
    free (global_names);
    for (size_t i = 0; i < stringc; i++) {
        free(string_list[i]);
    }
    free(string_list);
}

void print_symbols ( void )
{
    printf ( "String table:\n" );
    for ( size_t s=0; s<stringc; s++ )
        printf  ( "%zu: %s\n", s, string_list[s] );
    printf ( "-- \n" );

    printf ( "Globals:\n" );
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)global_list );
    for ( size_t g=0; g<n_globals; g++ ) {
        switch ( global_list[g]->type ) {
            case SYM_FUNCTION:
                printf (
                    "%s: function %zu:\n",
                    global_list[g]->name, global_list[g]->seq
                );
                if ( global_list[g]->locals != NULL ) {
                    size_t localsize = tlhash_size( global_list[g]->locals );
                    printf (
                        "\t%zu local variables, %zu are parameters:\n",
                        localsize, global_list[g]->nparms
                    );
                    symbol_t *locals[localsize];
                    tlhash_values(global_list[g]->locals, (void **)locals );
                    for ( size_t i=0; i<localsize; i++ ) {
                        printf ( "\t%s: ", locals[i]->name );
                        switch ( locals[i]->type ) {
                            case SYM_PARAMETER:
                                printf ( "parameter %zu\n", locals[i]->seq );
                                break;
                            case SYM_LOCAL_VAR:
                                printf ( "local var %zu\n", locals[i]->seq );
                                break;
                        }
                    }
                }
                break;
            case SYM_GLOBAL_VAR:
                printf ( "%s: global variable\n", global_list[g]->name );
                break;
        }
    }
    printf ( "-- \n" );
}


void print_bindings ( node_t *root )
{
    if ( root == NULL )
        return;
    else if ( root->entry != NULL ) {
        switch ( root->entry->type ) {
            case SYM_GLOBAL_VAR:
                printf ( "Linked global var '%s'\n", root->entry->name );
                break;
            case SYM_FUNCTION:
                printf ( "Linked function %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
            case SYM_PARAMETER:
                printf ( "Linked parameter %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
            case SYM_LOCAL_VAR:
                printf ( "Linked local var %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
        }
    } else if ( root->type == STRING_DATA ) {
        size_t string_index = *((size_t *)root->data);
        if ( string_index < stringc )
            printf ( "Linked string %zu\n", *((size_t *)root->data) );
        else
            printf ( "(Not an indexed string)\n" );
    }
    for ( size_t c=0; c<root->n_children; c++ )
        print_bindings ( root->children[c] );
}
