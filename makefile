
all: kapish.c
	gcc -g -Wall -Werror -std=c11 -pedantic-errors kapish.c -o kapish
clean:
	rm -f $(binaries) *.o
