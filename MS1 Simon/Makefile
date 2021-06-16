CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -D_POSIX_C_SOURCE=2100112L -g -ggdb -O0

output: main.o performConnection.o config.o connector.o sharedmem.o thinker.o spielverlauf.o board_representation.o
	$(CC) $(CFLAGS) -o sysprak-client main.o performConnection.o config.o connector.o sharedmem.o thinker.o spielverlauf.o board_representation.o

performConnection.o: performConnection.h performConnection.c
	$(CC) $(CFLAGS) -c performConnection.c
main.o: main.c bashniclient.h
	$(CC) $(CFLAGS) -c main.c
config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c
connector.o: connector.c connector.h
	$(CC) $(CFLAGS) -c connector.c
sharedmem.o: sharedmem.c sharedmem.h sharedmemstruct.h
	$(CC) $(CFLAGS) -c sharedmem.c
thinker.o: thinker.c thinker.h
	$(CC) $(CFLAGS) -c thinker.c
spielverlauf.o: spielverlauf.c spielverlauf.h
	$(CC) $(CFLAGS) -c spielverlauf.c
board_representation.o: board_representation.c board_representation.h
	$(CC) $(CFLAGS) -c board_representation.c
play:
	./sysprak-client -g $(GAME_ID) -p $(PLAYER)


PHONY: clean
clean:
	-@rm -rf *.o sysprak-client 2>/dev/null || true
	
