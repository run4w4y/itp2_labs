#pragma once

#include <map>
#include <variant>
#include <vector>
#include <string>
#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include "../storage.h"
#include "../passenger.h"


namespace wetaxi {
    class PassengerGateway {
        public: 
        // TODO: implement password hashing
        static void signup_passenger(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
            using json = nlohmann::json;

            const auto required_params = {"login", "password", "first_name", "last_name"};
            
            json encoded_body = json::parse(req.body);
            auto decoded_body = encoded_body.get<std::map<std::string, std::string>>();
            std::vector<std::string> missing_params(0);
            
            for (const auto &p : required_params)
                if (decoded_body.find(p) == std::end(decoded_body)) 
                    missing_params.push_back(p);
            
            if (!missing_params.empty()) {
                std::string body = "missing required parameters: " + 
                    std::accumulate(std::begin(missing_params), std::end(missing_params), std::string(), 
                        [](std::string &s1, std::string &s2) {
                            return s1.empty() ? s2 : s1 + ", " + s2;
                        }
                    );
                res.set_content(body, "text/plain");
                return;
            }

            for (const auto &[p, v] : decoded_body)
                std::cout << p << " = " << v << std::endl;
            
            wetaxi::Passenger user(
                decoded_body["login"], 
                decoded_body["password"], 
                decoded_body["first_name"], 
                decoded_body["last_name"]
            );

            storage.transaction([&]() {
                auto user_id = storage.insert(user); // that doesnt work, i have no idea why
                std::cout << "new userid: " << user_id << std::endl;
                return true;
            });

            res.set_content("all gucci", "text/plain");
        }
    };

    static void login_passenger(wetaxi::storage::Storage &storage, const httplib::Request &req, httplib::Response &res) {
        using json = nlohmann::json;

        const auto required_params = {"login", "password"};
        
        json encoded_body = json::parse(req.body);
        auto decoded_body = encoded_body.get<std::map<std::string, std::string>>();            
        std::vector<std::string> missing_params(0);
        
        for (const auto &p : required_params)
            if (decoded_body.find(p) == std::end(decoded_body)) 
                missing_params.push_back(p);
        
        if (!missing_params.empty()) {
            std::string body = "missing required parameters: " + 
                std::accumulate(std::begin(missing_params), std::end(missing_params), std::string(), 
                    [](std::string &s1, std::string &s2) {
                        return s1.empty() ? s2 : s1 + ", " + s2;
                    }
                );
            res.set_content(body, "text/plain");
            return;
        }
    }
}