#include "user.h"
#include <algorithm>
using namespace std;

void User::invite(User* invitee, Game* game) {
    Invitation *invitation = new Invitation(this, game);
    string s = "You receive invitation from " + invitation->inviter->name + "<" + invitation->inviter->email + ">\n";
    const char* replybuf = s.c_str();
    if (send(invitee->client->connfd, replybuf, strlen(replybuf), 0) < 0) {
        perror("send");
        exit(1);
    }
    for (int i = 0; i < invitee->invitations.size(); i++) {
        if (invitee->invitations[i]->inviter == this) {
            return;
        }
    }
    invitee->invitations.push_back(invitation);
}

string User::list_invitation() {
    string res = "List invitations\n"; 
    if (invitations.size() == 0) {
        res += "No Invitations\n";
        return res;
    }
    sort(invitations.begin(), invitations.end(), Invitation::compare);
    for (int i = 0; i < invitations.size(); i++) {
        string s = to_string(i + 1) + ". " + invitations[i]->inviter->name + "<" + invitations[i]->inviter->email + ">";
        s += " invite you to join game room " + to_string(invitations[i]->game->room_id);
        s += ", invitation code is " + to_string(invitations[i]->game->invite_code) + "\n";
        res += s;
    }
    return res;
}

Invitation* User::find_invitation(string email) {
    for (int i = 0; i < invitations.size(); i++) {
        if (email == invitations[i]->inviter->email) {
            return invitations[i];
        }
    }
    return NULL;
}