#ifndef thinker
#define thinker


/*
struct tuple{
	int a;
	int b;
}; t1 = {0,0}, t2 = {0,1}, t3 = {0,2}, t4 = {0,3},
   t5 = {1,0}, t6 = {1,1}, t7 = {1,2}, t8 = {1,3}, 
   t9 = {2,0} ,t10 = {2,1}, t11 = {2,2}, t12 = {2,3}, 
   t13 = {3,0}, t14 = {3,1}, t15 = {3,2}, t16 = {3,3};
*/

void delete_stone(int stone);
void reset_int_board();
void print_int_board();
int * select_row_column();
char *place_stone(int stone);
int choose_stone();
char * superDummy(int next_stone);
void attach_Thinker(int shm_id);


// new fuctions for reseting the board, we can use them later in ausgabe_spielfeld
// so that the code looks cleaner

void int_Board_To_char_Board();
void update_board(char *board);
void update_int_board();

//draw checking

int sumOfArray(int n);
int detectDraw();
// new functions for win-checking

int winCheck ();
int dec_to_bin_prop(int *line);
int evaluate_properties ();


#endif
