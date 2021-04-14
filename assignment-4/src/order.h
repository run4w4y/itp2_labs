#pragma once

#include <string>
#include "car_type.h"
#include "order_status.h"


namespace wetaxi {
    struct Order {
        int id;
        int passenger_id;
        int driver_id;
        int car_id;
        CarType car_type;
        int price;
        int payment_method_id;
        std::string route_from;
        std::string route_to;
        std::string timestamp;
        OrderStatus status;
    };
}