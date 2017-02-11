#include <stdio.h>
#include <stdlib.h>
#include <vslc.h>

node_t *root;

int main ( int argc, char **argv ) {
    /*while(1) {
        yylex();
    }*/
    yyparse();
    /*root = (node_t *)malloc(sizeof(node_t));
    node_t *node1 = (node_t *)malloc(sizeof(node_t));
    node_t *node2 = (node_t *)malloc(sizeof(node_t));
    node_t *node3 = (node_t *)malloc(sizeof(node_t));
    node1->type = RELATION;
    node2->type = RELATION;
    node3->type = RELATION;
        
    node_init(root,PROGRAM,NULL,3,node1,node2,node3);
    */
    //yyparse();
    //node_print ( root, 0 );
    //destroy_subtree ( root );
    
}
