#pragma once

#include <chrono>
#include <string>
#include "gateway.h"
#include "auth.h"
#include "../order.h"
#include "../order_status.h"


namespace wetaxi {
    class DriverGateway : public Gateway {
        public:
        static void signup(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            using namespace std::string_literals;
            using json = nlohmann::json;
            const auto required_car = {"model"s, "registration_plate"s, "type"s};
            const auto required_driver = {"login"s, "password"s, "first_name"s, "last_name"s};
            auto parsed_body = json::parse(req.body);
            
            if (parsed_body.find("car") == parsed_body.end() || parsed_body.find("driver") == parsed_body.end()) {
                res.set_content("the fuck did you send nigga", "text/plain");
                return;
            }

            json car_j = parsed_body.at("car");
            json driver_j = parsed_body.at("driver");

            std::vector<std::string> car_missing(0);
            for (const auto &i : required_car) 
                if (car_j.find("i") == car_j.end())
                    car_missing.push_back(i);

            if (!car_missing.empty()) {
                res.set_content("in field car: " + missing_params_msg(car_missing), "text/plain");
                return;
            }

            std::vector<std::string> driver_missing(0);
            for (const auto &i : required_driver)
                if (driver_j.find(i) == driver_j.end())
                    driver_missing.push_back(i);
            
            if (!driver_missing.empty()) {
                res.set_content("in field driver: " + missing_params_msg(driver_missing), "text/plain");
                return;
            }

            std::string car_type_str;
            car_j.at("type").get_to(car_type_str);
            auto car_type = car_type_from_string(car_type_str);
            if (!car_type) {
                res.set_content("invalid string for cartype", "text/plain");
                return;
            }

            wetaxi::Car car{-1, "", "", *car_type};
            car_j.at("model").get_to(car.model);
            car_j.at("registration_plate").get_to(car.registration_plate);
            auto car_id = storage.insert(car);
            car.id = car_id;

            wetaxi::Driver driver{-1, car.id, false, false, 0.0, "", "", "", ""};
            driver_j.at("first_name").get_to(driver.first_name);
            driver_j.at("last_name").get_to(driver.last_name);
            driver_j.at("login").get_to(driver.login);
            driver_j.at("password").get_to(driver.password);
            auto driver_id = storage.insert(driver);
            driver.id = driver_id;
        }

        static void login(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            using namespace std::string_literals;
            const auto required = {"login"s, "password"s};
            auto parsed_body = validate_and_parse<std::string, std::string>(req.body, required);

            try { // everything is alright, we got the data
                auto decoded_body = std::get<std::map<std::string, std::string>>(parsed_body);
                if (auto token_pair = auth::login<wetaxi::Driver>(storage, decoded_body["login"], decoded_body["password"])) {
                    auto token = token_pair->first;
                    auto user = token_pair->second;
                    res.set_content("successfully logged in as " + 
                        user.first_name + " " + user.last_name + 
                        " token: " + token.keystring, "text/plain"
                    );
                } else {
                    res.set_content("naw that didnt work", "text/plain");
                }
            } catch (const std::bad_variant_access&) { // hell naw man
                res.set_content(missing_params_msg(std::get<std::vector<std::string>>(parsed_body)), "text/plain");
            }
        }

        static void car_info(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Driver>(storage, req)) {
                using namespace sqlite_orm;
                using json = nlohmann::json;

                auto found = storage.get_all<wetaxi::Car>(
                    where(c(&wetaxi::Car::id) == user->car_id),
                    limit(1)
                );

                if (found.empty()) {
                    res.set_content("car was not found", "text/plain");
                    return;
                }

                wetaxi::Car car = found[0];
                std::stringstream ss;
                ss << json{
                    {"model", car.model},
                    {"registration_plate", car.registration_plate},
                    {"type", wetaxi::car_type_to_string(car.type)}
                };
                res.set_content(ss.str(), "application/json");
            } else
                res.set_content("log in first", "text/plain");
        }

        static void order_history(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Driver>(storage, req)) {
                using namespace sqlite_orm;
                using json = nlohmann::json;

                auto found = storage.get_all<wetaxi::Order>(
                    where(c(&wetaxi::Order::driver_id) == user->id)
                );

                std::vector<json> vj(0);
                for (const auto &i : found)
                    vj.push_back(json{
                        {"id", i.id},
                        {"passenger_id", i.passenger_id},
                        {"driver_id", i.driver_id},
                        {"car_id", i.car_id},
                        {"route_from", i.route_from},
                        {"route_to", i.route_to},
                        {"timestamp", i.timestamp}
                    });

                json j(vj);
                std::stringstream ss;
                ss << j;
                res.set_content(ss.str(), "application/json");
            } else
                res.set_content("log in first", "text/plain");
        }

        static void update_status(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Driver>(storage, req)) {
                using json = nlohmann::json;
                using namespace sqlite_orm;
                
                json j = json::parse(req.body);

                if (j.find("is_working") == j.end()) {
                    res.set_content("is_working field is not present", "text/plain");
                    return;
                }
                
                j.at("is_working").get_to(user->is_working);
                if (user->is_working)
                    user->is_busy = false;

                storage.update_all(
                    set(
                        c(&wetaxi::Driver::is_working) = user->is_working,
                        c(&wetaxi::Driver::is_busy) = user->is_busy
                    ),
                    where(c(&wetaxi::Driver::id) == user->id)
                );
                res.set_content("all gucci", "text/plain");
            } else
                res.set_content("log in first", "text/plain");
        }

        static void available_orders(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Driver>(storage, req)) {
                using json = nlohmann::json;
                using namespace sqlite_orm;
                
                auto found_car = storage.get_all<wetaxi::Car>(
                    where(c(&wetaxi::Car::id) = user->car_id)
                );

                if (found_car.empty()) {
                    res.set_content("couldnt find car", "text/plain");
                    return;
                }

                auto car = found_car[0];

                auto found = storage.get_all<wetaxi::Order>(
                    where(
                        c(&wetaxi::Order::status) == wetaxi::OrderStatus::pending &&
                        c(&wetaxi::Order::car_type) == car.type
                    )
                );

                std::vector<json> vj(0);
                for (const auto &i : found)
                    vj.push_back(json{
                        {"id", i.id},
                        {"from", i.route_from},
                        {"to", i.route_to},
                        {"price", i.price}
                    });

                json j(vj);
                std::stringstream ss;
                ss << j;
                res.set_content(ss.str(), "application/json");
            } else
                res.set_content("log in first", "text/plain");
        }

        static void pick_order(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Driver>(storage, req)) {
                using json = nlohmann::json;
                using namespace sqlite_orm;
                
                json j = json::parse(req.body);

                if (j.find("order_id") == j.end()) {
                    res.set_content("order_id field is not present", "text/plain");
                    return;
                }
                
                int order_id;
                j.at("order_id").get_to(order_id);

                auto found_car = storage.get_all<wetaxi::Car>(
                    where(c(&wetaxi::Car::id) = user->car_id)
                );

                if (found_car.empty()) {
                    res.set_content("couldnt find car", "text/plain");
                    return;
                }

                auto car = found_car[0];

                storage.transaction([&]() {
                    auto found = storage.get_all<wetaxi::Order>(
                        where(
                            c(&wetaxi::Order::id) == order_id &&
                            c(&wetaxi::Order::status) == "pending" &&
                            c(&wetaxi::Order::car_type) == car.type
                        )
                    );

                    if (found.empty())
                        return false;
                    
                    wetaxi::Order order = found[0];
                    order.driver_id = user->id;
                    order.car_id = user->car_id; 

                    auto current_timepoint = std::chrono::system_clock::now();
                    time_t current_ts = std::chrono::system_clock::to_time_t(current_timepoint);
                    std::string current_timestring = auth::format_time(current_ts, auth::DEFAULT_DATETIME_FORMAT);

                    order.timestamp = current_timestring;
                    order.status = wetaxi::OrderStatus::in_progress;

                    storage.update_all(
                        set(
                            c(&wetaxi::Order::driver_id) = order.driver_id,
                            c(&wetaxi::Order::car_id) = order.car_id,
                            c(&wetaxi::Order::timestamp) = order.timestamp,
                            c(&wetaxi::Order::status) = order.status
                        ),
                        where(
                            c(&wetaxi::Order::id) == order.id
                        )
                    );
                    return true;
                });

                res.set_content("all gucci", "text/plain");
            } else
                res.set_content("log in first", "text/plain");
        }

        static void finish_ride(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Driver>(storage, req)) {
                using json = nlohmann::json;
                using namespace sqlite_orm;
                
                auto found = storage.get_all<wetaxi::Order>(
                    where(
                        c(&wetaxi::Order::driver_id) == user->id &&
                        c(&wetaxi::Order::status) == wetaxi::OrderStatus::in_progress
                    )
                );

                if (found.empty()) {
                    res.set_content("no rides going on atm", "text/plain");
                    return;
                }

                wetaxi::Order order = found[0];
                order.status = wetaxi::OrderStatus::finished;
                storage.update_all(
                    set(c(&wetaxi::Order::status) = order.status),
                    where(c(&wetaxi::Order::id) == order.id)
                );

                // we gotta charge the passenger here but we cant do that oh well

                res.set_content("all gucci", "text/plain");
            } else
                res.set_content("log in first", "text/plain");
        }
    };
}