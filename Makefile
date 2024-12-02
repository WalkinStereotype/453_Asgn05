CC = gcc
CFLAGS = -Wall -Wextra -g

all: minls minget

minls: minls.c 
	$(CC) $(CFLAGS) -o minls minls.c 

minget: minget.c 
	$(CC) $(CFLAGS) -o minget minget.c 

clean:
	rm -f minls minget *.o
