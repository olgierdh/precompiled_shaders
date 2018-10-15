// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries.
// For conditions of distribution and use, see copying.txt file in root folder.

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
