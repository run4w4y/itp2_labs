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

        const std::string KEYSTRING_SALT = "nigger";
        const std::string DEFAULT_DATETIME_FORMAT = "%Y-%m-%d %H:%M";
    }

    // why the fuck do i have to care about security when im literally not using ssl
    std::string hash_pass(std::string s) {
        using namespace CryptoPP;

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
            return std::nullopt;
        
        if (auto found_user = storage.get_pointer<UserT>(found_token[0].user_id)) 
            return *found_user;
        else
            return std::nullopt;
    }

    template<typename UserT>
    AuthToken issue_token(wetaxi::storage::Storage &storage, UserT user) {
        // take current timestamp add some random shit and we get our token whatever i dont fucking care anyways
        // need to make a table in db too fuck piss shit

        // what in the actual fuck is wrong with c++ god i hate this
        auto current_timepoint = std::chrono::system_clock::now();
        time_t current_ts = std::chrono::system_clock::to_time_t(current_timepoint);
        std::string current_timestring = format_time(current_ts, DEFAULT_DATETIME_FORMAT);
        std::string raw_keystring = current_timestring + user.login + KEYSTRING_SALT;
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
            where(c(&UserT::login) == login && c(&UserT::password) == password),
            limit(1)
        );

        if (found.empty())
            return std::nullopt;
        
        UserT user = found[0];
        AuthToken token = issue_token<UserT>(storage, user);

        return std::optional<std::pair<AuthToken, UserT>>{std::make_pair(token, user)};
    }
}