all: createdb

createdb: createdb.o
	gcc createdb.o -lsqlite3 -o createdb

createdb.o: createdb.c
	gcc -c createdb.c

clean:
	rm -rf *o createdb
