#ifndef INVITATION_H
#define INVITATION_H

#include "user.h"
#include "game.h"
using namespace std;

class User;
class Game;

class Invitation {
    public:
    User* inviter = NULL;
    Game* game = NULL;

    Invitation(User* inviter, Game* game) {
        this->inviter = inviter;
        this->game = game;
    }
    uint32_t get_room_id();
    static bool compare(Invitation* a, Invitation* b) {
        return a->get_room_id() < b->get_room_id();
    }
};

#endif