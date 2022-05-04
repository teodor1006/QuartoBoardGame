#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/shm.h>

#include "thinker.h"
#include "sharedMemory.h"

#include "ausgabe_spielfeld.h"

#define USED 8888
#define FREE -1
//#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
int counter;	//um zu zählen wie viele elemente in newArray sind
//time_t t;
char spielzug[6];
// these three are used for win-checking
int line[4];
int binary_prop[4][4];
int position[2];
// this "int position[2]" gets updated with the position we chose to put our stone into, then we use it to check if we made a winning move
// it gets updated in "place_stone" function, just after it chooses the place on the board


//int int_board[BOARD_SIZE][BOARD_SIZE]; //---> moved it to shared memory, it works, but lets keep this just in case, dont know why..
int unused_stones [16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
char unused_bin_stones [16][5] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};

struct game_info *ptr;


/* we basically dont need any return here, and nothing big actually....the whole purpose of this method is to attach our thinker to
   shared memory, created in main, since we cant have our main here as well.
   We call this from our parent process as soon as the program starts, and attach this "*ptr" up there to the memory.

   (( shmat() returns the address to the memory area, so we save it in *ptr ))
*/
void attach_Thinker(int shm_id) {
	ptr = shmat(shm_id, NULL, 0);

}

// at the beginning we have all 16 stones, each time a stone gets used, we save that event by saving "8888" in the place of the used stone
void delete_stone(int stone) {
	unused_stones[stone] = USED;
}

// used just at the beginning of the game, reset all fields to FREE (-1) (we need this to check which places are filled)
void reset_int_board()
{
	//outer loop for rows and inner loop for columns
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE ; j++)
		{
			ptr->int_board[i][j] = FREE;
		}
	}
}

void int_Board_To_char_Board() {

	//outer loop for rows and inner loop for columns
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE ; j++)
		{
			int num = ptr->int_board[i][j];
			if (num == -1) {
				strncpy(board[i][j], "****", 5);
			}
			else {
				strncpy(board[i][j], unused_bin_stones[num], 5);
			}
		}

	}
}


void print_int_board()
{
	printf("+-----------------------  +\n");
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		printf("| ");
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			printf("%d  ",   ptr->int_board[i][j]);
		}
		printf("|\n");
	}
	printf("+-----------------------  +\n");
}


//##########################
//andis function lol
//##########################
//die arrays hier sollen tupel darstellen für die jeweiligen board positionen, die werden in dem arrayTry gespeichert, das auf die einträge zeigt
//eins von den tupeln soll zurückgegeben werden, aber nur dann wenn die spielfeld stelle frei ist

int t1[2] = {0, 0};
int t2[2] = {0, 1};
int t3[2] = {0, 2};
int t4[2] = {0, 3};
int t5[2] = {1, 0};
int t6[2] = {1, 1};
int t7[2] = {1, 2};
int t8[2] = {1, 3};
int t9[2] = {2, 0};
int t10[2] = {2, 1};
int t11[2] = {2, 2};
int t12[2] = {2, 3};
int t13[2] = {3, 0};
int t14[2] = {3, 1};
int t15[2] = {3, 2};
int t16[2] = {3, 3};

int * arrayTry[4][4] = {
	{t1, t2, t3, t4},
	{t5, t6, t7, t8},
	{t9, t10, t11, t12},
	{t13, t14, t15, t16}
};


//neuer ansatz: spielfeld checken und schauen welches feld frei ist, die freien felder in ein array werfen und daraus ein spielzug generieren.
int *select_row_column() {

	static int r[2];

	counter = 0;	//um zu zählen wie viele elemente in newArray sind
	int **newArray;
	if ((newArray = malloc(sizeof(int) * 32)) == NULL) {
		perror("line 137 thinker.c:: malloc error");
		exit(EXIT_FAILURE);
	}
	for (int a = 0; a < BOARD_SIZE; a++) {
		for (int b = 0; b < BOARD_SIZE; b++) {
			if (ptr->int_board[a][b] == FREE) {
				int aSpeicher = a; //speichert den wert von a aus der ersten for schleife
				int bSpeicher = b;// speichert den wert von b aus der zweiten for schleife
				//hier tritt der segmentationfault auf
				//soll über das newArray laufen, dewegen j < counter und and die stellen die elemente aus arrayTry packen die frei waren
				newArray[counter] = arrayTry[aSpeicher][bSpeicher];
				counter++; //erhoet counter wenn ein freies feld entdeckt wurde

			}
		}

	}
	//hier wird ein random wert generiert mit der begrenzung bei der newArray länge und daruas ein wert t1, t2 etc genommen
	srand(time(&t));

	if (counter == 0) {
		free(newArray);
		return NULL;
	}

	int randVal = (rand() % counter);
	r[0] = newArray[randVal][0]; // hier wird t1, oder t2 etc hinterlegt
	r[1] = newArray[randVal][1];
	// r ist wieder ein array mit 2 elementen, wie oben: t1, t2 etc.
	free(newArray);
	return r;
}


// puts the stone given to us by the server on the (previously checked) board field
char *place_stone(int stone) {

	char *line;
	char *row;
	static char move[3];


	while (1) {


		if (select_row_column() == NULL) {
			return NULL;
		}

		int* r = select_row_column();

		if (ptr->int_board[r[0]][r[1]] == FREE) {
			ptr->int_board[r[0]][r[1]] = stone;
			position[0] = r[0];
			position[1] = r[1];
			delete_stone(stone);

			// these two switch statements translate the final (chosen) value of our field spot into "server language"
			// for example we wanna place the stone at "01", so we send our move as "B4"

			switch (r[0]) {
			case 0:
				line = "4";
				break;
			case 1:
				line = "3";
				break;
			case 2:
				line = "2";
				break;
			case 3:
				line = "1";
				break;
			}

			switch (r[1]) {
			case 0:
				row = "A";
				break;
			case 1:
				row = "B";
				break;
			case 2:
				row = "C";
				break;
			case 3:
				row = "D";
				break;
			}

			strcpy(move, row);
			strcat(move, line);
			return move;
		}
	}
}

// super easy function: just chooses a random stone from the list until it finds one that is not used (not "8888" -> USED )
int choose_stone() {
	srand(time(&t));
	int stone;
	while (1)
	{
		int i = rand() % 16;
		if (unused_stones[i] != USED) {
			stone = unused_stones[i];
			delete_stone(stone);
			return stone;
		}
	}
}

// combines all of the above and makes our move into a string
// draw und winCheck kann man vielleicht in den gleichen if case packen, aber das hat zeit
char * superDummy(int next_stone) {

	char* placeVar = place_stone(next_stone);

	if ((placeVar == NULL)) {
		return spielzug;
	}

	strcpy(spielzug, placeVar);
	if (winCheck()) {

		return spielzug;
	}
	else if (detectDraw()) {
		//hier muss auch nur der spielzug gesendet werden, da es keinen stein mehr gibt zum übergeben
		return spielzug;
	}
	else {

		strcat(spielzug, ",");

		int choose_stn = choose_stone();
		char stone[2];
		sprintf(stone, "%d", choose_stn);

		strcat(spielzug, stone);
		return spielzug;
	}
}


// **********************************************************************************
// NEW FUNCTIONS, we can move them later to ausgabe_spielfeld so that it looks better


// This one takes the string that we have read from the "+ FIELD 4,4" and onward until "+ ENDFIELD"
// and converts it, line by line, into strings (lines of our field), that each get saved in the "char playing_field[BOARD_SIZE][BOARD_SIZE*3]"
// which is in shared memory. In the end we basically have this array with 4 lines which look like "* * * *" or "2 * 1 *" for example, if places are taken.
void update_board(char *board) {

	char *tok_ptr;
	tok_ptr = strtok(board, "\n");
	for (int lines = 0; lines < 4; lines++) {
		tok_ptr = strtok(NULL, "\n");

		if (strstr(tok_ptr, "+ 4") != NULL) {
			strcpy(ptr->playing_field[0], tok_ptr + 4);
		} else if (strstr(tok_ptr, "+ 3") != NULL) {
			strcpy(ptr->playing_field[1], tok_ptr + 4);
		} else if (strstr(tok_ptr, "+ 2") != NULL) {
			strcpy(ptr->playing_field[2], tok_ptr + 4);
		} else if (strstr(tok_ptr, "+ 1") != NULL) {
			strcpy(ptr->playing_field[3], tok_ptr + 4);
		} else {
			printf("Error by trying to read the board lines\n");
		}
	}

	printf("\033[1;31m");
	for (int s = 0; s < 4; s++) {
		printf("%s\n", ptr->playing_field[s]);
	}
	printf("\033[0m");

	update_int_board();
}

// Now this one takes the updated "char playing_field" from the above function and transforms the stars and number-strings into integers
// and saves them in a new structure element i added, two-dimensional int array, which we use for easier comparisons
// so now, after each turn, we have both the string lines and the integer lines at our disposal, that could be useful
// Also we dont need the "reset_board" function anymore, since this one does exactly that from the begining, but i left it here just in case something goes wrong
void update_int_board() {
	for (int line = 0; line < 4; line++) {
		char *tok_ptr;

		tok_ptr = strtok(ptr->playing_field[line], " ");
		for (int row = 0; row < 4; row++) {
			if (strstr(tok_ptr, "*") != NULL) {
				ptr->int_board[line][row] = -1;
			} else {
				ptr->int_board[line][row] = atoi(tok_ptr);
				delete_stone(ptr->int_board[line][row]);
			}
			tok_ptr = strtok(NULL, " ");
		}
	}

	// Just for testing the shared-memory int board, if its all ok, we will delete this later
	// Funny thing is, this is kinda like our ausgabeSpielfeld,
	// which is constantly updating.... we basically have that too now, unintentionally xD
	for (int a = 0; a < 4; a++) {
		for (int b = 0; b < 4; b++) {
			printf("%d ", ptr->int_board[a][b]);
		}
		printf("\n");
	}
	// Until here
}

// ############################################################
// Functions for win-checking
// ############################################################



// winCheck receives the position our client has chosen to put the stone to. It uses this position, which corresponds to the line and row,
// and extracts the corresponding line and row in our int_board. After that, it converts the numbers into binary properties and checks if we won
// If the position is on a diagonal line, it makes another check for that too.

//##############
//andis code lol
//##############

//detectDraw wird nur dann aufgerufen wenn wir den letzten stein aufs board setzen ohne dabei eine win chance zu haben.
//wenn keine -1 gefunden wird, dann ist es ein draw und keine stones mehr frei sind.
int sumOfArray(int n) {
	int sum = 0;
	for (int i = 0; i < n; i++) {
		sum += unused_stones[i];
	}
	return sum;
}

int detectDraw() {
	if (sumOfArray(15) != 133320) { //dann ist noch ein stein im array vorhanden
		return 0;
	}
	for (int a = 0; a < 4; a++) {
		for (int b = 0; b < 4; b++) {
			if (ptr->int_board[a][b] != -1) {
				return 1; // dann ist es ein draw

			}
		}
	}
	return 0; //return 1 wenn keine der beiden oben eintreten
}


int winCheck () {
	// converting the line, which corresponds to position[0], to binary and checking for win
	if (dec_to_bin_prop(ptr->int_board[position[0]])) {
		if (evaluate_properties()) {
			return 1;
		}
	}


	// converting the row, which corresponds to position[1], to binary and checking for win
	for (int i = 0; i < 4; i++) {
		line[i] = ptr->int_board[i][position[1]];
	}

	if (dec_to_bin_prop(line)) {
		if (evaluate_properties()) {
			return 1;
		}
	}


	// if the position is on a diagonal line, check that too
	if (position[0] == position[1]) {
		for (int i = 0; i < 4; i++) {
			line[i] = ptr->int_board[i][i];
		}
		if (dec_to_bin_prop(line)) {
			if (evaluate_properties()) {
				return 1;
			}
		}

	} else if ((position[0] + position[1]) == 3) {
		line[0] = ptr->int_board[0][3];
		line[1] = ptr->int_board[3][0];
		line[2] = ptr->int_board[1][2];
		line[3] = ptr->int_board[2][1];
		if (dec_to_bin_prop(line)) {
			if (evaluate_properties()) {
				return 1;
			}
		}
	}

	// return 0 if none of the above are wins
	return 0;
}


// Converts stones in respective columns and rows (and one diagonal if the position is also on a diagonal) so that we may
// use these properties to evaluate if we won
int dec_to_bin_prop(int *line) {
	for (int i = 0; i < 4; i++) {
		switch (line[i]) {
		case -1:
			return 0;
		case 0:
			binary_prop[i][0] = 0; binary_prop[i][1] = 0; binary_prop[i][2] = 0; binary_prop[i][3] = 0;
			break;
		case 1:
			binary_prop[i][0] = 0; binary_prop[i][1] = 0; binary_prop[i][2] = 0; binary_prop[i][3] = 1;
			break;
		case 2:
			binary_prop[i][0] = 0; binary_prop[i][1] = 0; binary_prop[i][2] = 1; binary_prop[i][3] = 0;
			break;
		case 3:
			binary_prop[i][0] = 0; binary_prop[i][1] = 0; binary_prop[i][2] = 1; binary_prop[i][3] = 1;
			break;
		case 4:
			binary_prop[i][0] = 0; binary_prop[i][1] = 1; binary_prop[i][2] = 0; binary_prop[i][3] = 0;
			break;
		case 5:
			binary_prop[i][0] = 0; binary_prop[i][1] = 1; binary_prop[i][2] = 0; binary_prop[i][3] = 1;
			break;
		case 6:
			binary_prop[i][0] = 0; binary_prop[i][1] = 1; binary_prop[i][2] = 1; binary_prop[i][3] = 0;
			break;
		case 7:
			binary_prop[i][0] = 0; binary_prop[i][1] = 1; binary_prop[i][2] = 1; binary_prop[i][3] = 1;
			break;
		case 8:
			binary_prop[i][0] = 1; binary_prop[i][1] = 0; binary_prop[i][2] = 0; binary_prop[i][3] = 0;
			break;
		case 9:
			binary_prop[i][0] = 1; binary_prop[i][1] = 0; binary_prop[i][2] = 0; binary_prop[i][3] = 1;
			break;
		case 10:
			binary_prop[i][0] = 1; binary_prop[i][1] = 0; binary_prop[i][2] = 1; binary_prop[i][3] = 0;
			break;
		case 11:
			binary_prop[i][0] = 1; binary_prop[i][1] = 0; binary_prop[i][2] = 1; binary_prop[i][3] = 1;
			break;
		case 12:
			binary_prop[i][0] = 1; binary_prop[i][1] = 1; binary_prop[i][2] = 0; binary_prop[i][3] = 0;
			break;
		case 13:
			binary_prop[i][0] = 1; binary_prop[i][1] = 1; binary_prop[i][2] = 0; binary_prop[i][3] = 1;
			break;
		case 14:
			binary_prop[i][0] = 1; binary_prop[i][1] = 1; binary_prop[i][2] = 1; binary_prop[i][3] = 0;
			break;
		case 15:
			binary_prop[i][0] = 1; binary_prop[i][1] = 1; binary_prop[i][2] = 1; binary_prop[i][3] = 1;
			break;
		default:
			printf("Error by converting decimal to binary\n");
			exit(EXIT_FAILURE);
		}
	}

	return 1;
}

// Uses the binary properties that we got from the "dec_to_bin_prop" function and evaluates them
int evaluate_properties () {
	int win = 0;

	for (int row = 0; row < 4; row++) {
		for (int property = 0; property < 4; property++) {
			win += binary_prop[property][row];
		}
		if (win == 0 || win == 4) return 1;
		win = 0;
	}
	return 0;
}

