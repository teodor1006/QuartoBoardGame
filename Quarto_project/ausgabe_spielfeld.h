#pragma once

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<limits.h>
#include<time.h>


#include "config.h"


#define BINARY_PROPERTIES 4
#define BOARD_SIZE 4

extern char* board[BOARD_SIZE][BOARD_SIZE];
extern time_t t;
//from https://stackoverflow.com/questions/15114140/writing-binary-number-system-in-c-code
#define B(x) S_to_binary_(#x)

void dummy_think();
void reset_board();
void print_board();
void free_board();

//int* select_row_column();

