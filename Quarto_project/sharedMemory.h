#ifndef sharedMemory
#define sharedMemory

#include <sys/types.h>

#define NAMELEN 50
#define BOARD_SIZE 4
#define FIELD_BUFFER 100



struct player_info {
	int player_nr;
	char name[NAMELEN];
	int registered;                            // is the player registered/bereit or not

};

struct game_info {
	int flag; //testweise für SignalHandling
	char game_type[NAMELEN];
	char game_name[NAMELEN]; //abgespeichert
	int totalplayers;
	int timeout;
	pid_t pid0; //Thinker
	pid_t pid1; //Connector
	size_t move;
	size_t try_next_piece;
	int next_stone;
	int height;
	int width;
	char playing_field[BOARD_SIZE][BOARD_SIZE * 3]; // mal 3, weil es ein ganzes line speichert (z.B. + 4 * * * *)
	int winner;
	int winner2;
	int gameover;
	int players_shm_ids;
	int *shm_ptr_field;
	//struct player_info
	struct player_info me;
	struct player_info opponent;
	//new additions to the structure
	int int_board[BOARD_SIZE][BOARD_SIZE];
	char temp_field[FIELD_BUFFER];
};



int shm_id (int size);

void *address_shm(int shm_id);

int *shm_spielfeld_anbinden(int shm_id);

int dettach_shm (void *address);

int delete_shm (int shm_id);

#endif

