#include "middleware.h"

Middleware::AuthMiddleware::AuthMiddleware()
{

}

Middleware::AuthMiddleware::~AuthMiddleware()
{

}

/*
    Login, Register제외 토큰인가 확인
*/
bool Middleware::AuthMiddleware::VerityToken()
{
    return true;
}