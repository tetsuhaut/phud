#include "TestInfrastructure.hpp" // std::string_view
#include "db/SqlInsertor.hpp"
#include "entities/Seat.hpp"

BOOST_AUTO_TEST_SUITE(SqlInsertorTest)

BOOST_AUTO_TEST_CASE(SqlInsertorTest_buildNonQuotedSimpleInsertShouldSucceed) {
  SqlInsertor query {"INSERT INTO myTable (col1,col2,col3) VALUES (?ante,?amount,?buttonSeat);"};
  const auto sql = query.ante(1).amount(2).buttonSeat(Seat::seatEight).build();
  BOOST_TEST("INSERT INTO myTable (col1,col2,col3) VALUES (1,2.000000,8);" == sql);
}

BOOST_AUTO_TEST_CASE(SqlInsertorTest_buildNonQuotedMultipleInsertShouldSucceed) {
  SqlInsertor query("INSERT INTO myTable (col1,col2,col3) VALUES (?ante,?amount,?buttonSeat);");
  const auto& sql {query.ante(1)
                       .amount(2)
                       .buttonSeat(Seat::seatThree)
                       .newInsert()
                       .ante(4)
                       .amount(5)
                       .buttonSeat(Seat::seatSix)
                       .newInsert()
                       .ante(7)
                       .amount(888)
                       .buttonSeat(Seat::seatNine)
                       .newInsert()
                       .build()};
  BOOST_TEST("INSERT INTO myTable (col1,col2,col3) VALUES (1,2.000000,3),(4,5.000000,6),"
             "(7,888.000000,9);" == sql);
}

BOOST_AUTO_TEST_CASE(SqlInsertorTest_buildQuotedSimpleInsertShouldSucceed) {
  SqlInsertor query("INSERT INTO myTable (col1,col2,col3) VALUES (?ante,'?comments',?buttonSeat);");
  const auto sql = query.ante(1).comments("someValue").buttonSeat(Seat::seatThree).build();
  BOOST_TEST("INSERT INTO myTable (col1,col2,col3) VALUES (1,'someValue',3);" == sql);
}

BOOST_AUTO_TEST_CASE(SqlInsertorTest_buildQuotedMultipleInsertShouldSucceed) {
  SqlInsertor query("INSERT INTO myTable (col1,col2,col3) VALUES (?ante,'?comments',?buttonSeat);");
  query.ante(1).comments("2").buttonSeat(Seat::seatThree).newInsert();
  query.ante(1).comments("someValue").buttonSeat(Seat::seatThree).newInsert();
  query.ante(1).comments("someOtherValue").buttonSeat(Seat::seatThree).newInsert();
  BOOST_TEST("INSERT INTO myTable (col1,col2,col3) VALUES (1,'2',3),(1,'someValue',3),"
             "(1,'someOtherValue',3);" == query.build());
}

BOOST_AUTO_TEST_CASE(SqlInsertorTest_buildNonQuotedSimpleInsertSqliteStyleShouldSucceed) {
  SqlInsertor query {
      "INSERT INTO myTable (col1,col2,col3,col4) VALUES (?ante,?buttonSeat,?amount,?level);"};
  const auto sql = query.ante(1).buttonSeat(Seat::seatTwo).amount(666).level(7).build();
  BOOST_TEST("INSERT INTO myTable (col1,col2,col3,col4) VALUES (1,2,666.000000,7);" == sql);
}

BOOST_AUTO_TEST_CASE(SqlInsertorTest_buildNonQuotedMultipleInsertSqliteStyleShouldSucceed) {
  SqlInsertor query("INSERT INTO myTable (col1,col2,col3) VALUES (?ante,?buttonSeat,?amount);");
  const auto& sql {query.ante(1)
                       .buttonSeat(Seat::seatTwo)
                       .amount(3)
                       .newInsert()
                       .ante(4)
                       .buttonSeat(Seat::seatFive)
                       .amount(6)
                       .newInsert()
                       .ante(7)
                       .buttonSeat(Seat::seatEight)
                       .amount(9)
                       .newInsert()
                       .build()};
  BOOST_TEST("INSERT INTO myTable (col1,col2,col3) VALUES (1,2,3.000000),"
             "(4,5,6.000000),(7,8,9.000000);" == sql);
}

BOOST_AUTO_TEST_SUITE_END()
