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
#include <fcntl.h>
#include <sys/epoll.h>

#define PORT "6379"
#define BACKLOG 10
#define MAX_EVENTS 10

using namespace std;

// Set the socket to non-blocking mode
int set_socket_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }
    return 0;
}

// Extract the IP address from sockaddr
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Handle client requests
int one_request(int newfd) {
    char buf[1000];
    memset(buf, 0, sizeof(buf));

    while (true) {
        int bytes_received = recv(newfd, buf, sizeof(buf) - 1, 0);
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data to read
                break;
            }
            perror("recv failed");
            return -1;
        }
        if (bytes_received == 0) {
            // Client disconnected
            cout << "Client disconnected." << endl;
            return 0;
        }

        buf[bytes_received] = '\0';
        cout << "Received: " << buf << endl;

        const char* response = "+PONG\r\n";
        if (send(newfd, response, strlen(response), 0) == -1) {
            perror("send");
            return -1;
        }
        cout << "Sent: " << response << endl;
    }

    return 1;  // Continue processing
}

void cleanup(int sockfd, int epollfd) {
    close(sockfd);
    close(epollfd);
    cout << "Server shut down." << endl;
}

int main() {
    int sockfd, newfd, epollfd;
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(nullptr, PORT, &hints, &res) != 0) {
        cerr << "Get addrinfo ERROR" << endl;
        return 1;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            close(sockfd);
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        cerr << "Server FAILED to bind" << endl;
        exit(1);
    }

    if (set_socket_nonblocking(sockfd) == -1) {
        cerr << "Failed to set socket non-blocking" << endl;
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("Listen");
        exit(1);
    }

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create");
        exit(1);
    }

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
        perror("epoll_ctl: listen socket");
        exit(1);
    }

    cout << "Server Waiting for Connections..." << endl;

    while (true) {
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            cleanup(sockfd, epollfd);
            exit(1);
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == sockfd) {
                sin_size = sizeof(their_addr);
                newfd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
                if (newfd == -1) {
                    perror("Accept failed");
                    continue;
                }

                inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
                cout << "Server: got connection from " << s << endl;

                if (set_socket_nonblocking(newfd) == -1) {
                    cerr << "Failed to set client socket non-blocking" << endl;
                    close(newfd);
                    continue;
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = newfd;

                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, newfd, &ev) == -1) {
                    perror("epoll_ctl: add client socket");
                    close(newfd);
                }
            } else {
                int client_fd = events[i].data.fd;
                int status = one_request(client_fd);
                if (status <= 0) {
                    close(client_fd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, client_fd, nullptr);
                }
            }
        }
    }

    cleanup(sockfd, epollfd);
    return 0;
}
