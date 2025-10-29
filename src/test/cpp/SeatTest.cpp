#include "TestInfrastructure.hpp"
#include "entities/Seat.hpp"

/**
 * To be able to use the boost unit_test API with the Seat enum, we have to provide this.
 * Note: it has to be in the global scope, putting it in the anonymous namespace won't work
 */
static std::ostream& operator<<(std::ostream& os, Seat s) {
  return os << tableSeat::toString(s);
}

BOOST_AUTO_TEST_SUITE(SeatTest)

BOOST_AUTO_TEST_CASE(SeatTest_nextShouldLoopIfMax) {
  for (auto seat : { Seat::seatOne,Seat::seatTwo,Seat::seatThree,Seat::seatFour,Seat::seatFive,
                    Seat::seatSix,Seat::seatSeven,Seat::seatEight,Seat::seatNine,Seat::seatTen }) {
    BOOST_CHECK_EQUAL(Seat::seatOne, tableSeat::next(seat, seat));
  }
}

BOOST_AUTO_TEST_CASE(SeatTest_nextShouldAddOneIfNotMax) {
  for (auto seat : {Seat::seatTwo,Seat::seatThree,Seat::seatFour,Seat::seatFive,
                  Seat::seatSix,Seat::seatSeven,Seat::seatEight,Seat::seatNine,Seat::seatTen}) {
    BOOST_CHECK_EQUAL(Seat::seatTwo, tableSeat::next(Seat::seatOne, seat));
  }
  for (auto seat : {Seat::seatThree,Seat::seatFour,Seat::seatFive,
                  Seat::seatSix,Seat::seatSeven,Seat::seatEight,Seat::seatNine,Seat::seatTen}) {
    BOOST_CHECK_EQUAL(Seat::seatThree, tableSeat::next(Seat::seatTwo, seat));
  }
  for (auto seat : {Seat::seatFour,Seat::seatFive,
                  Seat::seatSix,Seat::seatSeven,Seat::seatEight,Seat::seatNine,Seat::seatTen}) {
    BOOST_CHECK_EQUAL(Seat::seatFour, tableSeat::next(Seat::seatThree, seat));
  }
  for (auto seat : {Seat::seatFive,
                  Seat::seatSix,Seat::seatSeven,Seat::seatEight,Seat::seatNine,Seat::seatTen}) {
    BOOST_CHECK_EQUAL(Seat::seatFive, tableSeat::next(Seat::seatFour, seat));
  }
  for (auto seat : {Seat::seatSix,Seat::seatSeven,Seat::seatEight,Seat::seatNine,Seat::seatTen}) {
    BOOST_CHECK_EQUAL(Seat::seatSix, tableSeat::next(Seat::seatFive, seat));
  }
  for (auto seat : {Seat::seatSeven,Seat::seatEight,Seat::seatNine,Seat::seatTen}) {
    BOOST_CHECK_EQUAL(Seat::seatSeven, tableSeat::next(Seat::seatSix, seat));
  }
  for (auto seat : {Seat::seatEight,Seat::seatNine,Seat::seatTen}) {
    BOOST_CHECK_EQUAL(Seat::seatEight, tableSeat::next(Seat::seatSeven, seat));
  }
  for (auto seat : {Seat::seatNine,Seat::seatTen}) {
    BOOST_CHECK_EQUAL(Seat::seatNine, tableSeat::next(Seat::seatEight, seat));
  }
}

BOOST_AUTO_TEST_SUITE_END()