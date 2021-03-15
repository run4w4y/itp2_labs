#pragma once

#include "access_level.h"


namespace rooms {
    class Room {
        public:
        int number; // room number
        virtual access_level::AccessLevel access() {
            return access_level::AccessLevel::no_level;
        }
        Room(int number);
        bool operator==(Room other);
    };

    class ClassRoom : public Room {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::green;
        }
        ClassRoom(int number);
    };

    class LectureRoom : public Room {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::green;
        }
        LectureRoom(int number);
    }; 

    class ConferenceRoom: public Room {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::yellow;
        }
        ConferenceRoom(int number);
    };

    class Cabinet : public Room {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::yellow;
        }
        Cabinet(int number);
    };

    class DirectorCabinet : public Room {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::red;
        }
        DirectorCabinet(int number);
    };
}
