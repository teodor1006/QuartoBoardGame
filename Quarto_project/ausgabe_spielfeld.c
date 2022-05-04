#include <stdio.h>
#include <stdlib.h>

#include "ausgabe_spielfeld.h"




char* board[BOARD_SIZE][BOARD_SIZE];
time_t t;

/*static inline unsigned long long S_to_binary_(const char *s)
{
    unsigned long long i = 0;
    while (*s) {
        i <<= 1;
        i += *s++ - '0';
    }
    return i;
}*/


/*void dummy_think(char* move)
{
    char* properties =(char*) malloc(sizeof(char)*(BINARY_PROPERTIES+1));
    srand(time(&t));
    for(int i=0; i<BINARY_PROPERTIES;i++)
    {
        properties[i] = (rand()%2)+'0';
    }
    strncpy(move,properties,BINARY_PROPERTIES);
    move[4] = '\0';
    free(properties);
}*/

void reset_board()
{
    //outer loop for rows and inner loop for columns
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE ; j++)
        {
            if ((board[i][j] = calloc((BOARD_SIZE) + 1, 1)) == NULL) {
                perror("line 43 ausgabe_spielfeld.c:: calloc error");
                exit(EXIT_FAILURE);
            }
            memset(board[i][j], '*', BINARY_PROPERTIES);

        }
    }
}

void print_board()
{
    printf("   A      B     C     D  \n");
    printf("+----------------------- +\n");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("| ");
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            printf("%s  ",   board[i][j]);
        }
        printf("|\n");
    }
    printf("+----------------------- +\n");
    printf("   A      B     C     D  \n");
}

void free_board()
{
    //outer loop for rows and inner loop for columns
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            free(board[i][j]);

        }
    }

}