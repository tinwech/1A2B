# 1A2B game server

A 1A2B tcp/udp server implemented with c++. The server is able to handle multiple clients and each client can join the game  room to play together.

## Introduction

The game 1A2B is a simple number guessing game where the player tries to guess a four-digit secret number chosen by the computer. The player can enter a four-digit guess and the computer will respond with how many digits are in the correct place (A) and how many digits are in the wrong place but still present in the secret number (B). The player continues to make guesses until they correctly guess the secret number.

## Getting started

### Compile the code

```shell
mkdir build
make
```

### Start the server

```shell
./build/server 8888
```

### Connect to server

```shell
./build/client 127.0.0.1 8888
```

### Usage

Show the available commands:

```shell
> help
available commands:
register <username> <email> <user password>
login <username> <password>
logout
create public room <game room id>
create private room <game_room_id> <invitation code>
list rooms
list users
join room <game room id>
invite <invitee email>
list invitations
accept <inviter email> <invitation code>
leave room
start game <number of rounds> <guess number>
guess <guess number>
exit
```