dungeonGame: dungeonGame.o buildDungeon.o heap.o pathFinding.o character.o move.o io.o
	g++ -lncurses buildDungeon.o dungeonGame.o heap.o pathFinding.o character.o move.o io.o -o dungeonGame

buildDungeon.o: buildDungeon.cpp dungeonInfo.h buildDungeon.h
	g++ -c buildDungeon.cpp -Wall -Werror -ggdb -g3 -o buildDungeon.o

dungeonGame.o: dungeonGame.cpp dungeonInfo.h buildDungeon.h heap.h
	g++ -c dungeonGame.cpp -Wall -Werror -ggdb -g3 -o dungeonGame.o

pathFinding.o: pathFinding.cpp pathFinding.h dungeonInfo.h heap.h
	g++ -c pathFinding.cpp -Wall -Werror -ggdb -g3 -o pathFinding.o

character.o: character.cpp dungeonInfo.h pathFinding.h buildDungeon.h
	g++ -c character.cpp -Wall -Werror -ggdb -g3 -o character.o

move.o: move.cpp move.h dungeonInfo.h pathFinding.h buildDungeon.h
	g++ -c move.cpp -Wall -Werror -ggdb -g3 -o move.o

heap.o: heap.cpp heap.h macros.h
	g++ -c heap.cpp -Wall -Werror -ggdb -o heap.o

io.o: io.cpp io.h dungeonInfo.h buildDungeon.h move.h
	g++ -c io.cpp -Wall -Werror -ggdb -g3 -o io.o
clean:
	rm -f buildDungeon.o dungeonGame.o heap.o pathFinding.o character.o dungeonGame
