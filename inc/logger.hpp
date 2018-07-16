#pragma once

#include <iostream>

namespace logger
{
    template < typename... Ts > void eater( Ts&&... )
    {
    }

    template < typename... Ts > constexpr void log( Ts&&... args )
    {
        eater( ( ( std::cout << args ), 0 )... );
        std::cout << std::endl;
    }
} // namespace logger
