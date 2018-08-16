// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries. 
// For conditions of distribution and use, see copying.txt file in root folder.

#pragma once

#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>

#include "logger.hpp"

inline std::vector< unsigned char >
load_binary_data( std::string_view file_name )
{
    std::vector< unsigned char > ret;
    std::ifstream input( file_name.data(), std::ios::binary );

    if ( !input.good() )
    {
        logger::log( "File: ", file_name.data(), " does not exist" );
        return ret;
    }

    std::copy( std::istreambuf_iterator< char >( input ),
            {}, std::back_inserter( ret ) );

    return ret;
}
