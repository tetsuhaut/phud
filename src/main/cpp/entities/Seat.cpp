#include "constants/TableConstants.hpp"
#include "entities/Seat.hpp"
#include "language/EnumMapper.hpp"
#include "language/PhudException.hpp" // PhudException
#include "language/Validator.hpp"
#include <cassert> // assert

static constexpr auto SEAT_MAPPER = makeEnumMapper<Seat>(
  std::pair{Seat::seatOne, "1"}, std::pair{Seat::seatTwo, "2"},
  std::pair{Seat::seatThree, "3"}, std::pair{Seat::seatFour, "4"},
  std::pair{Seat::seatFive, "5"}, std::pair{Seat::seatSix, "6"},
  std::pair{Seat::seatSeven, "7"}, std::pair{Seat::seatEight, "8"},
  std::pair{Seat::seatNine, "9"}, std::pair{Seat::seatTen, "10"}
);

/*[[nodiscard]]*/ Seat tableSeat::fromString(std::string_view seatStr) {
  validation::require(SEAT_MAPPER.isValid(seatStr), "bad seat string");
  return SEAT_MAPPER.fromString(seatStr);
}

/*[[nodiscard]]*/ Seat tableSeat::fromArrayIndex(std::size_t i) {
  validation::require(TableConstants::MAX_SEATS > i, "Can't find a seat for that value");
  return static_cast<Seat>(i);
}

/*[[nodiscard]]*/ std::string_view tableSeat::toString(Seat seat) {
  return SEAT_MAPPER.toString(seat);
}

/*[[nodiscard]]*/ Seat tableSeat::fromInt(int i) {
  validation::require(0 < i and TableConstants::MAX_SEAT_NUMBER >= i, "Can't find a seat for that value");
  return static_cast<Seat>(i - 1);
}

/*[[nodiscard]]*/ int tableSeat::toInt(Seat seat) {
  switch (seat) {
  case Seat::seatOne: return 1;
  case Seat::seatTwo: return 2;
  case Seat::seatThree: return 3;
  case Seat::seatFour: return 4;
  case Seat::seatFive: return 5;
  case Seat::seatSix: return 6;
  case Seat::seatSeven: return 7;
  case Seat::seatEight: return 8;
  case Seat::seatNine: return 9;
  case Seat::seatTen: return 10;
  case Seat::seatUnknown: return 11;
  default: throw PhudException("Unknown seat value");
  }
}

/*[[nodiscard]]*/ std::size_t tableSeat::toArrayIndex(Seat seat) {
  switch (seat) {
  case Seat::seatOne: return 0;
  case Seat::seatTwo: return 1;
  case Seat::seatThree: return 2;
  case Seat::seatFour: return 3;
  case Seat::seatFive: return 4;
  case Seat::seatSix: return 5;
  case Seat::seatSeven: return 6;
  case Seat::seatEight: return 7;
  case Seat::seatNine: return 8;
  case Seat::seatTen: return 9;
  case Seat::seatUnknown: return 10;
  default: throw PhudException("Unknown seat value");
  }
}

/*[[nodiscard]]*/ Seat tableSeat::next(Seat current, Seat max) {
  assert(current != Seat::seatUnknown);
  assert(max != Seat::seatUnknown);
  assert(tableSeat::toInt(current) <= tableSeat::toInt(max));
  return (current == max) ? Seat::seatOne : tableSeat::fromInt(tableSeat::toInt(current) + 1);
}