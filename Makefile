all: 3dbpp

3dbpp:
	gcc -ansi -o 3dbpp -O5 3dbpp.c test3dbpp.c -lm
	
clean:
	rm -f 3dbpp