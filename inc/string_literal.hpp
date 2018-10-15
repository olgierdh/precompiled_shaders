// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries.
// For conditions of distribution and use, see copying.txt file in root folder.

#pragma once

template < char... chrs > struct string_literal
{
    constexpr const char* str() const noexcept
    {
        constexpr char str[sizeof...( chrs ) + 1] = {chrs..., 0};
        return str;
    }
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
template < typename T, T... chrs > constexpr string_literal< chrs... > operator""_tstr()
{
    return {};
}
#pragma clang diagnostic pop
