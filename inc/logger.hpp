#pragma once

#include <iostream>

namespace logger
{
    template < typename... Ts > constexpr void log( Ts&&... args )
    {
        const int a[] = {( ( std::cout << args ), 0 )...};
        ( void )a;
        std::cout << std::endl;
    }
} // namespace logger
