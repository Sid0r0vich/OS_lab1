all:
	gcc -Wall -Wextra -Werror main.c -c main.o
	gcc main.o -o main

clean:
	rm -rf main
	rm -rf main.o
