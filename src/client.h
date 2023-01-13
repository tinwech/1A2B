#ifndef CLIENT_H
#define CLIENT_H

#include "user.h"

class User;

class Client {
    public:
    User* user = NULL;
    int connfd;

    Client() {}
    Client(int connfd) : connfd(connfd) {}
};

#endif