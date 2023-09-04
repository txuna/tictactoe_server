#ifndef __MODEL_H
#define __MODEL_H

#include <string>
#include <stdint.h>

namespace Model
{
    class Account
    {
        public:
            int user_id; 
            std::string email;
            std::string password; 
            std::string salt;
            std::string name;

            Account(int user_id, 
                        std::string email, 
                        std::string password, 
                        std::string salt, 
                        std::string name);
                        
            ~Account();
    };
}

#endif 