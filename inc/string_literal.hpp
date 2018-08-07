#pragma once

template < char... chrs > struct string_literal
{
    constexpr const char* str()
    {
        constexpr char str[sizeof...(chrs) + 1] = { chrs..., 0 };
        return str;
    }
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
template < typename T, T... chrs >
constexpr string_literal< chrs... > operator""_tstr()
{
    return {};
}
#pragma clang diagnostic pop
