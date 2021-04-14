#pragma once

#include <variant>
#include <vector>
#include <string>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <initializer_list>
#include <algorithm>
#include "../storage.h"
#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <sqlite_orm/sqlite_orm.h>


namespace wetaxi {
    class Gateway {
        protected:
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

        static std::string json_message(std::string k, std::string v) {
            using json = nlohmann::json;
            json j = json{
                {k, v}
            };
            std::stringstream ss;
            ss << j;
            return ss.str();
        }

        static std::string json_error_message(std::string s) {
            return json_message("error_message", s);
        }
        
        static std::string json_keystring_message(std::string keystring) {
            return json_message("token", keystring);
        }

        static inline std::string missing_params_msg(std::vector<std::string> &missing_params) {
            return json_error_message(
                "missing required parameters: " + 
                    std::accumulate(std::begin(missing_params), std::end(missing_params), std::string(), 
                        [](std::string &s1, std::string &s2) {
                            return s1.empty() ? s2 : s1 + ", " + s2;
                        }
                    )
            );
        }

        template<typename UserT>
        static std::optional<UserT> get_authorization(wetaxi::storage::Storage &storage, const httplib::Request &req) {
            if (!req.has_header("Authorization"))
                return std::nullopt;

            auto val = req.get_header_value("Authorization");
            std::string keystring = val.substr(val.find(" ") + 1);

            if (auto user = auth::user_by_keystring<UserT>(storage, keystring))
                return *user;
            else
                return std::nullopt;
        }

        // based off some shitty formula i found online
        // since i dont really know the distance/traffic i dont give a fuck either way
        static int calculate_price_(wetaxi::storage::Storage &storage, const std::string &from, const std::string &to, wetaxi::CarType car_type) {
            using namespace sqlite_orm;

            std::srand(std::time(nullptr)); // seed
            double distance = std::rand() % 30; // some random ass distance in km, doing it this way is biased af but idc 
            double ride_time = distance / 60; // estimated time, whatever
            double cost_per_minute = 4.2; // i dont really know how to calculate it so its just whatever
            double cost_per_km = 3.1; // same as cost_per_minute
            double booking_fee = 10; // we get 10 rub from each ride wooo
            double surge_price = .25 + 10 / storage.get_all<wetaxi::Driver>( // since the end result is ass anyways i didnt bother too much
                where(c(&wetaxi::Driver::is_busy) == true && c(&wetaxi::Driver::is_working) == true)
            ).size();
            double base_fare; // literally just base fare, different for each car type
            switch (car_type) {
                case wetaxi::CarType::economy:
                    base_fare = 20;
                case wetaxi::CarType::comfort:
                    base_fare = 30;
                case wetaxi::CarType::comfort_plus:
                    base_fare = 40;
                case wetaxi::CarType::business:
                    base_fare = 65;
            }
            return base_fare + (cost_per_minute * ride_time + cost_per_km * distance) * surge_price + booking_fee;
        }
    };
}