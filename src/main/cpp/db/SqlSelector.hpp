#pragma once

#include "language/Validator.hpp"
#include "strings/StringUtils.hpp" // std::string_view

class [[nodiscard]] SqlSelector final {
private:
  std::string m_query;

public:
  explicit SqlSelector(std::string_view sqlTemplate) : m_query { sqlTemplate } {
    validation::require(phud::strings::contains(sqlTemplate, '?'), "The given sqlTemplate should contain '?'");
  }

  [[nodiscard]] std::string toString() {
    validation::require(!phud::strings::contains(m_query, '?'),
               "At least one variable still needs to be replaced");
    return m_query;
  }

  [[nodiscard]] SqlSelector& site(std::string_view site) {
    m_query = phud::strings::replaceAll(m_query, "?siteName", site);
    return *this;
  }

  [[nodiscard]] SqlSelector& table(std::string_view table) {
    m_query = phud::strings::replaceAll(m_query, "?tableName", table);
    return *this;
  }

  [[nodiscard]] SqlSelector& player(std::string_view playerName) {
    m_query = phud::strings::replaceAll(m_query, "?playerName", playerName);
    return *this;
  }
}; // class SqlSelector
