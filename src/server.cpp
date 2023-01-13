#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include "user.h"
#include "game.h"
#include "invitation.h"
#include "client.h"
using namespace std;


vector<User*> users; // registerd users
vector<Game*> games; // game rooms
vector<Client*> clients; // connected clients

string list_rooms() {
    string res = "List Game Rooms\n";
    if (games.size() == 0) {
        res += "No Rooms\n";
        return res;
    }
    sort(games.begin(), games.end(), Game::compare);
    for (int i = 0; i < games.size(); i++) {
        string s = to_string(i + 1) + ". (" + games[i]->type + ") Game Room " + to_string(games[i]->room_id);
        if (games[i]->rounds) {
            s += " has started playing\n";
        }
        else {
            s += " is open for players\n";
        }
        res += s;
    }
    return res;
}

string list_users() {
    string res = "List Users\n";
    if (users.size() == 0) {
        res += "No Users\n";
        return res;
    }
    sort(users.begin(), users.end(), User::compare);
    for (int i = 0; i < users.size(); i++) {
        string s = to_string(i + 1) + ". " + users[i]->name + "<" + users[i]->email + "> ";
        if (users[i]->client) {
            s += "Online\n";
        }
        else {
            s += "Offline\n";
        }
        res += s;
    }
    return res;
}

void remove_invitations(uint32_t room_id) {
    for (int i = 0; i < users.size(); i++) {
        for (int j = 0; j < users[i]->invitations.size(); j++) {
            if (users[i]->invitations[j]->get_room_id() == room_id) {
                users[i]->invitations.erase(users[i]->invitations.begin() + j);
                break;
            }
        }
    }
}

void remove_game(uint32_t room_id) {
    for (int i = 0; i < games.size(); i++) {
        if (games[i]->room_id == room_id) {
            remove_invitations(room_id);
            games.erase(games.begin() + i);
            break;
        }
    }
}

void parser(char *recvbuf, vector<string> &command) {
    command.clear();
    int argc = 0;
    char *argv[10];
    argv[argc] = strtok(recvbuf, " ");
    while (argv[argc] != NULL) {
        string s(argv[argc]);
        if (s[s.size() - 1] == '\n') s.erase(s.length() - 1);
        command.push_back(s);
        argv[++argc] = strtok(NULL, " ");
    }
}

User* find_usermail(string email) {
    for (int i = 0; i < users.size(); i++) {
        if (users[i]->email == email) {
            return users[i];
        }
    }
    return NULL;
}

User* find_username(string name) {
    for (int i = 0; i < users.size(); i++) {
        if (users[i]->name == name) {
            return users[i];
        }
    }
    return NULL;
}

Game* find_gameroom(uint32_t room_id) {
    for (int i = 0; i < games.size(); i++) {
        if (games[i]->room_id == room_id) {
            return games[i];
        }
    }
    return NULL;
}

string help_msg() {
    stringstream ss;
    ss << "Available commands:" << endl;
    ss << "register <username> <email> <user password>" << endl;
    ss << "login <username> <password>" << endl;
    ss << "logout" << endl;
    ss << "create public room <game room id>" << endl;
    ss << "create private room <game_room_id> <invitation code>" << endl;
    ss << "list rooms" << endl;
    ss << "list users" << endl;
    ss << "join room <game room id>" << endl;
    ss << "invite <invitee email>" << endl;
    ss << "list invitations" << endl;
    ss << "accept <inviter email> <invitation code>" << endl;
    ss << "leave room" << endl;
    ss << "start game <number of rounds> <guess number>" << endl;
    ss << "guess <guess number>" << endl;
    ss << "exit" << endl;
    string res = "";
    string s;
    while (getline(ss, s)) {
        res += s + "\n";
    }
    return res;
}

string tcp_handler(Client* client, vector<string> &command) {
    if (command.size() == 0 || command[0] == "exit") {
        if (client->user) {
            if (client->user->game) {
                if (client->user->game->manager == client->user) {
                    remove_game(client->user->game->room_id);
                }
                client->user->game->leave(client);
                client->user->game = NULL;
            }
            client->user->client = NULL;
            client->user = NULL;
        }
        return "exit";
    }
    else if (command[0] == "login") {
        string name = command[1];
        string pwd = command[2];
        User* user = find_username(name);
        if (!user) {
            return "Username does not exist\n";
        }
        else if (client->user) {
            return "You already logged in as " + client->user->name + "\n";
        }
        else if (user->client) {
            return "Someone already logged in as " + name + "\n";
        }
        else if (user->pwd != pwd) {
            return "Wrong password\n";
        }
        client->user = user;
        user->client = client;
        return "Welcome, " + name + "\n";
    }
    else if (command[0] == "logout") {
        if (!client->user) {
            return "You are not logged in\n";
        }
        else if (client->user->game) {
            return "You are already in game room " + to_string(client->user->game->room_id) + ", please leave game room\n";
        }
        string name = client->user->name;
        client->user->client = NULL;
        client->user = NULL;
        return "Goodbye, " + name + "\n";
    }
    else if (command[0] == "create") {
        uint32_t room_id = stoi(command[3]);
        if (!client->user) {
            return "You are not logged in\n";
        }
        else if (client->user->game) {
            return "You are already in game room " + to_string(client->user->game->room_id) + ", please leave game room\n";
        }
        else if (find_gameroom(room_id)) {
            return "Game room ID is used, choose another one\n";
        }
        Game* game = NULL;
        if (command[1] == "public") {
            game = new Game("Public", room_id, client->user);
        }
        else {
            uint32_t invite_code = stoi(command[4]);
            game = new Game("Private", room_id, client->user, invite_code);
        }
        games.push_back(game);
        return "You create " + command[1] + " game room " + to_string(room_id) + "\n";
    }
    else if (command[0] == "list") {
        return client->user->list_invitation();
    }
    else if (command[0] == "join") {
        uint32_t room_id = stoi(command[2]);
        if (!client->user) {
            return "You are not logged in\n";
        }
        else if (client->user->game) {
            return "You are already in game room " + to_string(client->user->game->room_id) + ", please leave game room\n";
        }
        Game* game = find_gameroom(room_id);
        if (!game) {
            return "Game room " + to_string(room_id) + " is not exist\n";
        }
        else if (game->type != "Public") {
            return "Game room is private, please join game by invitation code\n";
        }
        else if (game->rounds) {
            return "Game has started, you can't join now\n";
        }
        game->join(client);
        return "You join game room " + to_string(room_id) + "\n";
    }
    else if (command[0] == "invite") {
        string email = command[1];
        if (!client->user) {
            return "You are not logged in\n";
        }
        else if (!client->user->game) {
            return "You did not join any game room\n";
        }
        else if (client->user->game->manager != client->user) {
            return "You are not private game room manager\n";
        }
        User* invitee = find_usermail(email);
        if (!invitee->client) {
            return "Invitee not logged in\n";
        }
        client->user->invite(invitee, client->user->game);
        return "You send invitation to " + invitee->name + "<" + invitee->email + ">\n";
    }
    else if (command[0] == "accept") {
        string email = command[1];
        uint32_t invitation_code = stoi(command[2]);
        if (!client->user) {
            return "You are not logged in\n";
        }
        else if (client->user->game) {
            return "You are already in game room " + to_string(client->user->game->room_id) + ", please leave game room\n";
        }
        int invite_id;
        Invitation* invitation = client->user->find_invitation(email);
        if (!invitation) {
            return "Invitation not exist\n";
        }
        else if (invitation_code != invitation->game->invite_code) {
            return "Your invitation code is incorrect\n";
        }
        else if (invitation->game->rounds) {
            return "Game has started, you can't join now\n";
        }
        invitation->game->join(client);
        return "You join game room " + to_string(invitation->game->room_id) + "\n";
    }
    else if (command[0] == "leave") {
        if (!client->user) {
            return "You are not logged in\n";
        }
        else if (!client->user->game) {
            return "You did not join any game room\n";
        }
        Game* game = client->user->game;
        client->user->game = NULL;
        game->leave(client);
        if (game->manager == client->user) {
            game->broadcast(client, "Game room manager leave game room " + to_string(game->room_id) + ", you are forced to leave too\n");
            game->leave_all();
            game->end();
            remove_game(game->room_id);
            return "You leave game room " + to_string(game->room_id) + "\n";
        }
        else if (game->rounds) {
            game->broadcast(client, client->user->name + " leave game room " + to_string(game->room_id) + ", game ends\n");
            game->end();
            return "You leave game room " + to_string(game->room_id) + ", game ends\n";
        }
        else {
            game->broadcast(client, client->user->name + " leave game room " + to_string(game->room_id) + "\n");
            return "You leave game room " + to_string(game->room_id) + "\n";
        }
    }
    else if (command[0] == "start") {
        int rounds = stoi(command[2]);
        string answer = to_string(1000 + (rand() % 9000));
        if (command.size() > 3) {
            answer = command[3];
        }

        if (!client->user) {
            return "You are not logged in\n";
        }
        else if (!client->user->game) {
            return "You did not join any game room\n";
        }
        else if (client->user->game->manager != client->user) {
            return "You are not game room manager, you can't start game\n";
        }
        else if (client->user->game->rounds) {
            return "Game has started, you can't start again\n";
        }
        else if (answer.size() != 4 || !all_of(answer.begin(), answer.end(), ::isdigit)) {
            return "Please enter 4 digit number with leading zero\n";
        }
        client->user->game->rounds = rounds;
        client->user->game->answer = answer;
        string s = "Game start! Current player is " + client->user->name + "\n";
        client->user->game->broadcast(client, s);
        return s;
    }
    else if (command[0] == "guess") {
        string guess = command[1];
        if (!client->user) {
            return "You are not logged in\n";
        }
        else if (!client->user->game) {
            return "You did not join any game room\n";
        }
        Game* game = client->user->game;
        if (game->manager == client->user && game->rounds == 0) {
            return "You are game room manager, please start game first\n";
        }
        else if (game->rounds == 0) {
            return "Game has not started yet\n";
        }
        else if (client != game->players[game->turn]) {
            return "Please wait..., current player is " + game->players[game->turn]->user->name + "\n";
        }
        else if (guess.size() != 4 || !all_of(guess.begin(), guess.end(), ::isdigit)) {
            return "Please enter 4 digit number with leading zero\n";
        }
        else if (guess == game->answer) {
            string s = client->user->name + " guess '" + guess + "' and got Bingo!!! " + client->user->name + " wins the game, game ends\n";
            game->broadcast(client, s);
            game->end();
            return s;
        }
        else {
            if (++game->turn == game->players.size()) {
                game->turn = 0;
                game->rounds--;
            }
            int a = 0, b = 0;
            vector<char> wrong_guess;
            set<char> left;
            for (int i = 0; i < 4; i++) {
                if (game->answer[i] == guess[i]) {
                    a++;
                }
                else {
                    wrong_guess.push_back(guess[i]);
                    left.insert(game->answer[i]);
                }
            }
            for (int i = 0; i < wrong_guess.size(); i++) {
                if (left.find(wrong_guess[i]) != left.end()) {
                    b++;
                }
            }
            string s = to_string(a) + "A" + to_string(b) + "B";
            if (game->rounds == 0) {
                string str = client->user->name + " guess '" + guess + "' and got '" + s + "'\nGame ends, no one wins\n";
                game->broadcast(client, str);
                game->end();
                return str;
            }
            else {
                string str = client->user->name + " guess '" + guess + "' and got '" + s + "'\n";
                game->broadcast(client, str);
                return str;
            }
        }
    }
    // return "unknown command";
    return help_msg();
}

string udp_handler(vector<string>& command) {
    if (command[0] == "register") {
        string name = command[1];
        string email = command[2];
        string pwd = command[3];
        for (int i = 0; i < users.size(); i++) {
            if (users[i]->name == name || users[i]->email == email) {
                return "Username or Email is already used\n";
            }
        }
        users.push_back(new User(name, email, pwd));
        return "Register Successfully\n";
    }
    else if (command[0] == "list") {
        if (command[1] == "rooms") {
            return list_rooms();
        }
        else if (command[1] == "users") {
            return list_users();
        }
    }
    return "unknown command";
}


int main(int argc, char **argv) {
    struct sockaddr_in servaddr, cliaddr;
    socklen_t addrlen = sizeof(cliaddr);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = PF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8888);

    int udp_sockfd;
    if ((udp_sockfd = socket(AF_INET , SOCK_DGRAM , 0)) < 0) {
        perror("udp socket");
        exit(1);
    }
    const int enable = 1;
    if (setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("udp setsockopt");
        exit(1);
    }
    if (bind(udp_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("udp bind");
        exit(1);
    }
    // printf("UDP server is running\n");

    int tcp_sockfd; 
    if ((tcp_sockfd = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
        perror("tcp socket");
        exit(1);
    }
    if (setsockopt(tcp_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("tcp setsockopt");
        exit(1);
    }
    if (bind(tcp_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("tcp bind");
        exit(1);
    }
    if (listen(tcp_sockfd, 32) < 0) {
        perror("listen");
        exit(1);
    }
    // printf("TCP server is running\n");

    fd_set rset;
    char recvbuf[1024] = {};
    while (1) {
        FD_ZERO(&rset);
        FD_SET(tcp_sockfd, &rset);
        FD_SET(udp_sockfd, &rset);
        int maxfd = max(tcp_sockfd, udp_sockfd);
        for (int i = 0; i < clients.size(); i++) {
            FD_SET(clients[i]->connfd, &rset);
            if (clients[i]->connfd > maxfd) {
                maxfd = clients[i]->connfd;
            }
        }

        select(maxfd + 1, &rset, NULL, NULL, NULL); // block untill a fd is ready

        int connfd;
        if (FD_ISSET(tcp_sockfd, &rset)) { // new connection
            connfd = accept(tcp_sockfd, (struct sockaddr *) &cliaddr, &addrlen);
            clients.push_back(new Client(connfd));
        }
        else { // command from connected clients
            vector<string> command;
            if (FD_ISSET(udp_sockfd, &rset)) {
                if (recvfrom(udp_sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&cliaddr, &addrlen) < 0) {
                    perror("recvfrom");
                    exit(1);
                }
                parser(recvbuf, command);
                memset(recvbuf, 0, sizeof(recvbuf));
                string s = udp_handler(command);
                // cout << "reply: " << s;
                const char* replybuf = s.c_str();
                if (sendto(udp_sockfd, replybuf, strlen(replybuf), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) {
                    perror("sendto");
                    exit(1);
                }
            }
            auto iter = clients.begin();
            while (iter != clients.end()) {
                connfd = (*iter)->connfd;
                if (FD_ISSET(connfd, &rset)) {
                    if (recv(connfd, recvbuf, sizeof(recvbuf), 0) < 0) {
                        perror("recv");
                        exit(1);
                    }
                    parser(recvbuf, command);
                    memset(recvbuf, 0, sizeof(recvbuf));
                    string s = tcp_handler(*iter, command);
                    if (s == "exit") {
                        iter = clients.erase(iter);
                        close(connfd);
                        continue;
                    }
                    // cout << "reply: " << s;
                    const char* replybuf = s.c_str();
                    if (send(connfd, replybuf, strlen(replybuf), 0) < 0) {
                        perror("send");
                        exit(1);
                    }
                }
                iter++;
            }
        }
    }
}