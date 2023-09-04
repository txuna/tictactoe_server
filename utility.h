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
}

#endif 