#pragma once

#include <string>
#include <vector>


namespace wetaxi {
    struct Passenger {
        int id;
        std::string login;
        std::string password;
        std::string first_name;
        std::string last_name;
        double rating;

        Passenger() {}

        Passenger(std::string login, std::string password, std::string first_name, std::string last_name) {
            this->id = -1;
            this->login = login;
            this->password = password;
            this->first_name = first_name;
            this->last_name = last_name;
            this->rating = 0.0;
        }
    };
}