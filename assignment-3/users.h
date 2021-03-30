#pragma once

#include "access_level.h"
#include "rooms.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>


namespace users {
    namespace {
        std::map<std::string, std::vector<int>> additional_rooms;
        bool emergency = false;
    }
    
    bool toggle_emergency();
    bool emergency_state();

    class User {
        public:
        std::string first_name; // first name of the user
        std::string last_name; // last name of the user

        virtual access_level::AccessLevel access() {
            return access_level::AccessLevel::no_level;
        }

        template <typename RoomType>
        bool has_access_to(RoomType r) {
            std::vector<int> v = additional_rooms[first_name + last_name];
            
            bool res = (
                    static_cast<int>(r.access()) != -1 &&
                    static_cast<int>(access()) != -1 && 
                    static_cast<int>(r.access()) <= static_cast<int>(access())
                ) || std::find(v.begin(), v.end(), r.number) != v.end() || emergency_state();

            // logging in std::cout
            std::cout << "User " << first_name << " " << last_name << " tried to access the room #" << r.number << " and ";
            if (res) 
                std::cout << "was granted access." << std::endl;
            else
                std::cout << "was refused access." << std::endl; 
            
            return res;
        }
        
        User(std::string first_name, std::string last_name);
    };

    class Admin : public User {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::no_level;
        }

        template <typename NonAdminUser>
        void add_room_access(NonAdminUser u, rooms::Room r) {
            std::cout << "Admin " << first_name << " " << last_name << " gave user " 
                << u.first_name << " " << u.last_name << " access to room #" << r.number << "." << std::endl;

            additional_rooms[u.first_name + u.last_name].push_back(r.number);
        }
        
        Admin(std::string first_name, std::string last_name);
    };
    
    class Student : public User {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::green;
        }
        Student(std::string first_name, std::string last_name);
    };
    
    class Professor : public User {
        std::string favorite_joke_;

        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::yellow;
        }
        std::string& favorite_joke(); 
        Professor(std::string first_name, std::string last_name, std::string favorite_joke);
    };
    
    class LabEmployee : public User {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::yellow;
        }
        LabEmployee(std::string first_name, std::string last_name);
    };
 
    class Director : public Admin {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::red;
        }
        Director(std::string first_name, std::string last_name);
    };

    class Guest : public User {
        public:
        access_level::AccessLevel access() override {
            return access_level::AccessLevel::blue;
        }
        Guest(std::string first_name, std::string last_name);
    };
}