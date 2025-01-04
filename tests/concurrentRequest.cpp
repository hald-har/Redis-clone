#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <cerrno>
#include <thread>
#include <vector>

#define PORT "6379"

using namespace std;
mutex m;

void runping(int count , int client_id ) {
    int sockfd, bytes_received;
    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get address info
    if (getaddrinfo("localhost", PORT, &hints, &res) != 0) {
        cerr << "Get Address ERROR: " << strerror(errno) << endl;
        return;
    }

    // Try to connect to the server
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            cerr << "Socket creation failed: " << strerror(errno) << endl;
            continue;
        }

        // Try to connect to the server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1) {
            break;
        }

        cerr << "Connection failed: " << strerror(errno) << endl;
        close(sockfd);
    }

    freeaddrinfo(res);

    if (p == NULL) {
        cerr << "Failed to connect after multiple attempts" << endl;
        return;
    }

    const char* ptr = "+PING\r\n";
    for (int i = 1; i <= count; i++) {
        char buff[100];
        // Send the PING command
        int bytes_sent = send(sockfd, ptr, strlen(ptr), 0);
        if (bytes_sent == -1) {
            cerr << "Client " << client_id << " - Error sending PING command: " << strerror(errno) << endl;
            break;
        }

{
  lock_guard<mutex> lock(m) ;
   cout << "Client " << client_id << " - Sent: PING" << endl;
}


        // Receive the response
        bytes_received = recv(sockfd, buff, sizeof(buff) - 1, 0);
        if (bytes_received > 0) {
            buff[bytes_received] = '\0';  // Null-terminate the received data
            lock_guard<mutex> lock(m);
            cout << "Client " << client_id << " - Received: " << buff << endl;
        } else if (bytes_received == 0) {
            cerr << "Server closed the connection." << endl;
            break;
        } else {
            cerr << "Error receiving response: " << strerror(errno) << endl;
            break;
        }
    }

    close(sockfd);
}

int main() {
    int choice, count , num_clients;
    cout << "Choose test mode:\n";
    cout << "1. Sequential PING\n";
    cout << "2. Concurrent PING (Not Implemented)\n";
    cout << "Enter your choice: ";
    cin >> choice;

    if (choice == 1) {
        cout << "Enter the number of PING commands: ";
        cin >> count;
        runping(count , 1 );
    }
    else if (choice == 2) {
        cout << "Enter the number of clients: ";
        cin >> num_clients;
        cout << "Enter the number of PING commands per client: ";
        cin >> count;

        vector<thread> threads ;
        for( int i = 1 ; i<= num_clients ; i++){
          threads.emplace_back( runping , count , i ) ;
        }

        for( auto& t : threads ){
          t.join() ;
        }
    }
    else {
        cerr << "INVALID CHOICE" << endl;
    }

    return 0;
}
