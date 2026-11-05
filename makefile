CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g
CPPFLAGS = -I./include/
LDLIBS = -lncurses
SOURCES = src/*.c

all: compile run clean

compile: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(CPPFLAGS) $(LDLIBS) -o game

run: ./game
	./game

clean:
	rm -f game
