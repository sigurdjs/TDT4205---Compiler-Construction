#include <stdio.h>
#include <stdlib.h>
#include <vslc.h>

node_t *root;

int main ( int argc, char **argv ) {

    yyparse();
    simplify_tree ( &root, root );

    // Put the global names in the global symbol table //
   find_globals();
   find_locals();

    // Print the final state of the symbol table(s) //
    print_symbols();
    printf ( "Bindings:\n" );
    print_bindings ( root );

    destroy_symtab();

    destroy_subtree ( root );
}
