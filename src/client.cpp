#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
using namespace std;

char recvbuf[1024];
string replybuf;
int tcp_sockfd;

void *recvmsg(void *arg) {
    while (1) {
        memset(recvbuf, 0, sizeof(recvbuf));
        if (recv(tcp_sockfd, recvbuf, sizeof(recvbuf), 0) < 0) {
            perror("recv");
            exit(1);
        }
        if (strcmp(recvbuf, "exit") == 0) break;
        cout << recvbuf << endl;
    }
}

int main(int argc, char **argv) {
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = PF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if ((tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("tcp socket");
        exit(1);
    }
    if (connect(tcp_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        exit(1);
    }

    pthread_t t;
    pthread_create(&t, NULL, recvmsg, NULL);

    while (1) {
        // send to server
        cout << "> ";
        getline(cin, replybuf);
        if (send(tcp_sockfd, replybuf.c_str(), replybuf.size(), 0) < 0) {
            perror("send");
            exit(1);
        }
        if (replybuf == "exit") {
            break;
        }
    }

    pthread_join(t, NULL);
    close(tcp_sockfd);
    return 0;
}