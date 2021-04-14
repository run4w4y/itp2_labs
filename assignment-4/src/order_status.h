#pragma once

#include <string>
#include <memory>


namespace wetaxi {
    enum OrderStatus {
        pending, in_progress, finished
    };

    std::string order_status_to_string(wetaxi::OrderStatus car_type) {
        switch (car_type) {
            case wetaxi::OrderStatus::pending:
                return "pending";
            case wetaxi::OrderStatus::in_progress:
                return "in progress";
            case wetaxi::OrderStatus::finished:
                return "finished";
        }
        throw std::domain_error("invalid OrderStatus enum");
    }

    std::unique_ptr<wetaxi::OrderStatus> order_status_from_string(const std::string &s) {
        if (s == "pending")
            return std::make_unique<wetaxi::OrderStatus>(wetaxi::OrderStatus::pending);
        if (s == "in_progress")
            return std::make_unique<wetaxi::OrderStatus>(wetaxi::OrderStatus::in_progress);
        if (s == "finished")
            return std::make_unique<wetaxi::OrderStatus>(wetaxi::OrderStatus::finished);
        return nullptr;
    }
}

namespace sqlite_orm {
    template<>
    struct type_printer<wetaxi::OrderStatus> : public text_printer {};

    template<>
    struct statement_binder<wetaxi::OrderStatus> {
        int bind(sqlite3_stmt *stmt, int index, const wetaxi::OrderStatus &value) {
            return statement_binder<std::string>().bind(stmt, index, wetaxi::order_status_to_string(value));
        }
    };

    template<>
    struct field_printer<wetaxi::OrderStatus> {
        std::string operator()(const wetaxi::OrderStatus &t) const {
            return wetaxi::order_status_to_string(t);
        }
    };

    template<>
    struct row_extractor<wetaxi::OrderStatus> {
        wetaxi::OrderStatus extract(const char *row_value) {
            return *wetaxi::order_status_from_string(row_value);
        }

        wetaxi::OrderStatus extract(sqlite3_stmt *stmt, int column_index) {
            auto str = sqlite3_column_text(stmt, column_index);
            return this->extract((const char*) str);
        }
    };
}