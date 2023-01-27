#---------------------------------------------
# Name: Kashish Mittal, Tanushree Goyal
# NAME ID #: 1638174, 1639635
# CMPUT 275 Winter 2021
#
# Assignemnt 1: Part 1
#---------------------------------------------
# Makefile contents here
OBJ = server.o digraph.o dijkstra.o 
#OBJ= sample.o
LINK_TARGET = server
REBUILDABLES = $(OBJ) $(LINK_TARGET)

CC = g++
CC_FLAGS = -Wall -O2 -g 

all: $(LINK_TARGET)

$(LINK_TARGET): $(OBJ)
	$(CC) -o $(LINK_TARGET) $(OBJ)

server.o: server.cpp digraph.h wdigraph.h dijkstra.h
	$(CC) $(CC_FLAGS) -c server.cpp

digraph.o: digraph.cpp wdigraph.h
	$(CC) $(CC_FLAGS) -c digraph.cpp


dijkstra.o: dijkstra.cpp heap.h
	$(CC) $(CC_FLAGS) -c dijkstra.cpp

clean:
	rm -f $(REBUILDABLES)