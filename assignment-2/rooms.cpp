#include "access_level.h"
#include "rooms.h"

using namespace rooms;


Room::Room(int number) {
    this->number = number;
}

bool Room::operator==(Room other) {
    return number == other.number;
}

ClassRoom::ClassRoom(int number) : Room(number) {}

LectureRoom::LectureRoom(int number) : Room(number) {}

ConferenceRoom::ConferenceRoom(int number) : Room(number) {}

Cabinet::Cabinet(int number) : Room(number) {}

DirectorCabinet::DirectorCabinet(int number) : Room(number) {}