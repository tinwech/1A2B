#include "game.h"

Game::Game(string type, uint32_t id, User* manager) : type(type), room_id(id) {
    this->manager = manager;
    manager->game = this;
    players.push_back(manager->client);
}

Game::Game(string type, uint32_t id, User* manager, uint32_t invite_code) : type(type), room_id(id), invite_code(invite_code) {
    this->manager = manager;
    manager->game = this;
    players.push_back(manager->client);
}

void Game::join(Client* client) {
    string s = "Welcome, " + client->user->name + " to game!\n";
    broadcast(client, s);
    client->user->game = this;
    players.push_back(client);
}

void Game::leave(Client* client) {
    for (int i = 0; i < players.size(); i++) {
        if (players[i]->user == client->user) {
            players.erase(players.begin() + i);
            break;
        }
    }
}

void Game::leave_all() {
    for (int i = 0; i < players.size(); i++) {
        players[i]->user->game = NULL;
    }
    players.clear();
}

void Game::end() {
    rounds = 0;
    turn = 0;
    answer = "";
}

void Game::broadcast(Client* sender, string s) {
    const char* replybuf = s.c_str();
    for (int i = 0; i < players.size(); i++) {
        if (players[i] == sender) {
            continue;
        }
        if (send(players[i]->connfd, replybuf, strlen(replybuf), 0) < 0) {
            perror("send");
            exit(1);
        }
    }
}