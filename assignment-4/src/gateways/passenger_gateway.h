#pragma once

#include <map>
#include <variant>
#include <vector>
#include <string>
#include <optional>
#include <algorithm>
#include <initializer_list>
#include <sstream>
#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <sqlite_orm/sqlite_orm.h>
#include "../storage.h"
#include "../passenger.h"
#include "../order.h"
#include "../payment_method.h"
#include "../pinned_address.h"
#include "../car_type.h"
#include "../order_status.h"
#include "auth.h"
#include "auth_token.h"
#include "gateway.h"


namespace wetaxi {
    // i have no idea why is this a class when literally every single member function in it is static 
    class PassengerGateway : public Gateway {
        public:
        static void signup(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            using namespace std::string_literals;
            const auto required = {"login"s, "password"s, "first_name"s, "last_name"s};
            auto parsed_body = validate_and_parse<std::string, std::string>(req.body, required);
            
            try { // everything is alright, we got the data
                auto decoded_body = std::get<std::map<std::string, std::string>>(parsed_body);

                wetaxi::Passenger user(
                    decoded_body["login"], // TODO: actually logins should be unique bruh
                    auth::hash_pass(decoded_body["password"]), 
                    decoded_body["first_name"], 
                    decoded_body["last_name"]
                );

                storage.transaction([&]() {
                    auto user_id = storage.insert(user);
                    return true;
                });

                res.set_content("all gucci", "text/plain");
            } catch (const std::bad_variant_access&) { // hell naw man
                res.status = 400;
                res.set_content(missing_params_msg(std::get<std::vector<std::string>>(parsed_body)), "application/json");
            }
        }

        static void login(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            using namespace std::string_literals;
            const auto required = {"login"s, "password"s};
            auto parsed_body = validate_and_parse<std::string, std::string>(req.body, required);

            try { // everything is alright, we got the data
                auto decoded_body = std::get<std::map<std::string, std::string>>(parsed_body);
                if (auto token_pair = auth::login<wetaxi::Passenger>(storage, decoded_body["login"], decoded_body["password"])) {
                    auto token = token_pair->first;
                    auto user = token_pair->second;
                    res.set_content(json_keystring_message(token.keystring), "application/json");
                } else {
                    res.status = 400;
                    res.set_content(json_error_message("naw that didnt work"), "application/json");
                }
            } catch (const std::bad_variant_access&) { // hell naw man
                res.status = 400;
                res.set_content(missing_params_msg(std::get<std::vector<std::string>>(parsed_body)), "application/json");
            }
        }

        static void order_history(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Passenger>(storage, req)) {
                using namespace sqlite_orm;
                using json = nlohmann::json;

                auto found = storage.get_all<wetaxi::Order>(
                    where(c(&wetaxi::Order::passenger_id) == user->id)
                );

                std::vector<json> vj(0);
                for (const auto &i : found)
                    vj.push_back(json{
                        {"id", i.id},
                        {"passenger_id", i.passenger_id},
                        {"driver_id", i.driver_id},
                        {"car_id", i.car_id},
                        {"car_type", i.car_type},
                        {"route_from", i.route_from},
                        {"route_to", i.route_to},
                        {"timestamp", i.timestamp},
                        {"status", order_status_to_string(i.status)}
                    });

                json j(vj);
                std::stringstream ss;
                ss << j;
                res.set_content(ss.str(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json_error_message("log in first"), "application/json");
            }
        }

        static void payment_methods_get(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Passenger>(storage, req)) {
                using namespace sqlite_orm;
                using json = nlohmann::json;

                auto found = storage.get_all<wetaxi::PaymentMethod>(
                    where(c(&wetaxi::PaymentMethod::passenger_id) == user->id)
                );

                std::vector<json> vj(0);
                for (const auto &i : found)
                    vj.push_back(json{
                        {"id", i.id},
                        {"card_number", "**** **** **** " + i.card_number.substr(12)},
                        {"year", i.year},
                        {"month", i.month},
                        {"first_name", i.first_name},
                        {"last_name", i.last_name}
                    });

                json j(vj);
                std::stringstream ss;
                ss << j;
                res.set_content(ss.str(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json_error_message("log in first"), "application/json");
            }
        }
        
        static void payment_methods_post(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Passenger>(storage, req)) {
                using json = nlohmann::json;
                using namespace std::string_literals;
                const auto required = {"card_number"s, "cvc"s, "month"s, "year"s, "first_name"s, "last_name"s};
                
                json j = json::parse(req.body);

                std::vector<std::string> missing(0);
                for (const auto &i : required)
                    if (j.find(i) == j.end())
                        missing.push_back(i);

                if (!missing.empty()) {
                    res.status = 400;
                    res.set_content(missing_params_msg(missing), "application/json");
                    return;
                }

                wetaxi::PaymentMethod p{
                    -1, "", -1, -1, -1, "", "", -1
                };
                
                j.at("card_number").get_to(p.card_number);
                if (!wetaxi::PaymentMethod::validate_card(p.card_number)) {
                    res.status = 400;
                    res.set_content(json_error_message("ivalid card number"), "application/json");
                    return;
                }
                
                j.at("cvc").get_to(p.cvc);
                if (!wetaxi::PaymentMethod::validate_cvc(p.cvc)) {
                    res.status = 400;
                    res.set_content(json_error_message("ivalid cvc"), "application/json");
                    return;
                }

                j.at("month").get_to(p.month);
                if (!wetaxi::PaymentMethod::validate_month(p.month)) {
                    res.status = 400;
                    res.set_content(json_error_message("ivalid month"), "application/json");
                    return;
                }

                j.at("year").get_to(p.year);
                if (!wetaxi::PaymentMethod::validate_year(p.year)) {
                    res.status = 400;
                    res.set_content(json_error_message("invalid year"), "application/json");
                    return;
                }

                j.at("first_name").get_to(p.first_name);
                j.at("last_name").get_to(p.last_name);
                p.passenger_id = user->id;

                storage.transaction([&]() {
                    auto payment_id = storage.insert(p);
                    return true;
                });

                res.set_content("all gucci", "text/plain");
            } else {
                res.status = 400;
                res.set_content(json_error_message("log in first"), "application/json");
            }
        }

        static void pinned_addresses_get(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Passenger>(storage, req)) {
                using namespace sqlite_orm;
                using json = nlohmann::json;

                auto found = storage.get_all<wetaxi::PinnedAddress>(
                    where(c(&wetaxi::PinnedAddress::passenger_id) == user->id)
                );

                std::vector<json> vj(0);
                for (const auto &i : found)
                    vj.push_back(json{
                        {"id", i.id},
                        {"address", i.address}
                    });

                json j(vj);
                std::stringstream ss;
                ss << j;
                res.set_content(ss.str(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json_error_message("log in first"), "application/json");
            }
        }

        static void pinned_addresses_post(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Passenger>(storage, req)) {
                using namespace sqlite_orm;
                using json = nlohmann::json;

                json j = json::parse(req.body);
                auto got_addresses = j.get<std::vector<std::string>>();

                storage.transaction([&]() {
                    for (const auto &address : got_addresses) {
                        PinnedAddress t{
                            -1,
                            address,
                            user->id
                        };
                        auto address_id = storage.insert(t);
                    }
                    return true;
                });

                res.set_content("all gucci", "text/plain");
            } else {
                res.status = 400;
                res.set_content(json_error_message("log in first"), "application/json");
            }
        }

        // there are no guarantees that price will be the same when the order is made since 
        // the distance is unknown and i dont care enough
        static void calculate_price(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Passenger>(storage, req)) {
                using namespace sqlite_orm;
                using json = nlohmann::json;
                using namespace std::string_literals;

                const auto required = {"from", "to", "car_type"};

                json j = json::parse(req.body);

                std::vector<std::string> missing(0);
                for (const auto &i : required)
                    if (j.find(i) == j.end())
                        missing.push_back(i);

                if (!missing.empty()) {
                    res.status = 400;
                    res.set_content(missing_params_msg(missing), "application/json");
                    return;
                }

                std::string car_type_str;
                j.at("type").get_to(car_type_str);
                auto car_type = car_type_from_string(car_type_str);
                if (!car_type) {
                    res.status = 400;
                    res.set_content(json_error_message("invalid string for cartype"), "application/json");
                    return;
                }

                std::string from;
                std::string to;
                j.at("from").get_to(from);
                j.at("to").get_to(to);

                int price = calculate_price_(storage, from, to, *car_type);

                json r;
                r["estimated_price"] = price;
                std::stringstream ss;
                ss << r;

                res.set_content(ss.str(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json_error_message("log in first"), "application/json");
            }
        }

        static void place_order(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization<wetaxi::Passenger>(storage, req)) {
                using namespace sqlite_orm;
                using json = nlohmann::json;
                using namespace std::string_literals;

                const auto required = {"from", "to", "car_type", "paymend_method"};

                json j = json::parse(req.body);

                std::vector<std::string> missing(0);
                for (const auto &i : required)
                    if (j.find(i) == j.end())
                        missing.push_back(i);

                if (!missing.empty()) {
                    res.status = 400;
                    res.set_content(missing_params_msg(missing), "application/json");
                    return;
                }

                wetaxi::Order order{
                    -1, -1, -1, -1, wetaxi::CarType::economy, 0, -1, "", "", "", wetaxi::OrderStatus::pending
                };

                std::string car_type_str;
                j.at("type").get_to(car_type_str);
                auto car_type = car_type_from_string(car_type_str);
                if (!car_type) {
                    res.status = 400;
                    res.set_content(json_error_message("invalid string for cartype"), "application/json");
                    return;
                }

                order.car_type = *car_type;

                j.at("from").get_to(order.route_from);
                j.at("to").get_to(order.route_to);
                j.at("payment_method").get_to(order.payment_method_id);
                order.passenger_id = user->id;
                order.price = calculate_price_(storage, order.route_from, order.route_to, order.car_type);

                storage.insert(order);
                res.set_content("all gucci", "text/plain");
            } else {
                res.status = 400;
                res.set_content(json_error_message("log in first"), "application/json");
            }
        }
    };
}