#pragma once

#include <array>
#include <string_view>
#include <algorithm>
#include <ranges>
#include <stdexcept>

template <typename EnumType, std::size_t N>
class EnumMapper {
private:
    using EnumToString = std::pair<EnumType, std::string_view>;
    std::array<EnumToString, N> m_enumToStringList;

public:
    explicit constexpr EnumMapper(std::array<EnumToString, N> pairs) : m_enumToStringList(pairs) {}

    [[nodiscard]] constexpr std::string_view toString(EnumType e) const {
        const auto it { std::ranges::find_if(m_enumToStringList, [e](const auto& p) { return p.first == e; }) };
        return (it != m_enumToStringList.end()) ? it->second : throw std::invalid_argument("Invalid enum type value");
    }

    [[nodiscard]] constexpr EnumType fromString(std::string_view s) const {
        const auto it { std::ranges::find_if(m_enumToStringList, [s](const auto& p) { return p.second == s; }) };
        return (it != m_enumToStringList.end()) ? it->first : throw std::invalid_argument("Invalid enum string value");
    }

    [[nodiscard]] constexpr bool isValid(std::string_view s) const {
        return std::ranges::any_of(m_enumToStringList, [s](const auto& p) { return p.second == s; });
    }
};

template <typename EnumType, typename... Args>
[[nodiscard]] constexpr auto makeEnumMapper(std::pair<EnumType, std::string_view> first, Args... rest) {
    constexpr auto N { 1 + sizeof...(Args) };
    return EnumMapper<EnumType, N>(std::array<std::pair<EnumType, std::string_view>, N> { first, rest... });
}
