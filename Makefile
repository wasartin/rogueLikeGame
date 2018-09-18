dungeonGame: dungeonGame.o buildDungeon.o heap.o
	gcc buildDungeon.o dungeonGame.o heap.o -o dungeonGame

buildDungeon.o: buildDungeon.c dungeon.h buildDungeon.h
	gcc -c buildDungeon.c -Wall -Werror -ggdb -o buildDungeon.o

dungeonGame.o: dungeonGame.c dungeon.h buildDungeon.h heap.h
	gcc -c dungeonGame.c -Wall -Werror -ggdb -o dungeonGame.o

heap.o: heap.c heap.h macros.h
	gcc -c heap.c -Wall -Werror -ggdb -o heap.o

clean:
	rm -f buildDungeon.o dungeonGame.o heap.o dungeonGame
