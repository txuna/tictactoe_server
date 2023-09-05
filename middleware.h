#ifndef __MIDDLEWARE_H_
#define __MIDDLEWARE_H_

#include "common.h"

namespace Middleware
{
    class AuthMiddleware
    {
        public:
            AuthMiddleware();
            ~AuthMiddleware();
            bool VerityToken();
    };
}

#endif 

