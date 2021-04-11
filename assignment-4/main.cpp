#include <iostream>
#include <string>
#include <functional>
#include <httplib/httplib.h>
#include "gateways/passenger_gateway.h"
#include "storage.h"
#include "passenger.h"


int main() {
    using namespace wetaxi::storage;

    Storage storage = storage_init("db.sqlite");
    storage.sync_schema();

    using IncompleteHandlerF = std::function<void(Storage &, const httplib::Request &, httplib::Response &)>;
    using HandlerF = std::function<void(const httplib::Request &, httplib::Response &)>;
    auto bind_storage = [&](IncompleteHandlerF f) {
        using namespace std::placeholders;
        return (HandlerF) std::bind(f, storage, _1, _2);
    };

    httplib::Server server;
    server.Post("/passenger/signup", bind_storage(wetaxi::PassengerGateway::signup));
    server.Post("/passenger/login", bind_storage(wetaxi::PassengerGateway::login));
    server.Get("/passenger/order_history", bind_storage(wetaxi::PassengerGateway::order_history));
    server.Get("/passenger/payment_methods", bind_storage(wetaxi::PassengerGateway::payment_methods_get));
    server.Post("/passenger/payment_methods", bind_storage(wetaxi::PassengerGateway::payment_methods_post));
    
    const std::string ip = "0.0.0.0";
    const int port = 8888;
    std::cout << "we are up and running on " << ip << ":" << port << " buddy" << std::endl;
    server.listen("0.0.0.0", 8888);
}