#pragma once
#include <httplib/httplib.h>
#include "../storage.h"
#include "../passenger.h"


namespace wetaxi {
    struct PassengerGateway {
        static void signup_passenger(const httplib::Request &req, httplib::Response &res) {
            const auto required_params = {"login", "password", "first_name", "last_name"};
            std::map<std::string, std::string> params;
            std::vector<std::string> missing_params(0);
            
            bool params_flag = true;
            for (const auto &p : required_params)
                if (req.has_param(p))
                    params[p] = req.get_param_value(p);
                else 
                    missing_params.push_back(p);
            
            if (!missing_params.empty()) {
                std::string body = "missing required parameters: ";

                std::string t = std::accumulate(std::begin(missing_params), std::end(missing_params), std::string(), 
                    [](std::string &s1, std::string &s2) {
                        return s1.empty() ? s2 : s1 + ", " + s2;
                    }
                );
                body += t;

                res.set_content(body, "text/plain");
                return;
            }

            for (const auto &[p, v] : params)
                std::cout << p << " = " << v << std::endl;
            
            wetaxi::Passenger user(
                params["login"], 
                params["password"], 
                params["first_name"], 
                params["last_name"]
            );

            storage::default_storage.transaction([&]() {
                auto user_id = storage::default_storage.insert(user); // that doesnt work, i have no idea why
                std::cout << "new userid: " << user_id << std::endl;
                return true;
            });

            res.set_content("all gucci", "text/plain");
        }
    };
}