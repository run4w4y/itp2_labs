#pragma once

#include <string>


namespace wetaxi {
    struct PinnedAddress {
        int id;
        std::string address;
        int passenger_id;
    };
}