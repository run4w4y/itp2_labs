mkdir -p builds
g++ -c assignment-3/rooms.cpp -o builds/rooms.o
g++ -c assignment-3/users.cpp builds/rooms.o -o builds/users.o
g++ assignment-3/main.cpp builds/users.o builds/rooms.o -o builds/assignment-3
