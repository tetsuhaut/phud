#pragma once

#include <array>
#include <string_view>
#include <algorithm>
#include <ranges>
#include <stdexcept>

template<typename EnumType, std::size_t N>
class EnumMapper {
private:
    using EnumToString = std::pair<EnumType, std::string_view>;
    std::array<EnumToString, N> m_enumToStringList;

public:
    constexpr EnumMapper(std::array<EnumToString, N> pairs) : m_enumToStringList(pairs) {}

    [[nodiscard]] constexpr std::string_view toString(EnumType e) const {
        auto it = std::ranges::find_if(m_enumToStringList, [e](const auto& p) { return p.first == e; });
        return it != m_enumToStringList.end() ? it->second : "";
    }

    [[nodiscard]] constexpr EnumType fromString(std::string_view s) const {
        if (auto it = std::ranges::find_if(m_enumToStringList, [s](const auto& p) { return p.second == s; }); it != m_enumToStringList.end()) {
          return it->first;
        }
        throw std::invalid_argument("Invalid enum string value");
    }

    [[nodiscard]] constexpr bool isValid(std::string_view s) const {
        return std::ranges::any_of(m_enumToStringList, [s](const auto& p) { return p.second == s; });
    }
};

template<typename EnumType, std::size_t N>
constexpr auto makeEnumMapper(const std::array<std::pair<EnumType, std::string_view>, N>& pairs) {
  return EnumMapper<EnumType, N>(pairs);
}
