#pragma once


namespace wetaxi::auth {
    // thats actually kinda weird to use since passengers and drivers are 
    // two separate entities but i dont give a flying fuck
    struct AuthToken { 
        int id;
        std::string keystring;
        std::string issued;
        std::string expires;
        int user_id;
    };
}