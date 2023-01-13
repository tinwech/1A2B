#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <sys/socket.h>
#include "user.h"
#include "client.h"
using namespace std;

class User;
class Client;

class Game {
    public:
    uint32_t room_id;
    uint32_t invite_code;
    string type;
    User* manager = NULL;
    vector<Client*> players;
    int rounds = 0;
    int turn = 0;
    string answer;
    int round;

    Game() {}
    Game(string type, uint32_t id, User* manager);
    Game(string type, uint32_t id, User* manager, uint32_t invite_code);
    void join(Client* client);
    void leave(Client* client);
    void leave_all();
    void end();
    void broadcast(Client* sender, string s);
    static bool compare(Game* a, Game* b) {
        return a->room_id < b->room_id;
    }
};

#endif