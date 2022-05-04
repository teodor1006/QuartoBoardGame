#include "sharedMemory.h"

#define _XOPEN_SOURCE

#include<sys/types.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>

/*int shm_id(int size) {                                                                  //Anlegen von SHM

	int shm_id;
	shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
	if (shm_id < 0) {
		perror("SHM anlegen fehlgeschlagen! \n");
		return -1;
	}
	return shm_id;
}*/

void *address_shm(int shm_id) {                                                       // Info an SHM anbinden
	void *address_shm_ptr;
	address_shm_ptr = shmat (shm_id, NULL, 0);
	if (address_shm_ptr == (void*) - 1) {
		perror("SHM anbinden fehlgeschlagen! \n");
	}
	return address_shm_ptr;
}

int *shm_spielfeld_anbinden(int shm_id) {
	//int *shm_spielfeld_anbinden;
	int *players_shm_ids;
	players_shm_ids = shmat (shm_id, NULL, 0);
	if (players_shm_ids == (void*) - 1) {
		perror("SHM anbinden fehlgeschlagen! \n");
	}
	return players_shm_ids;
}


/*int dettach_shm (void *address) {                                                  //Loslösen von SHM
	if (shmdt (address) < 0) {
		perror("SHM loslösen fehlgeschlagen! \n");
		return -1;
	}
	return 0;
}*/

int delete_shm (int shm_id) {                                                      //Löschen von SHM
	if (shmctl (shm_id, IPC_RMID, 0) < 0) {
		perror("SHM loeschen fehlgeschlagen! \n");
		return -1;
	}
	return 0;

}