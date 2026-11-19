CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g
CPPFLAGS = -I./include/
LDLIBS = -lncursesw
SOURCES = src/*.c

all: compile run clean

compile: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(CPPFLAGS) $(LDLIBS) -o game

test: tests/test_floor.c src/map.c src/room.c src/structs.c src/entity.c src/time.c src/log.c src/draw.c
	$(CC) $(CFLAGS) tests/test_floor.c src/map.c src/room.c src/structs.c src/entity.c src/time.c src/log.c src/draw.c $(CPPFLAGS) $(LDLIBS) -o test_runner
	./test_runner
	rm -f test_runner

run: ./game
	./game

clean:
	rm -f game test_runner
