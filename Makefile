CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -D_POSIX_C_SOURCE=2100112L

output: main.o performConnection.o config.o connector.o shMem.o
	$(CC) $(CFLAGS) -o sysprak-client main.o performConnection.o config.o connector.o shMem.o

shMem.o: shMem.c shMem.h
	$(CC) $(CFLAGS) -c shMem.c
connector.o: connector.c connector.h
	$(CC) $(CFLAGS) -c connector.c
config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c
performConnection.o: performConnection.c performConnection.h
	$(CC) $(CFLAGS) -c performConnection.c
main.o: main.c main.h shMemStrct.h
	$(CC) $(CFLAGS) -c main.c
	
play:
	./sysprak-client -g $(GAME_ID) -p $(PLAYER)
clean:
	rm sysprak-client *.o
