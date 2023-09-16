#pragma once

#include "language/assert.hpp" // phudAssert
#include "strings/String.hpp" // StringView


class [[nodiscard]] SqlSelector final {
private:
  String m_query;

public:
  explicit SqlSelector(StringView sqlTemplate) : m_query { sqlTemplate } {
    phudAssert(phud::strings::contains(sqlTemplate, '?'), "The given sqlTemplate should contain '?'");
  }

  [[nodiscard]] String toString() {
    phudAssert(!phud::strings::contains(m_query, '?'),
               "At least one variable still needs to be replaced");
    return m_query;
  }

  [[nodiscard]] SqlSelector& site(StringView site) {
    m_query = phud::strings::replaceAll(m_query, "?siteName", site);
    return *this;
  }

  [[nodiscard]] SqlSelector& table(StringView table) {
    m_query.replace(m_query.find("?tableName"), phud::strings::length("?tableName"), table);
    return *this;
  }

  [[nodiscard]] SqlSelector& players(StringView players) {
    m_query.replace(m_query.find("?playerNames"), phud::strings::length("?playerNames"), players);
    return *this;
  }

  [[nodiscard]] SqlSelector& player(StringView playerName) {
    m_query = phud::strings::replaceAll(m_query, "?playerName", playerName);
    return *this;
  }
}; // class SqlSelector