
all: kapish.c
	gcc -g -Wall -pedantic-errors kapish.c -o kapish
clean:
	rm -f $(binaries) *.o
