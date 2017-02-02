#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
typedef enum { INT, PUSH, POP, ADD, SUB, MUL, DIV, OUT, END } token_t;
#define BUFSIZE 12
char buffer[BUFSIZE];

#define NSTATES 24
int16_t transtab[NSTATES][256];
int16_t accepted_states[] = {2,5,8,11,14,17,20,23};
token_t tokens[] = {INT, MUL, POP, PUSH, ADD, SUB, DIV, OUT};

void setup_table ( void ) {
    memset ( transtab, 0, NSTATES*256*sizeof(int16_t) );
    /* TODO: insert DFA in table form */

    for(int i = 48; i < 58; i++) {
        transtab[1][i] = 2;
        transtab[2][i] = 2; 
    }
    transtab[1]['M'] = 3;
    transtab[3]['U'] = 4;
    transtab[6]['U'] = 9;
    transtab[15]['U'] = 16;
    transtab[21]['U'] = 22;
    transtab[4]['L'] = 5;
    transtab[1]['P'] = 6;
    transtab[7]['P'] = 8;
    transtab[1]['O'] = 21;
    transtab[6]['O'] = 7;
    transtab[1]['S'] = 15;
    transtab[9]['S'] = 10;
    transtab[10]['H'] = 11;
    transtab[1]['A'] = 12;
    transtab[1]['D'] = 18;
    transtab[12]['D'] = 13;
    transtab[13]['D'] = 14;
    transtab[16]['B'] = 17;
    transtab[18]['I'] = 19;
    transtab[19]['V'] = 20;
    transtab[22]['T'] = 23;
}


token_t next ( FILE *input ) {
    /* TODO: simulate DFA operation and buffer input characters.
     * Beware that the 'main' function expects the global character
     * array 'buffer' to contain matched lexeme characters, and will
     * need modification if you decide upon a different buffering scheme.
     */
    int16_t state = 1;
    fscanf(input, "%s",buffer);

    //printf("Value is: %d \n",state);
    for (int i = 0; i < strlen(buffer); i++) {
       /*if(buffer[i] == " ") {
           break;
       } */
//       printf("Buffer[i] = %c \n",buffer[i]);
//      printf("Value is: %d \n",transtab[state][(char) buffer[i]]);
       state = transtab[state][(char) buffer[i]];
    }
    printf("Ending state is: %d \n", state);
    for (int i = 0; i < NELEMS(accepted_states); i++) {
        if(accepted_states[i] == state) {
            return tokens[i];
        }
    }
    return END;
}


#define STACK_LIMIT 256
void act ( token_t token, char *buffer ) {
    static int32_t
        stack[STACK_LIMIT],
        stack_index = 0,
        register_value = 0;
    switch ( token )
    {
        case INT:
            register_value = strtol(buffer, NULL, 10);
            break;
        case PUSH:
            stack_index += 1;
            stack[stack_index] = register_value;
            break;
        case POP:
            register_value = stack[stack_index];
            stack_index -= 1;
            break;
        case ADD:
            stack[stack_index] += register_value;
            break;
        case SUB:
            stack[stack_index] -= register_value;
            break;
        case MUL:
            stack[stack_index] *= register_value;
            break;
        case DIV:
            stack[stack_index] /= register_value;
            break;
        case OUT:
            printf ( "%d\n", register_value );
            break;
        case END:
            break;
    }
}

int main () {
    setup_table();
    token_t next_token;
    do {
        next_token = next(stdin);
        act ( next_token, buffer );
    } while ( next_token != END );
    exit ( EXIT_SUCCESS );
}
