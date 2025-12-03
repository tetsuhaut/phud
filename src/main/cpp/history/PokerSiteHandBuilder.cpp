#include "filesystem/TextFile.hpp"
#include "history/PokerSiteHandBuilder.hpp"
#include "strings/StringUtils.hpp"

namespace ps = phud::strings;

static constexpr auto SEAT_LENGTH {ps::length("Seat ")};

[[nodiscard]] /*static*/ std::array<std::string, TableConstants::MAX_SEATS>
parseSeats(TextFile& tf, PlayerCache& /*cache*/) {
  std::array<std::string, TableConstants::MAX_SEATS> ret;

  while (tf.startsWith("Seat ")) {
    const auto line = tf.getLine();
    const auto pos = line.find(": ", SEAT_LENGTH);
    const auto seat = ps::toSizeT(line.substr(SEAT_LENGTH, pos - SEAT_LENGTH)) - 1;
    const auto player = line.substr(pos + 2, line.rfind(" (") - pos - 2);
    ret.at(seat) = player;
    tf.next();
  }

  while (!tf.contains(" posts ")) {
    tf.next();
  } // disreguard the blinds (lol)

  return ret;
}

// splits the given str into tokens, separated by delimiters.
// We assume there are always 5 tokens
[[nodiscard]] /*static*/ std::array<std::string_view, 5> split(std::string_view str,
                                                               std::string_view delimiter) {
  std::array<std::string_view, 5> ret = {"none", "none", "none", "none", "none"};
  std::size_t offset = 0, delimiterPosition = 0, arrayIndex = 0;

  while (std::string_view::npos != (delimiterPosition = str.find(delimiter, offset))) {
    ret.at(arrayIndex) = str.substr(offset, delimiterPosition - offset);
    offset = delimiterPosition + delimiter.size();
    arrayIndex++;
  }

  if (offset != str.size() - 1) {
    ret.at(arrayIndex) = str.substr(offset);
  }

  return ret;
}
