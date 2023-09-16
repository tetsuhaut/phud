#include "TestInfrastructure.hpp"
#include "containers/Vector.hpp"
#include "system/memory.hpp"
#include <boost/range.hpp>

namespace {
class [[nodiscard]] Owner final {
public:
  Vector<uptr<int>> v {};

  boost::iterator_range<Vector<uptr<int>>::const_iterator> getIntView() {
    return boost::make_iterator_range(v.begin(), v.end());
  }
}; // class Owner
}; // namespace

BOOST_AUTO_TEST_SUITE(RangeTest)

BOOST_AUTO_TEST_CASE(RangeTest_creatingRangeShouldSucceed) {
  const Vector<int> myInts {0, 1, 2};
  boost::iterator_range r { boost::make_iterator_range(myInts.begin(), myInts.end()) };
}

BOOST_AUTO_TEST_CASE(RangeTest_aViewShouldHaveTheSameContentAsRealContainer) {
  Owner o;
  o.v.push_back(mkUptr<int>(0));
  o.v.push_back(mkUptr<int>(666));
  const auto& view { o.getIntView() };
  BOOST_REQUIRE(view.size() == o.v.size());
}

BOOST_AUTO_TEST_SUITE_END()