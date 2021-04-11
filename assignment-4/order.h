#pragma once

#include <string>
#include "car_type.h"


namespace wetaxi {
    struct Order {
        int id;
        int passenger_id;
        int driver_id;
        int car_id;
        std::string route_from;
        std::string route_to;
        time_t timestamp;
    };
}