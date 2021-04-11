#pragma once

#include <string>
#include <vector>


namespace wetaxi { 
    struct Driver {
        int id;
        int car_id;
        bool is_working;
        bool is_busy;
        double rating;
        std::string first_name;
        std::string last_name;
        std::string login;
        std::string password;
        std::vector<int> order_history;
    };
}