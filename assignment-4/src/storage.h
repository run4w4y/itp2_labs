#pragma once

#include <string>
#include <sqlite_orm/sqlite_orm.h>
#include "passenger.h"
#include "driver.h"
#include "car.h"
#include "order.h"
#include "gateways/auth_token.h"
#include "payment_method.h"
#include "pinned_address.h"


namespace wetaxi::storage {
    inline auto storage_init(const std::string &path) {
        using namespace sqlite_orm;

        return make_storage(path,
            make_table("passengers",
                make_column("id", &wetaxi::Passenger::id, autoincrement(), primary_key()),
                make_column("login", &wetaxi::Passenger::login),
                make_column("password", &wetaxi::Passenger::password),
                make_column("first_name", &wetaxi::Passenger::first_name),
                make_column("last_name", &wetaxi::Passenger::last_name),
                make_column("rating", &wetaxi::Passenger::rating)
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
                make_column("password", &wetaxi::Driver::password)
            ),
            make_table("cars",
                make_column("id", &wetaxi::Car::id, autoincrement(), primary_key()),
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
                make_column("timestamp", &wetaxi::Order::timestamp),
                make_column("status", &wetaxi::Order::status),
                make_column("price", &wetaxi::Order::price),
                make_column("payment_method_id", &wetaxi::Order::payment_method_id)
            ),
            make_table("tokens",
                make_column("id", &wetaxi::auth::AuthToken::id, autoincrement(), primary_key()),
                make_column("keystring", &wetaxi::auth::AuthToken::keystring),
                make_column("issued", &wetaxi::auth::AuthToken::issued),
                make_column("expires", &wetaxi::auth::AuthToken::expires),
                make_column("user_id", &wetaxi::auth::AuthToken::user_id)
            ),
            make_table("payment_methods",
                make_column("id", &wetaxi::PaymentMethod::id, autoincrement(), primary_key()),
                make_column("card_number", &wetaxi::PaymentMethod::card_number),
                make_column("year", &wetaxi::PaymentMethod::year),
                make_column("month", &wetaxi::PaymentMethod::month),
                make_column("cvc", &wetaxi::PaymentMethod::cvc),
                make_column("first_name", &wetaxi::PaymentMethod::first_name),
                make_column("last_name", &wetaxi::PaymentMethod::last_name),
                make_column("passenger_id", &wetaxi::PaymentMethod::passenger_id)
            ),
            make_table("pinned_addresses",
                make_column("id", &wetaxi::PinnedAddress::id, autoincrement(), primary_key()),
                make_column("address", &wetaxi::PinnedAddress::address),
                make_column("passenger_id", &wetaxi::PinnedAddress::passenger_id)
            )
        );
    };

    using Storage = decltype(storage_init(""));
}