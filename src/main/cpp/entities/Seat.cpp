#include "constants/TableConstants.hpp"
#include "entities/Seat.hpp"
#include "language/EnumMapper.hpp"
#include "language/Validator.hpp"

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
