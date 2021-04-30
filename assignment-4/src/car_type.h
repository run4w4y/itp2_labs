#pragma once

#include <string>
#include <memory>


namespace wetaxi {
    enum CarType {
        economy, comfort, comfort_plus, business
    };

    std::string car_type_to_string(wetaxi::CarType car_type) {
        switch (car_type) {
            case wetaxi::CarType::economy:
                return "economy";
            case wetaxi::CarType::comfort:
                return "comfort";
            case wetaxi::CarType::comfort_plus:
                return "comfort_plus";
            case wetaxi::CarType::business:
                return "business";
        }
        throw std::domain_error("invalid cartype enum");
    }

    std::unique_ptr<wetaxi::CarType> car_type_from_string(const std::string &s) {
        if (s == "economy")
            return std::make_unique<wetaxi::CarType>(wetaxi::CarType::economy);
        if (s == "comfort")
            return std::make_unique<wetaxi::CarType>(wetaxi::CarType::comfort);
        if (s == "comfort_plus")
            return std::make_unique<wetaxi::CarType>(wetaxi::CarType::comfort_plus);
        if (s == "business")
            return std::make_unique<wetaxi::CarType>(wetaxi::CarType::business);
        return nullptr;
    }
}

namespace sqlite_orm {
    template<>
    struct type_printer<wetaxi::CarType> : public text_printer {};

    template<>
    struct statement_binder<wetaxi::CarType> {
        int bind(sqlite3_stmt *stmt, int index, const wetaxi::CarType &value) {
            return statement_binder<std::string>().bind(stmt, index, wetaxi::car_type_to_string(value));
        }
    };

    template<>
    struct field_printer<wetaxi::CarType> {
        std::string operator()(const wetaxi::CarType &t) const {
            return wetaxi::car_type_to_string(t);
        }
    };

    template<>
    struct row_extractor<wetaxi::CarType> {
        wetaxi::CarType extract(const char *row_value) {
            return *wetaxi::car_type_from_string(row_value);
        }

        wetaxi::CarType extract(sqlite3_stmt *stmt, int column_index) {
            auto str = sqlite3_column_text(stmt, column_index);
            return this->extract((const char*) str);
        }
    };
}