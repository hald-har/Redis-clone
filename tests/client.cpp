#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 6379  // Server port
#define SERVER_IP "127.0.0.1"  // Assuming server is running locally

using namespace std;

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    // Set server address details
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    // Get user input (command to send to the server)
    string message;
    cout << "Enter a command to send to the server (e.g., 'echo hello'): ";
    getline(cin, message);

    // Send the message to the server
    if (send(sockfd, message.c_str(), message.length(), 0) == -1) {
        perror("send");
        close(sockfd);
        return -1;
    }

    // Receive response from the server
    int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(sockfd);
        return -1;
    }

    // Null-terminate and print the received response
    buffer[bytes_received] = '\0';
    cout << "Server response: " << buffer << endl;

    // Close the socket
    close(sockfd);
    return 0;
}
