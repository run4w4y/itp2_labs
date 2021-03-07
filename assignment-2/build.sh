mkdir -p builds
g++ -c assignment-2/rooms.cpp -o builds/rooms.o
g++ -c assignment-2/users.cpp builds/rooms.o -o builds/users.o
g++ assignment-2/main.cpp builds/users.o builds/rooms.o -o builds/assignment-2