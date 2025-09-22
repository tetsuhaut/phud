#include "db/SqlInsertor.hpp" // phud::strings
#include "entities/Action.hpp"
#include "entities/Card.hpp"
#include "entities/Game.hpp" // Limit, Variant
#include "entities/Seat.hpp" // tableSeat
#include "language/Validator.hpp"
#include "strings/StringUtils.hpp" // phud::strings::*

namespace ps = phud::strings;

[[nodiscard]] inline std::string toString(auto s) = delete;
template<> inline std::string toString<std::string_view>(std::string_view s) { return std::string(s); }
template<> inline std::string toString<char>(char s) { return std::string(1, s); }
template<> inline std::string toString<const char*>(const char* const s) { return s; }
template<> inline std::string toString<bool>(bool s) { return s ? "1" : "0"; }
template<> inline std::string toString<int>(int s) { return std::to_string(s); }
template<> inline std::string toString<std::size_t>(std::size_t s) { return std::to_string(s); }
template<> inline std::string toString<double>(double s) { return std::to_string(s); }
template<> inline std::string toString<Seat>(Seat seat) { return tableSeat::toString(seat).data(); }

static std::string_view isOk(std::string_view s) {
  validation::require(ps::contains(s, '('), "The given sqlTemplate should contain '('");
  validation::require(ps::contains(s, ')'), "The given sqlTemplate should contain ')'");
  validation::require(ps::contains(s, '?'), "The given sqlTemplate should contain '?'");
  return s;
}

// sqlTemplate contains 'INSERT INTO <table> (<columns>) VALUES (<model of values>);'
// or 'INSERT OR IGNORE INTO <table> (<columns>) VALUES (<model of values>);'
// m_query contains 'INSERT INTO <table> (<columns>) VALUES '
// m_valueModel contains <model of values>
SqlInsertor::SqlInsertor(std::string_view sqlTemplate)
  : m_query { isOk(sqlTemplate).substr(0, sqlTemplate.rfind('(')) },
    m_valueModel { sqlTemplate.substr(sqlTemplate.rfind('(') + 1, sqlTemplate.rfind(')')
                                      - sqlTemplate.rfind('(') - 1) },
    m_values { m_valueModel } {
  validation::require(ps::contains(sqlTemplate, '?'), "The given sqlTemplate should contain '?'");
}

std::string SqlInsertor::build() {
  if (',' == m_query.back()) {
    m_query.pop_back();
    m_query.push_back(';');
  } else {
    m_query += fmt::format("({});", m_values);
  }

  return m_query;
}

#define REPLACE_IN_VALUES(PLACE_HOLDER) \
  do { \
    validation::require(std::string::npos != m_values.find(PLACE_HOLDER), "m_values should contain " #PLACE_HOLDER); \
    m_values.replace(m_values.find(PLACE_HOLDER), ps::length(PLACE_HOLDER), toString(value)); \
    return *this; \
  } while (false)

SqlInsertor& SqlInsertor::siteName(std::string_view value) { REPLACE_IN_VALUES("?siteName"); }
SqlInsertor& SqlInsertor::isHero(bool value) { REPLACE_IN_VALUES("?isHero"); }
SqlInsertor& SqlInsertor::comments(std::string_view value) { REPLACE_IN_VALUES("?comments"); }
SqlInsertor& SqlInsertor::playerSeat(Seat value) { REPLACE_IN_VALUES("?playerSeat"); }
SqlInsertor& SqlInsertor::playerName(std::string_view value) { REPLACE_IN_VALUES("?playerName"); }
SqlInsertor& SqlInsertor::isWinner(bool value) { REPLACE_IN_VALUES("?isWinner"); }
SqlInsertor& SqlInsertor::tableName(std::string_view value) { REPLACE_IN_VALUES("?tableName"); }
SqlInsertor& SqlInsertor::buttonSeat(Seat value) { REPLACE_IN_VALUES("?buttonSeat"); }
SqlInsertor& SqlInsertor::maxSeats(Seat value) { REPLACE_IN_VALUES("?maxSeats"); }
SqlInsertor& SqlInsertor::level(int value) { REPLACE_IN_VALUES("?level"); }
SqlInsertor& SqlInsertor::ante(int value) { REPLACE_IN_VALUES("?ante"); }
SqlInsertor& SqlInsertor::heroCard1(Card value) { REPLACE_IN_VALUES("?heroCard1"); }
SqlInsertor& SqlInsertor::heroCard2(Card value) { REPLACE_IN_VALUES("?heroCard2"); }
SqlInsertor& SqlInsertor::heroCard3(Card value) { REPLACE_IN_VALUES("?heroCard3"); }
SqlInsertor& SqlInsertor::heroCard4(Card value) { REPLACE_IN_VALUES("?heroCard4"); }
SqlInsertor& SqlInsertor::heroCard5(Card value) { REPLACE_IN_VALUES("?heroCard5"); }
SqlInsertor& SqlInsertor::boardCard1(Card value) { REPLACE_IN_VALUES("?boardCard1"); }
SqlInsertor& SqlInsertor::boardCard2(Card value) { REPLACE_IN_VALUES("?boardCard2"); }
SqlInsertor& SqlInsertor::boardCard3(Card value) { REPLACE_IN_VALUES("?boardCard3"); }
SqlInsertor& SqlInsertor::boardCard4(Card value) { REPLACE_IN_VALUES("?boardCard4"); }
SqlInsertor& SqlInsertor::boardCard5(Card value) { REPLACE_IN_VALUES("?boardCard5"); }
SqlInsertor& SqlInsertor::gameId(std::string_view value) { REPLACE_IN_VALUES("?gameId"); }
SqlInsertor& SqlInsertor::gameName(std::string_view value) { REPLACE_IN_VALUES("?gameName"); }
SqlInsertor& SqlInsertor::handId(std::string_view value) { REPLACE_IN_VALUES("?handId"); }
SqlInsertor& SqlInsertor::street(Street value) { REPLACE_IN_VALUES("?street"); }
SqlInsertor& SqlInsertor::actionType(ActionType value) { REPLACE_IN_VALUES("?actionType"); }
SqlInsertor& SqlInsertor::actionIndex(std::size_t value) { REPLACE_IN_VALUES("?actionIndex"); }
SqlInsertor& SqlInsertor::betAmount(double value) { REPLACE_IN_VALUES("?betAmount"); }
SqlInsertor& SqlInsertor::variant(Variant value) { REPLACE_IN_VALUES("?variant"); }
SqlInsertor& SqlInsertor::limitType(Limit value) { REPLACE_IN_VALUES("?limitType"); }
SqlInsertor& SqlInsertor::isRealMoney(bool value) { REPLACE_IN_VALUES("?isRealMoney"); }
SqlInsertor& SqlInsertor::nbMaxSeats(Seat value) { REPLACE_IN_VALUES("?nbMaxSeats"); }
SqlInsertor& SqlInsertor::startDate(std::string_view value) { REPLACE_IN_VALUES("?startDate"); }
SqlInsertor& SqlInsertor::tournamentId(std::string_view value) { REPLACE_IN_VALUES("?tournamentId"); }
SqlInsertor& SqlInsertor::buyIn(double value) { REPLACE_IN_VALUES("?buyIn"); }
SqlInsertor& SqlInsertor::cashGameId(std::string_view value) { REPLACE_IN_VALUES("?cashGameId"); }
SqlInsertor& SqlInsertor::smallBlind(double value) { REPLACE_IN_VALUES("?smallBlind"); }
SqlInsertor& SqlInsertor::bigBlind(double value) { REPLACE_IN_VALUES("?bigBlind"); }

#undef REPLACE_IN_VALUES

SqlInsertor& SqlInsertor::newInsert() {
  m_query += fmt::format("({}),", m_values);
  m_values = m_valueModel;
  return *this;
}

// externalized for unit testing
std::string formatSQL(std::string_view sql) {
  auto ret { ps::replaceAll(sql, "FROM", "\nFROM") };
  ret = ps::replaceAll(ret, "WHERE", "\nWHERE");
  ret = ps::replaceAll(ret, ",", ",\n\t");
  ret = ps::replaceAll(ret, "AND", "AND\n\t");
  ret = ps::replaceAll(ret, "OR", "OR\n\t");
  return ret;
}