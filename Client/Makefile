#---------------------------------------------
# Name: Kashish Mittal, Tanushree Goyal
# NAME ID #: 1638174, 1639635
# CMPUT 275 Winter 2021
#
# Assignemnt 1: Part 1
#---------------------------------------------
# Makefile contents here
OBJ = client.o
LINK_TARGET = client
REBUILDABLES = $(OBJ) $(LINK_TARGET)
#root_dir = part2_starter
CC = g++
CC_FLAGS = -Wall -O2 -g 

all: $(LINK_TARGET)

$(LINK_TARGET): $(OBJ)
	$(CC) -o $(LINK_TARGET) $(OBJ)

client.o: client.cpp 
	 $(CC) $(CC_FLAGS) -c client.cpp 

clean:
	rm -f $(REBUILDABLES)
