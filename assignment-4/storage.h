#pragma once

#include <sqlite_orm/sqlite_orm.h>
#include "passenger.h"
#include "driver.h"
#include "car.h"
#include "order.h"


namespace wetaxi::storage {
    using namespace sqlite_orm;

    auto default_storage = make_storage("db.sqlite",
        make_table("passengers",
            make_column("id", &wetaxi::Passenger::id, autoincrement(), primary_key()),
            make_column("login", &wetaxi::Passenger::login),
            make_column("password", &wetaxi::Passenger::password),
            make_column("first_name", &wetaxi::Passenger::first_name),
            make_column("last_name", &wetaxi::Passenger::last_name),
            make_column("rating", &wetaxi::Passenger::rating),
            make_column("order_history", &wetaxi::Passenger::order_history)
        ),
        make_table("drivers",
            make_column("id", &wetaxi::Driver::id, autoincrement(), primary_key()),
            make_column("car_id", &wetaxi::Driver::car_id),
            make_column("is_working", &wetaxi::Driver::is_working),
            make_column("is_busy", &wetaxi::Driver::is_busy),
            make_column("rating", &wetaxi::Driver::rating),
            make_column("first_name", &wetaxi::Driver::first_name),
            make_column("last_name", &wetaxi::Driver::last_name),
            make_column("login", &wetaxi::Driver::login),
            make_column("password", &wetaxi::Driver::password),
            make_column("order_history", &wetaxi::Driver::order_history)
        ),
        make_table("cars",
            make_column("id", &wetaxi::Car::id, autoincrement(), primary_key()),
            make_column("owner_id", &wetaxi::Car::owner_id),
            make_column("model", &wetaxi::Car::model),
            make_column("registration_plate", &wetaxi::Car::registration_plate),
            make_column("type", &wetaxi::Car::type)
        ),
        make_table("orders",
            make_column("id", &wetaxi::Order::id, autoincrement(), primary_key()),
            make_column("passenger_id", &wetaxi::Order::passenger_id),
            make_column("driver_id", &wetaxi::Order::driver_id),
            make_column("car_id", &wetaxi::Order::car_id),
            make_column("route_to", &wetaxi::Order::route_from),
            make_column("timestamp", &wetaxi::Order::timestamp)
        )
    );

    default_storage.sync_schema(); // does not work and is probably a bad idea
}