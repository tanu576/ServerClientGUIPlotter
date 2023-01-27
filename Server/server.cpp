#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <list>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <netdb.h>
#include <cstdlib>	
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "wdigraph.h"
#include "dijkstra.h"
#include "heap.h"

#define PORT 8888
typedef pair<long long , long long > PAIR;
struct Point {
    long long lat, lon;
};

// returns the manhattan distance between two points
long long manhattan(const Point& pt1, const Point& pt2) {
  long long dLat = pt1.lat - pt2.lat, dLon = pt1.lon - pt2.lon;
  return abs(dLat) + abs(dLon);
}

// finds the point that is closest to a given point, pt
int findClosest(const Point& pt, const unordered_map<int, Point>& points) {
  pair<int, Point> best = *points.begin();

  for (const auto& check : points) {
    if (manhattan(pt, check.second) < manhattan(pt, best.second)) {
      best = check;
    }
  }
  return best.first;
}

// reads graph description from the input file and builts a graph instance
void readGraph(const string& filename, WDigraph& g, unordered_map<int, Point>& points) {
  ifstream fin(filename);
  string line;

  while (getline(fin, line)) {
    // split the string around the commas, there will be 4 substrings either way
    string p[4];
    int at = 0;
    for (auto c : line) {
      if (c == ',') {
        // starting a new string
        ++at;
      }
      else {
        // appending a character to the string we are building
        p[at] += c;
      }
    }

    if (at != 3) {
      // empty line
      break;
    }

    if (p[0] == "V") {
      // adding a new vertex
      int id = stoi(p[1]);
      assert(id == stoll(p[1])); // sanity check: asserts if some id is not 32-bit
      points[id].lat = static_cast<long long>(stod(p[2])*100000);
      points[id].lon = static_cast<long long>(stod(p[3])*100000);
      g.addVertex(id);
    }
    else {
      // adding a new directed edge
      int u = stoi(p[1]), v = stoi(p[2]);
      g.addEdge(u, v, manhattan(points[u], points[v]));
    }
  }
}

// Keep in mind that in Part I, your program must handle 1 request
// but in Part 2 you must serve the next request as soon as you are
// done handling the previous one
int main(int argc, char* argv[]) {
  WDigraph graph;
  unordered_map<int, Point> points;
  // build the graph
  readGraph("edmonton-roads-2.0.1.txt", graph, points);
  char c;
  Point sPoint, ePoint;
  int server_fd, new_socket_fd; 
  struct sockaddr_in address, client_address; 
  int addrlen = sizeof(struct sockaddr_in);
  char packet[2][2048] = {0}; 
       
  // creating a socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) { 
    cout << "socket creation failed";
    return 1;
  }

  // set up the socket (initialize socket parameters)
  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
  address.sin_port = htons(PORT); 
  int cnt = 1;
  // binding to port 8888
  if (::bind(server_fd, (struct sockaddr *)&address, addrlen) == -1) { 
    cout << "Unable to bind to port address"; 
    return 1;
  }
  cout<<"Bind Successfull"<<endl;

  // listen for connection requests
  if (listen(server_fd, 10) == -1) { 
    cout << "Listen failed";
    return 1;
  }
  int i = 0;
  cout << "Listening to " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << endl;

  // accept a connection request
  new_socket_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);
  cout << "Connection request accepted from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << endl;
  long long lat;
  long long lon;
  struct timeval timer = {.tv_sec = 1000, .tv_usec = 0};
  if (setsockopt(new_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (void *) &timer, sizeof(timer)) == -1) {
	  std::cerr << "Cannot set socket options!\n";
	  return 1;
	}
  while (true) {
    char ack[2048] = {0};
    // blocking call - blocks until a message arrives 
    // (unless O_NONBLOCK is set on the socket's file descriptor)
    // recieving start coordinates
    int rec_size = recv(new_socket_fd, packet[0], 22, 0);
    string a(packet[0]);
    string z = a.substr(0,1);
    // checking for Q
    if (z.compare("Q") == 0) {
      std::cout << "Connection will be closed\n";
      break;
    }
    // recieving end cordinates
    rec_size = recv(new_socket_fd, packet[1], 22, 0);
    std::cout << "Message received\n";
    //cout<<packet[0]<<packet[1];
    double lat_d = stod(a.substr(0,9));
    double lon_d = stod(a.substr(10, 21));
    lat = static_cast<long long>(lat_d*100000);
    lon = static_cast<long long>(lon_d*100000);
    sPoint.lat = lat;
    sPoint.lon = lon;
    string b(packet[1]);
    lat_d = stod(b.substr(0,9));
    lon_d = stod(b.substr(10, 21));
    lat = static_cast<long long>(lat_d*100000);
    lon = static_cast<long long>(lon_d*100000);
    ePoint.lat  = lat;
    ePoint.lon = lon;
    // get the points closest to the two points we read
    int start = findClosest(sPoint, points), end = findClosest(ePoint, points);
    unordered_map<int, PIL> tree;
    // run dijkstra's, this is the unoptimized version that does not stop
    // when the end is reached but it is still fast enough
    dijkstra(graph, start, tree);
    if (tree.find(end) == tree.end() || start == end) {
      //cout<<"N 0"<<endl;
      send(new_socket_fd, "E\n", 2,0);
      continue;
    }
      int num = std::numeric_limits<int>::max();
      BinaryHeap <PAIR, int> waypoint;
      // read off the path by stepping back through the search tree
      waypoint.insert(PAIR(points[end].lat,points[end].lon), num);
      int next = tree[end].first;
      //back track from end vertex and store in the binary heap.
      while (next != start){
        waypoint.insert(PAIR(points[next].lat,points[next].lon),--num);
        next = tree[next].first;
      }
      waypoint.insert(PAIR(points[start].lat,points[start].lon),--num);
      int cnt = waypoint.size();                               
      string s = "N ";
      s += to_string(cnt) + '\n';
      int n = s.length();
      char numarr[n+1];
      strcpy(numarr,s.c_str());
      // sending number of waypoints	
      send(new_socket_fd, numarr, n+1, 0);
      // recieving acknowledment
      rec_size = recv(new_socket_fd, ack, 2,0);
      // check for timeout
      if (rec_size == -1){
          cout<<"timeout"<<endl;
          continue;
      }
      string chk(ack);
      chk = chk.substr(0,1);
      while(waypoint.size() != 0 && chk.compare("A") == 0){
        string w = "W ";
        // converting waypoint to required form
        w += to_string(waypoint.min().item.first)+" "+ to_string(waypoint.min().item.second) + '\n';
        int n = w.length();
        char numarr2[n+1];
        strcpy(numarr2,w.c_str());
        send(new_socket_fd, numarr2, n+1, 0);
	      rec_size = recv(new_socket_fd, ack, 2,0);
        if (rec_size == -1){
                cout<<"Timeout"<<endl;
                break;
        }
      	string ch(ack);
      	chk = ch.substr(0,1);
        waypoint.popMin();
      }
		send(new_socket_fd, "E\n", 2, 0);
      
  }
	// close socket descriptors
  close(server_fd);
  close(new_socket_fd);





  
  // In Part 2, client and server communicate using a pair of sockets
  // modify the part below so that the route request is read from a socket
  // (instead of stdin) and the route information is written to a socket

  // read a request
 




  // no path
  

  return 0;
}

