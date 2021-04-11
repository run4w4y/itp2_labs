#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <sqlite_orm/sqlite_orm.h>
#include "../storage.h"
#include "auth_token.h"


namespace wetaxi::auth {
    namespace {
        // fucking gross
        std::string format_time(time_t t, std::string format_str) {
            std::tm tt = *std::localtime(&t);
            std::stringstream ss;
            ss << std::put_time(&tt, format_str.c_str());
            return ss.str();
        }

        std::optional<time_t> parse_time(std::string s, std::string format_str) {
            std::tm tt;
            std::stringstream ss(s);
            ss >> std::get_time(&tt, format_str.c_str());
            
            if (ss.fail()) 
                return std::nullopt;
            
            return std::optional<time_t>{mktime(&tt)};
        }

        const std::string KEYSTRING_SALT = "ssssalty";
        const std::string DEFAULT_DATETIME_FORMAT = "%Y-%m-%d %H:%M";
    }

    // why the fuck do i have to care about security when im literally not using ssl
    std::string hash_pass(std::string s) {
        using namespace CryptoPP;

        s += KEYSTRING_SALT;
        SHA256 hash;
        std::string digest;

        StringSource ss(s, true, new HashFilter(hash, new HexEncoder(new StringSink(digest))));

        return digest;
    }

    template<typename UserT>
    std::optional<UserT> user_by_keystring(wetaxi::storage::Storage &storage, std::string s) {
        using namespace sqlite_orm;

        auto found_token = storage.get_all<AuthToken>(
            where(c(&AuthToken::keystring) == s),
            limit(1)
        );

        if (found_token.empty())
            return std::nullopt; // token not found

        AuthToken token = found_token[0];
        if (auto expires_ts = parse_time(token.expires, DEFAULT_DATETIME_FORMAT)) {
            auto expires_tp = std::chrono::system_clock::from_time_t(*expires_ts);
            if (std::chrono::system_clock::now() >= expires_tp) // TODO: might wanna delete the expired token
                return std::nullopt; // token expired
        } else
            return std::nullopt; // couldnt parse the date
        
        if (auto found_user = storage.get_pointer<UserT>(token.user_id)) 
            return *found_user;
        else
            return std::nullopt; // user not found
    }

    template<typename UserT>
    AuthToken issue_token(wetaxi::storage::Storage &storage, UserT user) {
        auto current_timepoint = std::chrono::system_clock::now();
        time_t current_ts = std::chrono::system_clock::to_time_t(current_timepoint);
        std::string current_timestring = format_time(current_ts, DEFAULT_DATETIME_FORMAT);
        std::string raw_keystring = current_timestring + user.login;
        std::string keystring = hash_pass(raw_keystring);

        auto expires_timepoint = current_timepoint + std::chrono::hours(48);
        time_t expires_ts = std::chrono::system_clock::to_time_t(expires_timepoint);
        std::string expires_timestring = format_time(expires_ts, DEFAULT_DATETIME_FORMAT);

        AuthToken token{-1, keystring, current_timestring, expires_timestring, user.id};
        
        using namespace sqlite_orm;

        auto token_id = storage.insert(token);
        token.id = token_id;
        
        return token;
    }

    template<typename UserT>
    std::optional<std::pair<AuthToken, UserT>> login(wetaxi::storage::Storage &storage, std::string login, std::string password) {
        using namespace sqlite_orm;

        auto found = storage.get_all<UserT>(
            where(c(&UserT::login) == login && c(&UserT::password) == hash_pass(password)),
            limit(1)
        );

        if (found.empty())
            return std::nullopt;
        
        UserT user = found[0];
        AuthToken token = issue_token<UserT>(storage, user);

        return std::optional<std::pair<AuthToken, UserT>>{std::make_pair(token, user)};
    }
}