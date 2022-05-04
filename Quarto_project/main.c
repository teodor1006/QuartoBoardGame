#define _POSIX_c_SOURCE 2

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>


#include "connection.h"
#include "communication.h"
#include "config.h"
#include "sharedMemory.h"
#include "ausgabe_spielfeld.h"
#include "thinker.h"

void printHelp() {
	printf("\n%46s", "### Program Manual ###\n\n");
	printf("%s", "Structure of the call:  sysprak-client -g <Game-ID> -p <0|1> -c <configuration file>\n");
	printf("\nOption -g :  Enter a 13-character long Game-ID\n");
	printf("Option -p(optional) :  Enter the player number (0 for player one or 1 for player two)\n");
	printf("Option -c(optional) :  Enter the name of the configuration file you want to use (with extension)\n\n");
}

#define ID_LENGTH 13
#define PLAYER_NUM_LEN 1


struct game_info *shm_ptr;
int intro = 1;
int gamename_flag = 0;


// Signal Handler um SIGUSR1 und ctr + c abzufangen
void handle_signal(int sig) {

	switch (sig) {
	case SIGUSR1:
		if (shm_ptr->flag == 0) {
			return;
		}
		printf("Flag set. Start thinking\n");
		shm_ptr->flag = 0;
		break;

	case SIGCHLD:

		if (shm_ptr->gameover == 1) {
			reset_board();
			update_board(shm_ptr->temp_field);
			int_Board_To_char_Board();
			print_board();
			free_board();

			if (shm_ptr->winner == 0 && shm_ptr->winner2) {
				printf("\033[1;36m");
				printf("\nGameover! Its a draw! Both %s and %s won!\nOr lost, depending on your philosophical views...\n", shm_ptr->me.name, shm_ptr->opponent.name);
				printf("\033[0m");
			} else {
				printf("\033[1;36m");
				if (shm_ptr->winner == shm_ptr->me.player_nr) {
					printf("\nGameover! %s won!\n", shm_ptr->me.name);
				} else printf("\nGameover! %s won!\n", shm_ptr->opponent.name);
				printf("\033[0m");
			}

			exit(EXIT_SUCCESS);

		} else {
			perror("SIGCHLD error:");
			exit(EXIT_FAILURE);
		}
	}

}


int main(int argc, char *argv[]) {


	int opt;
	char *readLine;
	char game_id[ID_LENGTH + 1] = "", player[PLAYER_NUM_LEN + 1] = "";

	struct configuration conf;
	conf.hostname[0] = '\0';
	conf.portnumber[0] = '\0';

	if (argc == 1) {
		printf("\n* Wrong initiation: Please read the manual *\n");
		printHelp();
		exit(EXIT_FAILURE);
	}

	for (int argument = 1; argument < argc; argument++) {
		if (strncmp(argv[argument], "-g", 2) == 0 ) {
			argument = argc;
		}

		if (argument == argc - 1) {
			printf("\n* Wrong initiation: Please read the manual *\n");
			printHelp();
			exit(EXIT_FAILURE);
		}
	}

	while ((opt = getopt(argc, argv, "g:p:c:")) != -1) {
		switch (opt) {
		case 'g':
			if (strlen(optarg) != ID_LENGTH) {
				printf("\n* Game-ID must be exactly 13 characters long *\n");
				printHelp();
				exit(EXIT_FAILURE);
			}
			if (strlen(game_id) < ID_LENGTH) {
				strncat(game_id, optarg, ID_LENGTH);
			} else {
				printf("\n* Argument \"-g\" was included multiple times as a parameter *\n");
				printHelp();
				exit(EXIT_FAILURE);
			}
			break;

		case 'p':
			if (!isdigit(*optarg) || (atoi(optarg) != 0 && atoi(optarg) != 1)) {
				printf("\n* Player number has to be either <0> or <1> *\n");
				printHelp();
				exit(EXIT_FAILURE);
			}

			if (strlen(player) < PLAYER_NUM_LEN) {
				strncat(player, optarg, PLAYER_NUM_LEN);
			} else {
				printf("\n* Argument \"-p\" was included multiple times as a parameter *\n");
				printHelp();
				exit(EXIT_FAILURE);
			}

			break;

		case 'c':
			conf = read_conf_file(optarg);
			break;

		default:
			fprintf(stderr, "\n* Please read the manual: * \n");
			printHelp();
			exit(EXIT_FAILURE);
		}
	}

	if (conf.hostname[0] == '\0' || conf.portnumber[0] == '\0') conf = read_conf_file("config.conf");





	int shm_id = shmget (IPC_PRIVATE, sizeof(struct game_info), IPC_CREAT | 0666);
	if (shm_id < 0) {
		perror("SHM anlegen fehlgeschlagen! \n");
		exit(EXIT_FAILURE);
	}
	shm_ptr = address_shm(shm_id);
	delete_shm(shm_id);



	char inbuf[BUFFSIZE];
	char * spielzug;

	int pipe_fd[2];
	pid_t pid;



	if (pipe(pipe_fd) < 0) {
		perror ("Fehler beim Erstellen von Pipe");
		exit(EXIT_FAILURE);
	}

	// Divide into Connector and Thinker


	switch (pid = fork()) {
	case -1:
		perror("Fehler bei fork()\n");
		break;

	case 0: //Connector

		close(pipe_fd[1]);    // closing write end of file

		shm_ptr->flag = 0;

		shm_ptr->pid1 = pid;


		performConnection(conf.hostname, conf.portnumber);

		int sock_fd = getSocketFd();


		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sock_fd, &readfds);
		int max = sock_fd;
		FD_SET(pipe_fd[0], &readfds);

		if (pipe_fd[0] > max) {
			max = pipe_fd[0];
		}




		while (1) {


			readLine = readSock(sock_fd);


			if (intro) {
				if (gamename_flag && (strncmp("+ Game from", readLine, 11) != 0)) {
					strncat(shm_ptr->game_name, readLine + 2, NAMELEN - 1);
					if ((strlen(readLine + 2)) > NAMELEN - 1) {
						printf("Game name too long: part of it had to be cut out\n");
					}
					int len = strlen(shm_ptr->game_name);
					shm_ptr->game_name[len - 1] = '\0';
					gamename_flag = 0;
					readLine = readSock(sock_fd);
				} else {
					gamename_flag = 0;
				}

				if (strncmp("+ MNM", readLine, 5) == 0) {
					writeSock(sock_fd, getVersion());

				}  else if (strncmp("+ Client", readLine, 8) == 0) {
					writeSock(sock_fd, "ID ");
					writeSock(sock_fd, game_id);
					writeSock(sock_fd, "\n");

				} else if (strncmp("+ PLAYING", readLine, 9) == 0) {
					if (strncmp("Quarto", &readLine[10], 5) != 0) {
						printf("%s\n", &readLine[10]);
						printf("Wrong game initiated: can only play Quarto\n");
						exit(EXIT_FAILURE);
					} else {
						strcpy (shm_ptr->game_type, "Quarto");
						gamename_flag = 1;
					}

					writeSock(sock_fd, "PLAYER");
					if (strlen(player) > 0) {
						writeSock(sock_fd, " ");
						writeSock(sock_fd, player);
					}
					writeSock(sock_fd, "\n");

				} else if (strncmp("+ YOU", readLine, 5) == 0) {

					shm_ptr->me.player_nr = readLine[6] - '0';
					strncat(shm_ptr->me.name, &readLine[8], NAMELEN - 1);
					if ((strlen(&readLine[8])) > NAMELEN - 1) {
						printf("Player %d name too long: part of it had to be cut out\n", shm_ptr->me.player_nr + 1);
					}
					int me_name_len = strlen(shm_ptr->me.name);
					shm_ptr->me.name[me_name_len - 1] = '\0';


					readLine = readSock(sock_fd);
					shm_ptr->totalplayers = readLine[8] - '0';

					readLine = readSock(sock_fd);
					shm_ptr->opponent.player_nr = readLine[2] - '0';
					int cut_out_name = strlen(readLine) - 6;
					if (cut_out_name > NAMELEN - 1) {
						cut_out_name = NAMELEN - 1;
						printf("Player %d name too long: part of it had to be cut out\n", shm_ptr->opponent.player_nr + 1);
					}
					strncat(shm_ptr->opponent.name, &readLine[4], cut_out_name);
					int opp_name_len = strlen(shm_ptr->opponent.name);
					shm_ptr->opponent.name[opp_name_len - 1] = '\0';
					shm_ptr->opponent.registered = (readLine[strlen(readLine) - 2] - '0');



				} else if (strstr(readLine, "+ ENDPLAYERS") != 0) {
					printf("\033[1;32m");
					printf("Playing %s\n", shm_ptr->game_type);
					printf("There are %d players in total\n", shm_ptr->totalplayers);
					if (strlen(shm_ptr->game_name) > 0) {
						printf("Game name: %s\n", shm_ptr->game_name);
					}
					printf("You are player %d\n", (shm_ptr->me.player_nr) + 1);
					printf("Your opponent is player %d\n", (shm_ptr->opponent.player_nr) + 1);
					if (shm_ptr->opponent.registered) {
						printf("Your opponent is registered\n");
						printf("\nHave fun\n\n");
					} else printf("Your opponent is not yet registered\n\n");
					printf("\033[0m");
					intro = 0;

				}

			} else {

				if (strstr(readLine, "+ MOVE") != 0) {
					if (strstr(readLine, "+ MOVEOK")) {
						continue;
					}
					char * timepointer = strstr(readLine, "+ MOVE");
					char str_time[4];
					strcpy(str_time, timepointer + 7);
					int time = atoi(str_time);
					shm_ptr->timeout = time;
					printf("Time wurde abgespeichert: %d\n", shm_ptr->timeout);

				} else if (strstr(readLine, "+ FIELD") != 0) {
					char * fieldpointer = strstr(readLine, "+ FIELD");
					char str_field[5];
					strcpy(str_field, fieldpointer + 8);
					char *width_str_pointer = strtok(str_field, ",");
					int width = atoi(width_str_pointer);
					shm_ptr->width = width;
					printf("Width wurde abgespeichert:%d\n", shm_ptr->width);
					char *height_str_pointer = strtok(NULL, ",");
					int height = atoi(height_str_pointer);
					shm_ptr->height = height;
					printf("Height wurde abgespeichert:%d\n", shm_ptr->height);


					// this one's new, it saves the "+ FIELD 4,4..." and onward, in shared memory
					strcpy(shm_ptr->temp_field, fieldpointer);
					/*printf("\033[1;31m");
					printf("\n\nTHIS IS IN SHM:\n%s\n\n", shm_ptr->temp_field);
					printf("\033[0m");*/

				} else if (strstr(readLine, "+ NEXT") != 0) {
					char * Steinpointer = strstr(readLine, "+ NEXT");
					char str_Stein[2];
					strcpy(str_Stein, Steinpointer + 7);
					int next;
					next = atoi(str_Stein);
					shm_ptr->next_stone = next;
					printf("Stein wurde abgespeichert:%d\n", shm_ptr->next_stone);

				} else if (strncmp(readLine, "+ GAMEOVER", 10) == 0) {
					shm_ptr->gameover = 1;

				} else if (strstr(readLine, "+ ENDFIELD") != NULL) {
					if (shm_ptr->gameover != 1) {
						writeSock(sock_fd, "THINKING\n");
						shm_ptr->flag = 1;
						kill(getppid(), SIGUSR1);

						for (int i = 0; i < 5; i++) {
							readLine = readSock(sock_fd);

							if (strstr(readLine, "+ OKTHINK") != 0) {

								//sleep(3);
								int activity;
								activity = select(max + 1, &readfds, NULL, NULL, NULL);
								if (activity == -1) {
									perror("Select Error!\n");
								} else if (activity) {
									if (FD_ISSET(pipe_fd[0], &readfds)) {
										if ((read(pipe_fd[0], inbuf, BUFFSIZE)) < 0) {
											perror("pipe read error");
											exit(EXIT_FAILURE);
										}

										sendMove(sock_fd, inbuf);
										printf("Finished reading\n");
									} else if (FD_ISSET(sock_fd, &readfds)) {
										printf("Error beim Socket!\n");
									}
								}
								break;
							}
						}
					}

				} else if (strstr(readLine, "Yes") != NULL) {
					char *helpstr;
					char winner[1]=  "";
					helpstr = strstr(readLine, "Yes");
					*(winner) = *(helpstr - 5);
					shm_ptr->winner = atoi(winner);

					if (shm_ptr->winner == 0) {

						readLine = readSock(sock_fd);
						if (strstr(readLine, "Yes") != NULL) shm_ptr->winner2 = 1;

					} else shm_ptr->winner2 = 0;


				} else if (strstr(readLine, "+ WAIT") != 0) {
					writeSock(sock_fd, "OKWAIT\n");

				} else if (strncmp("+ QUIT", readLine, 6) == 0) {
					disconnect();
					exit(EXIT_SUCCESS);

				} else if (strncmp("-", readLine, 1) == 0) {
					exit(EXIT_FAILURE);

				}  else if (readLine[0] == '\0') {
					printf("\033[0;31m");
					printf("i shouldn't be here...\n");
					printf("\033[0m");
					exit(EXIT_FAILURE);

				}
			}
		}



	default: // Thinker


		close(pipe_fd[0]);    // closing read end of file


		shm_ptr->pid0 = pid;

		signal(SIGUSR1, handle_signal);
		signal(SIGCHLD, handle_signal);


		attach_Thinker(shm_id);

		while (1) {

			pause();
			// i put the update_board here, don't ask why....at first i wanted it to be in the IF statement, where "strcpy(shm_ptr->temp_field, fieldpointer);"
			// is standing now, but there were so many problems that came with that logic that i went crazy trying to figure it out
			// have to ask skruppy about that, the problems were extremely mind f***ing xD
			// so it stands here now, and updates the field right before it checks for a valid move to play
			reset_board();

			update_board(shm_ptr->temp_field);
			spielzug = superDummy(shm_ptr->next_stone);
			printf("Thinker hat schon gedacht!\n");
			if ((write(pipe_fd[1], spielzug, BUFFSIZE)) < 0) {
				perror("pipe write error");
				exit(EXIT_FAILURE);
			}


			int_Board_To_char_Board();

			print_board();
			free_board();
		}


		if (waitpid(pid, NULL, 0) != pid) {
			perror ("Fehler beim Warten auf Kindprozess\n");
			return EXIT_FAILURE;
		}

		break;
	}
}


