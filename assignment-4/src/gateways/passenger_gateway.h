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
#include "auth.h"
#include "auth_token.h"


namespace wetaxi {
    // i have no idea why is this a class when literally every single member function in it is static 
    class PassengerGateway {
        private:
        template<typename K, typename V>
        static std::variant<std::map<K, V>, std::vector<K>> validate_and_parse(std::string s, std::initializer_list<const K> required) { 
            using json = nlohmann::json;

            json encoded = json::parse(s);
            auto decoded = encoded.get<std::map<K, V>>();
            std::vector<K> missing(0);

            for (const auto &p : required) 
                if (decoded.find(p) == std::end(decoded))
                    missing.push_back(p);
            
            if (!missing.empty())
                return missing;
            
            return decoded;
        }

        static inline std::string missing_params_msg(std::vector<std::string> &missing_params) {
            return "missing required parameters: " + 
                std::accumulate(std::begin(missing_params), std::end(missing_params), std::string(), 
                    [](std::string &s1, std::string &s2) {
                        return s1.empty() ? s2 : s1 + ", " + s2;
                    }
                );
        }

        static std::optional<wetaxi::Passenger> get_authorization(wetaxi::storage::Storage &storage, const httplib::Request &req) {
            if (!req.has_header("Authorization"))
                return std::nullopt;

            auto val = req.get_header_value("Authorization");
            std::string keystring = val.substr(val.find(" ") + 1);

            if (auto user = auth::user_by_keystring<wetaxi::Passenger>(storage, keystring))
                return *user;
            else
                return std::nullopt;
        }

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
                res.set_content(missing_params_msg(std::get<std::vector<std::string>>(parsed_body)), "text/plain");
            }
        }

        // TODO: need them tokens (and yeah password hashing too)
        static void login(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            using namespace std::string_literals;
            const auto required = {"login"s, "password"s};
            auto parsed_body = validate_and_parse<std::string, std::string>(req.body, required);

            try { // everything is alright, we got the data
                auto decoded_body = std::get<std::map<std::string, std::string>>(parsed_body);
                if (auto token_pair = auth::login<wetaxi::Passenger>(storage, decoded_body["login"], decoded_body["password"])) {
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

        static void order_history(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization(storage, req)) {
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

        static void payment_methods_get(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization(storage, req)) {
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
            } else
                res.set_content("log in first", "text/plain");
        }
        
        static void payment_methods_post(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            if (auto user = get_authorization(storage, req)) {
                using json = nlohmann::json;
                using namespace std::string_literals;
                const auto required = {"card_number"s, "cvc"s, "month"s, "year"s, "first_name"s, "last_name"s};
                
                json j = json::parse(req.body);

                std::vector<std::string> missing(0);
                for (const auto &i : required)
                    if (j.find(i) == j.end())
                        missing.push_back(i);

                if (!missing.empty()) {
                    res.set_content(missing_params_msg(missing), "text/plain");
                    return;
                }

                wetaxi::PaymentMethod p{
                    -1, "", -1, -1, -1, "", "", -1
                };
                
                j.at("card_number").get_to(p.card_number);
                if (!wetaxi::PaymentMethod::validate_card(p.card_number)) {
                    res.set_content("ivalid card number", "text/plain");
                    return;
                }
                
                j.at("cvc").get_to(p.cvc);
                if (!wetaxi::PaymentMethod::validate_cvc(p.cvc)) {
                    res.set_content("ivalid cvc", "text/plain");
                    return;
                }

                j.at("month").get_to(p.month);
                if (!wetaxi::PaymentMethod::validate_month(p.month)) {
                    res.set_content("ivalid month", "text/plain");
                    return;
                }

                j.at("year").get_to(p.year);
                if (!wetaxi::PaymentMethod::validate_year(p.year)) {
                    res.set_content("invalid year", "text/plain");
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
            } else
                res.set_content("log in first", "text/plain");
        }

        static void pinned_addresses_get(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            // empty for now
        }

        static void pinned_addresses_post(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            // empty for now
        }
    };
}