all:
	g++ src/client.cpp -o build/client
	g++ src/*.h src/game.cpp src/invitation.cpp src/server.cpp src/user.cpp -o build/server