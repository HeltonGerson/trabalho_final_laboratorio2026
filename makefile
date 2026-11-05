CC = gcc
CFLAGS = -Wall -lncurses
CPPFLAGS = -I./include/
LDLIBS = -lncurses
SOURCES = src/*.c

all: compile run clean

compile: $(SOURCES)
	$(CC) $(SOURCES) $(CPPFLAGS) $(LDLIBS) -o game

run: ./game
	./game

clean:
	rm -f game
