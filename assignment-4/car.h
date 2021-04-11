#pragma once

#include <string>


namespace wetaxi {
    struct Car {
        int id;
        int owner_id;
        std::string model;
        std::string registration_plate;
        int type;
    };
}