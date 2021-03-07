#include "access_level.h"
#include "rooms.h"
#include "users.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace users;


User::User(std::string first_name, std::string last_name) {
    this->first_name = first_name;
    this->last_name = last_name;
}

Admin::Admin(std::string first_name, std::string last_name) : User(first_name, last_name) {};

Student::Student(std::string first_name, std::string last_name) : User(first_name, last_name) {};

Professor::Professor(std::string first_name, std::string last_name, std::string favorite_joke) : User(first_name, last_name) {
    favorite_joke_ = favorite_joke;
}

std::string& Professor::favorite_joke() {
    return favorite_joke_;
};

LabEmployee::LabEmployee(std::string first_name, std::string last_name) : User(first_name, last_name) {};

Director::Director(std::string first_name, std::string last_name) : Admin(first_name, last_name) {};