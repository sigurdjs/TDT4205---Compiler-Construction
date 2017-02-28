#include <vslc.h>


void node_print ( node_t *root, int nesting ) {
    if ( root != NULL )
    {
        /* Print the type of node indented by the nesting level */
        printf ( "%*c%s", nesting, ' ', node_string[root->type] );

        /* For identifiers, strings, expressions and numbers,
         * print the data element also
         */
        if ( root->type == IDENTIFIER_DATA ||
             root->type == STRING_DATA ||
             root->type == RELATION ||
             root->type == EXPRESSION ) 
            printf ( "(%s)", (char *) root->data );
        else if ( root->type == NUMBER_DATA )
            printf ( "(%lld)", *((int64_t *)root->data) );

        /* Make a new line, and traverse the node's children in the same manner */
        putchar ( '\n' );
        for ( int64_t i=0; i<root->n_children; i++ )
            node_print ( root->children[i], nesting+1 );
    }
    else
        printf ( "%*c%p\n", nesting, ' ', root );
}


/* Take the memory allocated to a node and fill it in with the given elements */
void node_init (node_t *nd, node_index_t type, void *data, uint64_t n_children, ...) {
    nd->type = type;
    nd->data = data;
    nd->n_children = n_children;
    nd->children = (node_t **)malloc(sizeof(node_t)*n_children);
    va_list ap;
    va_start(ap, n_children);
    for(int i = 0; i < n_children; i++) {
        nd->children[i] = va_arg(ap, node_t*);
    }
}

/* Remove a node and its contents */
void node_finalize ( node_t *discard ) {
    if(discard != NULL) {
        free(discard->data);
        free(discard);
    }
}


/* Recursively remove the entire tree rooted at a node */
void destroy_subtree ( node_t *discard ) {
    if(discard != NULL) {
        for(int i = 0; i < discard->n_children; i++) {
            if(discard->children[i] != NULL) {
                destroy_subtree(discard->children[i]);
            } 
        }
        node_finalize(discard);
    } 
}

int is_list_type( node_t *node ) {
    switch( node->type ) { 
        case GLOBAL_LIST:
            return 1; break;
        case STATEMENT_LIST:
            return 1; break;
        case PRINT_LIST:
            return 1; break;
        case EXPRESSION_LIST:
            return 1; break;
        case VARIABLE_LIST:
            return 1; break;
        case ARGUMENT_LIST:
            return 1; break;
        case PARAMETER_LIST:
            return 1; break;
        case DECLARATION_LIST:
            return 1; break;
        default:
            return 0;
    }
}

int is_purely_syntactic( node_t *node ) {
    switch( node->type ) {
        case GLOBAL:
            return 1; break;
        case STATEMENT:
            return 1; break;
        case PRINT_ITEM:
            return 1; break;
        case EXPRESSION:
            return 1; break;
        case ARGUMENT_LIST:
            return 1; break;
        case PARAMETER_LIST:
            return 1; break;
        default:
            return 0;
    }
}


void collapse_list( node_t **current, node_t **parent, int child_index) {
    if( is_list_type( (*parent) ) &&  (*parent)->type == (*current)->type ) { 
        node_t *to_be_removed = (*current);
        (*parent)->n_children += (*current)->n_children-1;
        node_t **new_array =  realloc((*parent)->children,sizeof(node_t*)*(*parent)->n_children);
        if(new_array != NULL) {
            (*parent)->children = new_array;
            (*parent)->children[(*parent)->n_children-1] = (*parent)->children[1];  //Move non_list node to end of children
            for(int i = 0; i < (*current)->n_children; i++) {
                (*parent)->children[i] = (*current)->children[i];
            } 
            free(to_be_removed);
        }
    }
}

            
void simplify_tree( node_t **current, node_t **parent, int child_index) {
    if( (*current) == NULL ) {
        return;
    }
    
    for( int i = 0; i < (*current)->n_children; i++ ) {
        simplify_tree( &(*current)->children[i], current, i );
    }

    if(( is_purely_syntactic( (*current) ) && (*current)->data == NULL ) && (*current)->n_children == 1 ) {
        node_t *to_be_removed = (*current);
 //       node_t *to_be_removed = (*parent)->children[child_index];
        (*parent)->children[child_index] = (*current)->children[0];
        free(to_be_removed);
    }
    collapse_list( current, parent, child_index);
}

void begin_traversing( node_t **tree ) {
    for(int i = 0; i < (*tree)->n_children; i++) {
        simplify_tree( &(*tree)->children[i], tree, i);
    }
}


            






































