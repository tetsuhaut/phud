#include "filesystem/TextFile.hpp"
#include "history/PokerSiteHandBuilder.hpp"
#include "threads/PlayerCache.hpp"

[[nodiscard]] /*static*/ std::array<String, 10> parseSeats(TextFile& tf, PlayerCache& /*cache*/) {
  std::array<String, 10> ret;

  while (tf.startsWith("Seat ")) {
    const auto& line { tf.getLine() };
    const auto pos { line.find(": ", SEAT_LENGTH) };
    const auto seat { phud::strings::toSizeT(line.substr(SEAT_LENGTH, pos - SEAT_LENGTH)) - 1 };
    const auto& player { line.substr(pos + 2, line.rfind(" (") - pos - 2) };
    ret.at(seat) = player;
    tf.next();
  }

  while (!tf.contains(" posts ")) { tf.next(); } // disreguard the blinds (lol)

  return ret;
}

// splits the given str into tokens, separated by delimiters.
// We assume there are always 5 tokens
[[nodiscard]] /*static*/ std::array<StringView, 5> split(StringView str, StringView delimiter) {
  std::array<StringView, 5> ret { "none", "none", "none", "none", "none" };
  std::size_t offset = 0, delimiterPosition = 0, arrayIndex = 0;

  while (StringView::npos != (delimiterPosition = str.find(delimiter, offset))) {
    ret.at(arrayIndex) = str.substr(offset, delimiterPosition - offset);
    offset = delimiterPosition + delimiter.size();
    arrayIndex++;
  }

  if (offset != str.size() - 1) {
    ret.at(arrayIndex) = str.substr(offset);
  }

  return ret;
}