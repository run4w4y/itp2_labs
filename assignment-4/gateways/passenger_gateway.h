#pragma once

#include <map>
#include <variant>
#include <vector>
#include <string>
#include <initializer_list>
#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <sqlite_orm/sqlite_orm.h>
#include "../storage.h"
#include "../passenger.h"


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

        public:
        // TODO: implement password hashing
        static void signup(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            using namespace std::string_literals;
            const auto required = {"login"s, "password"s, "first_name"s, "last_name"s};
            auto parsed_body = validate_and_parse<std::string, std::string>(req.body, required);
            
            try { // everything is alright, we got the data
                auto decoded_body = std::get<std::map<std::string, std::string>>(parsed_body);

                wetaxi::Passenger user(
                    decoded_body["login"], // TODO: actually logins should be unique bruh
                    decoded_body["password"], 
                    decoded_body["first_name"], 
                    decoded_body["last_name"]
                );

                storage.transaction([&]() {
                    auto user_id = storage.insert(user);
                    std::cout << "new userid: " << user_id << std::endl;
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

                {
                    using namespace sqlite_orm;

                    using User = wetaxi::Passenger;

                    auto found = storage.get_all<User>(
                        where(c(&User::login) == decoded_body["login"] && c(&User::password) == decoded_body["password"]),
                        limit(1)
                    );

                    if (found.empty()) {
                        res.set_content("no such user found", "text/plain");
                        return;
                    }

                    auto user = found[0];
                    res.set_content("successfully logged in as " + user.first_name + " " + user.last_name, "text/plain");
                }
            } catch (const std::bad_variant_access&) { // hell naw man
                res.set_content(missing_params_msg(std::get<std::vector<std::string>>(parsed_body)), "text/plain");
            }
        }
    };
}