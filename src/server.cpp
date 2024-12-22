#include <iostream>
#include <cstring>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>

#define PORT "6379"
#define BACKLOG 10

using namespace std;

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int one_request(int newfd) {
    char buf[1000];
    memset(buf, 0, sizeof(buf));
    int bytes_received = recv(newfd, buf, sizeof(buf) - 1, 0);

    if (bytes_received < 0) {
        perror("recv failed");
        return -1;
    }
    if (bytes_received == 0) {
        // Client disconnected
        cout << "Client disconnected." << endl;
        return 0;  // Graceful client disconnect
    }

    buf[bytes_received] = '\0';
    cout << "Received: " << buf << endl;

    const char* response = "+PONG\r\n";
    if (send(newfd, response, strlen(response), 0) == -1) {
        perror("send");
        return -1;
    }
    cout << "Sent: " << response << endl;

    return 1;  // Continue processing
}

int main() {
    int sockfd, newfd;
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo("localhost", PORT, &hints, &res) != 0) {
        cerr << "Get addrinfo ERROR" << endl;
        return 1;
    }

    // Loop through all the possible addresses and try to bind
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sockfd == -1) {
            perror("server: socket");
            continue;
        }

        // Set socket options
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            close(sockfd);
            continue;
        }

        // Attempt to bind to the socket
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(res);

    if (p == NULL) {
        cerr << "Server FAILED to bind" << '\n';
        exit(1);
    }

    // Listen on the bound socket
    if (listen(sockfd, BACKLOG) == -1) {
        perror("Listen");
        exit(1);
    }

    cout << "Server Waiting for Connections...." << '\n';

    while (1) {
        sin_size = sizeof(their_addr);
        newfd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);

        if (newfd == -1) {
            perror("Accept failed");
            continue;  // Try accepting the next connection
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
        cout << "Server: got connection from " << s << endl;

        // Handle the client request sequentially
        while (true) {
            int status = one_request(newfd);
            if (status <= 0) {
                if (status == -1) {
                    cerr << "Error while processing request. Closing connection." << endl;
                }
                break;
            }
        }
        close(newfd);
    }
    close(sockfd);
    return 0;
}
