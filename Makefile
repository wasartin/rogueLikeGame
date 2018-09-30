dungeonGame: dungeonGame.o buildDungeon.o heap.o pathFinding.o
	gcc buildDungeon.o dungeonGame.o heap.o pathFinding.o -o dungeonGame

buildDungeon.o: buildDungeon.c dungeonInfo.h buildDungeon.h
	gcc -c buildDungeon.c -Wall -Werror -ggdb -o buildDungeon.o

dungeonGame.o: dungeonGame.c dungeonInfo.h buildDungeon.h heap.h
	gcc -c dungeonGame.c -Wall -Werror -ggdb -o dungeonGame.o

pathFinding.o: pathFinding.c pathFinding.h dungeonInfo.h heap.h
	gcc -c pathFinding.c -Wall -Werror -ggdb -o pathFinding.o

heap.o: heap.c heap.h macros.h
	gcc -c heap.c -Wall -Werror -ggdb -o heap.o

clean:
	rm -f buildDungeon.o dungeonGame.o heap.o pathFinding.o dungeonGame
