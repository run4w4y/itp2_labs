#pragma once

#include <string>
#include "car_type.h"


namespace wetaxi {
    struct Car {
        int id;
        std::string model;
        std::string registration_plate;
        wetaxi::CarType type;
    };
}