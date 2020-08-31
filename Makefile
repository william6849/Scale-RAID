all:
	gcc -g -fno-inline -std=gnu99 -o run runner.c scaleoutmanager.c console.c -I.

memcheck:
	valgrind --leak-check=full --show-reachable=yes ./run

callgrind:
	valgrind --tool=callgrind ./run
