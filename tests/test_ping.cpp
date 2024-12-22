#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT "6379"

using namespace std;

int main() {
    struct addrinfo hints, *res;
    int sockfd;
    char buffer[1024];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve server address
    if (getaddrinfo("localhost", PORT, &hints, &res) != 0) {
        cerr << "Error resolving address." << endl;
        return 1;
    }

    // Create socket and connect
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        cerr << "Error connecting to server." << endl;
        return 1;
    }

    // Send PING command
    const char* ping_command = "PING\r\n";
    if (send(sockfd, ping_command, strlen(ping_command), 0) == -1) {
        cerr << "Error sending PING command." << endl;
        return 1;
    }

    // Receive and print the response
    int bytes_received = recv(sockfd, buffer, sizeof(buffer)-1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        cout << "Server response: " << buffer << endl;
    }

    close(sockfd);
    return 0;
}
