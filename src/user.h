#ifndef USER_H
#define USER_H

#include <string>
#include <cstring>
#include <sys/socket.h>
#include "invitation.h"
#include "client.h"
#include "game.h"
using namespace std;

class Client;
class Game;
class Invitation;

class User {
    public:
    string name;
    string email;
    string pwd;
    Client* client = NULL;
    Game* game = NULL;
    vector<Invitation*> invitations;

    User() {}
    User(string name, string email, string pwd) : name(name), email(email), pwd(pwd) {}
    void invite(User* invitee, Game* game);
    string list_invitation();
    Invitation* find_invitation(string email);
    static bool compare(User* a, User* b) {
        return a->name < b->name;
    }
};

#endif