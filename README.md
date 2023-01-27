# ServerClientGUIPlotter

Included Files:
                > Server
                    *Makefile
                    *server.cpp
                    *dijkstra.cpp
                    *dijkstra.h
                    *heap.h
                    *wdigraph.h
                    *digraph.cpp
                    *digraph.h

                > Client
                    * Makefile
                    *client.cpp

                > README   
                > Makefile
                > plotter
                > link_emulator

                    

Accessories: None
Wiring Instructions: None

Main Makefile:
  make all: calls the make all command in both server and client makefile
  make clean: calls the make clean command in both server and client makefile

Server Makefile Targets:
  make all: Default target and compiles the entire program
  make server.o: Compiles server.cpp
  make digraph.o: Compiles digraph.cpp
  make dijkstra.o: Compiles dijkstra.cpp
  make server: Reads a request from input and prints the output to mysol.txt
  make clean: Clears all the object files and executables

Client Makefile Targets:
  make all: Default target and compiles the entire program
  make client.o: Compiles client.cpp

Notes and Assumptions:

    > Server.cpp
        * Inside the main, all the functions to create, setup and connect sockets
          have been added.
        * Instead of stdin and stdout, sockets were used to send and receive
          waypoints.
        * Instead of list, binary heap was used to store the waypoints.


    > Client.cpp
        * Inside the main, sockets were created to receive the waypoints and
          connection was established with the server.
        * The coordinates were read from inpipe and written to the socket 
        * If Q is encountered, take the request from the user again. 
        * Then first recieve the Number of waypoint from the server and 
          acknowledment was sent
        * Then waypoint are recieved from socket and written to the plotter and
        * another acknowledment was sent
        * Process was continued until the end of waypoint or timeout whichever 
        * occoured first
        * Close the pipes and the sockets.
    
