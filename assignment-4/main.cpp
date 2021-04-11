#include <iostream>
#include <httplib/httplib.h>
#include "gateways/passenger_gateway.h"
#include "storage.h"


int main() {
    httplib::Server server;

    server.Get("/test", [](const httplib::Request &req, httplib::Response &res) {
        res.set_content("test", "text/plain");
    });

    server.Post("/signup_passenger", &wetaxi::PassengerGateway::signup_passenger);
    
    std::cout << "we are up and running buddy" << std::endl;
    server.listen("0.0.0.0", 8888);
}