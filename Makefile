CC = gcc
CFLAGS = -Wall -Wextra -g

all: minls minget

minls: minls.c helpers.c
	$(CC) $(CFLAGS) -o minls minls.c helpers.c

minget: minget.c 
	$(CC) $(CFLAGS) -o minget minget.c 

clean:
	rm -f minls minget *.o
