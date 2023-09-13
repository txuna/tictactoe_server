#ifndef __UTILITY_H
#define __UTILITY_H

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <random>

namespace Utility
{
    class Security
    {
        public:
            static std::string GenerateHash(std::string password, std::string salt)
            {
                unsigned char hash[SHA512_DIGEST_LENGTH] = {0};
                std::string plain = password + salt;
                unsigned int len = plain.size();
                EVP_MD_CTX *evpCtx = EVP_MD_CTX_new();
                EVP_DigestInit_ex (evpCtx, EVP_sha512 (), NULL);

                EVP_DigestUpdate(evpCtx, plain.c_str(), len);
                EVP_DigestFinal_ex(evpCtx, hash, &len);

                std::stringstream ss; 
                for(int i=0; i<SHA512_DIGEST_LENGTH; i++)
                {
                    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
                }
                
                return ss.str();
            }

            static std::string GenerateSalt(int length)
            {
                std::string randomString;
                randomString.reserve(length);

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dist('a', 'z'); // 알파벳 범위 ('a' ~ 'z')로 수정

                for (int i = 0; i < length; ++i) {
                    randomString.push_back(static_cast<char>(dist(gen)));
                }

                return randomString;
            }

            static std::string GenerateToken()
            {
                std::string plain = Utility::Security::GenerateSalt(24);
                return Utility::Security::GenerateHash(plain, "");
            }
    };  

    class Validation 
    {
        public:            
            static bool VerifyEmail(std::string email)
            {
                const std::regex email_pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
                if(std::regex_match(email, email_pattern) == false)
                {
                    return false;
                }

                if(email.length() < 8 || email.length() > 40)
                {
                    return false;
                }

                return true;
            }

            static bool VerifyUserName(std::string name)
            {
                const std::regex username_pattern("^[a-zA-Z_][a-zA-Z0-9_]*$");
                if(std::regex_match(name, username_pattern) == false)
                {
                    return false;
                }

                if(name.length() < 8 || name.length() > 20)
                {
                    return false;
                }
                
                return true;
            }

            static bool VerifyPassword(std::string password)
            {
                if(password.length() < 8 || password.length() > 26)
                {
                    return false;
                }
                return true;
            }

            static bool VerifyRoomTitle(std::string title)
            {
                const std::regex title_pattern("^[a-zA-Z_][a-zA-Z0-9_]*$");

                if(std::regex_match(title, title_pattern) == false)
                {
                    return false;
                }

                if(title.length() < 8 || title.length() > 40)
                {
                    return false;
                }

                return true;
            }
    };
}

#endif 