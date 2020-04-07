all: main.o error.o list.o tree.o exec.o 
	gcc -o shell main.o error.o list.o tree.o exec.o 
	./shell
list.o:	list.c list.h exec.h
	gcc -c list.c
tree.o: tree.c tree.h list.o 
	gcc -c tree.c
exec.o: exec.c exec.h tree.o 
	gcc -c exec.c
error.o: error.c tree.h list.h exec.h
	gcc -c error.c
main.o: main.c tree.h exec.h list.h
	gcc -c main.c

clean:
	rm -rf shell *.o 
run:
	./shell