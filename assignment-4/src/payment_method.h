#pragma once

#include <string>
#include <cctype>


namespace wetaxi {
    struct PaymentMethod {
        int id;
        std::string card_number;
        int year;
        int month;
        int cvc;
        std::string first_name;
        std::string last_name;
        int passenger_id;

        static bool validate_card(std::string card_number) {
            return std::all_of(card_number.begin(), card_number.end(), ::isdigit) && card_number.size() == 16;
        }

        static bool validate_cvc(int cvc) {
            return cvc >= 100 && cvc <= 999;
        }

        static bool validate_month(int month) {
            return month >= 1 && month <= 12;
        }

        static bool validate_year(int year) {
            return year >= 2020 && year <= 9999;
        }
    };
}