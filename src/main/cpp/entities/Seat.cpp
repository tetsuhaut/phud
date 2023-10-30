#include "entities/Seat.hpp"
#include <frozen/string.h>
#include <frozen/unordered_map.h>

// Note : must use frozen::string when it is a map key.
// frozen::string can be created from std::string_view.

static constexpr auto ENUM_TO_STRING {
  frozen::make_unordered_map<Seat, std::string_view>({
    {Seat::seatOne, "1"}, {Seat::seatTwo, "2"}, {Seat::seatThree, "3"}, {Seat::seatFour, "4"}, {Seat::seatFive, "5"},
    {Seat::seatSix, "6"}, {Seat::seatSeven, "7"}, {Seat::seatEight, "8"}, {Seat::seatNine, "9"}, {Seat::seatTen, "10"}
  })
};

static constexpr auto STRING_TO_ENUM {
  frozen::make_unordered_map<frozen::string, Seat>({
    {"1", Seat::seatOne}, {"2", Seat::seatTwo}, {"3", Seat::seatThree}, {"4", Seat::seatFour}, {"5", Seat::seatFive},
    {"6", Seat::seatSix}, {"7", Seat::seatSeven}, {"8", Seat::seatEight}, {"9", Seat::seatNine}, {"10", Seat::seatTen}
  })
};

static constexpr auto ENUM_TO_INT {
  frozen::make_unordered_map<Seat, int>({
    {Seat::seatOne, 1}, {Seat::seatTwo, 2}, {Seat::seatThree, 3}, {Seat::seatFour, 4}, {Seat::seatFive, 5},
    {Seat::seatSix, 6}, {Seat::seatSeven, 7}, {Seat::seatEight, 8}, {Seat::seatNine, 9}, {Seat::seatTen, 10}
  })
};

static constexpr auto ENUM_TO_SIZET {
  frozen::make_unordered_map<Seat, std::size_t>({
    {Seat::seatOne, 1}, {Seat::seatTwo, 2}, {Seat::seatThree, 3}, {Seat::seatFour, 4}, {Seat::seatFive, 5},
    {Seat::seatSix, 6}, {Seat::seatSeven, 7}, {Seat::seatEight, 8}, {Seat::seatNine, 9}, {Seat::seatTen, 10}
  })
};
static constexpr auto INT_TO_ENUM {
  frozen::make_unordered_map<int, Seat>({
    {1, Seat::seatOne}, {2, Seat::seatTwo}, {3, Seat::seatThree}, {4, Seat::seatFour}, {5, Seat::seatFive},
    {6, Seat::seatSix}, {7, Seat::seatSeven}, {8, Seat::seatEight}, {9, Seat::seatNine}, {10, Seat::seatTen}
  })
};

static constexpr auto SIZET_TO_ENUM {
  frozen::make_unordered_map<std::size_t, Seat>({
    {1, Seat::seatOne}, {2, Seat::seatTwo}, {3, Seat::seatThree}, {4, Seat::seatFour}, {5, Seat::seatFive},
    {6, Seat::seatSix}, {7, Seat::seatSeven}, {8, Seat::seatEight}, {9, Seat::seatNine}, {10, Seat::seatTen}
  })
};

/*[[nodiscard]]*/ Seat tableSeat::fromString(std::string_view seatStr) {
  phudAssert("1" == seatStr or "2" == seatStr or "3" == seatStr or "4" == seatStr or "5" == seatStr
             or "6" == seatStr or "7" == seatStr or "8" == seatStr or "9" == seatStr or "10" == seatStr,
             "bad seat string");
  return STRING_TO_ENUM.find(seatStr)->second;
}

/*[[nodiscard]]*/ Seat tableSeat::fromArrayIndex(std::size_t i) {
  phudAssert(10 > i, "Can't find a seat for that value");
  return SIZET_TO_ENUM.find(i + 1)->second;
}

/*[[nodiscard]]*/ std::size_t tableSeat::toArrayIndex(Seat seat) {
  return ENUM_TO_SIZET.find(seat)->second - 1;
}

/*[[nodiscard]]*/ std::string_view tableSeat::toString(Seat seat) {
  return ENUM_TO_STRING.find(seat)->second;
}

/*[[nodiscard]]*/ Seat tableSeat::fromInt(int i) {
  phudAssert(0 < i and 11 > i, "Can't find a seat for that value");
  return INT_TO_ENUM.find(i)->second;
}

/*[[nodiscard]]*/ int tableSeat::toInt(Seat seat) {
  return ENUM_TO_INT.find(seat)->second;
}
