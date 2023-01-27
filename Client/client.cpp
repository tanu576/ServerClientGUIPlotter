#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <errno.h>
// Add more libraries, macros, functions, and global variables if needed

using namespace std;

#define MAX_SIZE 1024

int create_and_open_fifo(const char * pname, int mode) {
    // creating a fifo special file in the current working directory
    // with read-write permissions for communication with the plotter
    // both proecsses must open the fifo before they can perform
    // read and write operations on it
    
    if (mkfifo(pname, 0666) == -1) {
        cout << "Unable to make a fifo. Ensure that this pipe does not exist already!" << endl;
        exit(-1);
    }

    // opening the fifo for read-only or write-only access
    // a file descriptor that refers to the open file description is
    // returned
    int fd = open(pname, mode);
    if (fd == -1) {
        cout << "Error: failed on opening named pipe." << endl;
        exit(-1);
    }
    return fd;
}

int main(int argc, char const *argv[]) {
    const char * server_ip = argv[2];// the server ip address in numbers-and-dots notation
    int port_num = atoi(argv[1]); // the server port number
    struct sockaddr_in serv_addr; 
    int sock = 0;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cout << "Socket creation failed!" << endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET; // IPv4 socket family
    // server port number in network byte order 
    serv_addr.sin_port = htons(port_num);
    // server IP address in network byte order
    inet_aton(server_ip, &(serv_addr.sin_addr));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        cout << "Connection refused!" << endl;
        return 1;
    }
    cout<<"Connection request accepted from "<< inet_ntoa(serv_addr.sin_addr)<<
                                     ":" << ntohs(serv_addr.sin_port) << endl;
    const char *inpipe = "inpipe";
    const char *outpipe = "outpipe";
    int in = create_and_open_fifo(inpipe, O_RDONLY);
    cout << "inpipe opened..." << endl;
    int out = create_and_open_fifo(outpipe, O_WRONLY);
    cout << "outpipe opened..." << endl;
    char buffer1[2][MAX_SIZE];
    char buffer2[MAX_SIZE];
    int i = 0;
    int bytesread; 
    // struct timeeval for timeout
    struct timeval timer = {.tv_sec = 1, .tv_usec = 0};
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *) &timer, 
                                                        sizeof(timer)) == -1) {
        std::cerr << "Cannot set socket options!\n";
        return 1;
    }	
    while (true) {
        bytesread = read(in, buffer1[0], 22);
        if (bytesread == -1)
            cerr << "Error: read operation failed!" << endl;
        string z(buffer1[0]);
        // Check for Q if Q end
        z = z.substr(0,1);
        if (z.compare("Q") == 0){
            cout<<"Connection will be closed"<<endl;
            send(sock, buffer1[0], 1,0);
            break;
        }
        // Else read the cordinates for end
        bytesread = read(in, buffer1[1], 22);
        if (bytesread == -1)
            cerr << "Error: read operation failed!" << endl;
        // sending start and end coordinates to server through sockets
        send(sock, buffer1[0], 22, 0);
        send(sock, buffer1[1], 22, 0);
        // Recieving number of waypoints
        int recv_size = recv(sock, buffer2, MAX_SIZE, 0);
        string a(buffer2);
        // checking if E
	    string check = a.substr(0,1);
	    if (check.compare("E")== 0){
		    write(out, "E\n", 2);
		    continue;
	    }
        int len = a.find('\n', 2)-2;
        int num = stoi(a.substr(2,len));
        // sending acknowledments
        send(sock, "A\n", 2,0);
        while(num != -1 ){
            // recieving waypoints
            recv_size = recv(sock, buffer2, 24, 0);
            // check for timeout
            if (recv_size == -1){
                cout<<"Timeout"<<endl;
                break;
            }
            string w(buffer2);
            string chi = w.substr(0,1);
            // check to see if server sent E
            if (chi.compare("E")== 0){
                break;
            }
            w.insert(4, ".");
            w.insert(15, ".");
            //cout<<w;
            w = w.substr(2, 20);
            int n = w.length();
            char buffer3[n+1];
            strcpy(buffer3,w.c_str());
            // writing waypoint to outpipe
            write(out, buffer3, 20);
            num -= 1 ;
            send(sock, "A\n", 2,0);
        }
        // writing E to outpipe
        write(out, "E\n", 2);
    }
    close(in);
    close(out);
    unlink(inpipe);
    unlink(outpipe);
    close(sock);
    return 0;
}

