#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tlhash.h"

typedef struct {
    char *name, *symbol;
    uint16_t atomic_number;
    double mass;
} element_t;


int
main ( int argc, char **argv )
{
    /* Initialize a hash table */
    tlhash_t *table = malloc ( sizeof(tlhash_t) );
    tlhash_init ( table, 16 );

    /* Make some table elements */
    element_t
        *h  = malloc ( sizeof(element_t) ),
        *he = malloc ( sizeof(element_t) ),
        *li = malloc ( sizeof(element_t) );
    *h  = (element_t) {
        .name = strdup("Hydrogen"), .symbol=strdup ("H"),
        .atomic_number=1, .mass=1.008
    };
    *he = (element_t) {
        .name = strdup("Helium"), .symbol=strdup ("He"),
        .atomic_number=2, .mass=4.003
    };
    *li = (element_t) {
        .name = strdup("Lithium"), .symbol=strdup ("Li"),
        .atomic_number=3, .mass=6.940
    };

    /* Insert them, index by their names */
    tlhash_insert ( table, h->name, strlen(h->name)+1, h );
    tlhash_insert ( table, he->name, strlen(he->name)+1, he );
    tlhash_insert ( table, li->name, strlen(li->name)+1, li );

    /* Look one up */
    element_t *something = NULL;
    tlhash_lookup ( table, "Helium", strlen("Helium")+1, (void **)&something );
    if ( something != NULL )
        printf (
            "Found helium: %s, %u, %.3e amu\n",
            something->symbol, something->atomic_number, something->mass
        );

    /* Try one that isn't there */
    tlhash_lookup ( table, "Carbon", strlen("Carbon")+1, (void **)&something );
    if ( something == NULL )
        printf ( "Didn't find any carbon\n" );

    /* Remove helium */
    tlhash_lookup ( table, "Helium", strlen("Helium")+1, (void **)&something );
    tlhash_remove ( table, something->name, strlen(something->name)+1 );
    /* Removing the hash table entry only takes a struct pointer out of the
     * table, the struct itself is not removed unless we do it here:
     */
    free ( something->name ), free ( something->symbol );
    free ( something );
    printf ( "Deleted helium\n" );

    /* Find the number of elements */
    size_t sz = tlhash_size(table);
    printf ( "Table contents:\n" );

    /* Look up all the keys */
    char *keys[sz];
    tlhash_keys ( table, (void **)keys );
    for ( int i=0; i<sz; i++ )
        printf ( "Key %d: %s\n", i, keys[i] );

    /* Look up all the values */
    element_t *vals[sz];
    tlhash_values ( table, (void **)vals );
    for ( int i=0; i<sz; i++ )
        printf (
            "%s: %s, %u, %.3e amu\n",
            vals[i]->name, vals[i]->symbol,
            vals[i]->atomic_number, vals[i]->mass
        );

    /* Destroy the hash table */
    tlhash_finalize ( table );
    free ( table );

    /* Free the remaining elements */
    free ( h->name ), free ( li->name );
    free ( h->symbol ), free ( li->symbol );
    free ( h ), free ( li );
}
