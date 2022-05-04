CC = gcc
CFLAGS = -g -Wall -Wextra -Werror
SRCFILES = main.c communication.c config.c connection.c sharedMemory.c ausgabe_spielfeld.c thinker.c
TARGET = ./sysprak-client
GAME_ID ?= 1l0yrqvt1g3ve
PLAYER ?= 0
GAME_CONFIG ?= config.conf

.PHONY: clean val play

#.SILENT:
sysprak-client: $(SRCFILES)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o $(TARGET)	      	   	   	   

val:
	valgrind --leak-check=full --trace-children=yes $(TARGET) -g $(GAME_ID) -p $(PLAYER) -c $(GAME_CONFIG)

play:
	$(TARGET) -g $(GAME_ID) -p $(PLAYER) -c $(GAME_CONFIG)

