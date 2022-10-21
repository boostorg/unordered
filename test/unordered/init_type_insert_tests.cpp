#if !defined(BOOST_UNORDERED_FOA_TESTS)
#error "This test is only for the FOA-style conatiners"
#endif

#include "../helpers/unordered.hpp"

#include "../helpers/test.hpp"

struct move_only
{
  int x_ = -1;

  move_only() = default;
  move_only(int x) : x_{x} {}
  move_only(move_only const&) = delete;
  move_only(move_only&&) = default;

  friend bool operator==(move_only const& lhs, move_only const& rhs)
  {
    return lhs.x_ == rhs.x_;
  }
};

namespace std{

template <> struct hash<move_only>
{
  std::size_t operator()(move_only const& mo) const noexcept
  {
    return std::hash<int>()(mo.x_);
  }
};

} // namespace std

struct raii_tracker
{
  static int move_constructs;
  static int copy_constructs;

  int x_ = -1;

  static void reset_counts()
  {
    move_constructs = 0;
    copy_constructs = 0;
  }

  raii_tracker() {}
  raii_tracker(int x) : x_{x} {}
  raii_tracker(raii_tracker const& rhs) : x_{rhs.x_} { ++copy_constructs; }

  raii_tracker(raii_tracker&& rhs) noexcept : x_{rhs.x_}
  {
    rhs.x_ = -1;

    ++move_constructs;
  }

  friend bool operator==(raii_tracker const& lhs, raii_tracker const& rhs)
  {
    return lhs.x_ == rhs.x_;
  }
};

namespace std{

template <> struct hash<raii_tracker>
{
  std::size_t operator()(raii_tracker const& rt) const noexcept
  {
    return std::hash<int>()(rt.x_);
  }
};

} // namespace std

int raii_tracker::move_constructs = 0;
int raii_tracker::copy_constructs = 0;

static void test_move_only()
{
  int const v = 128;

  boost::unordered_flat_map<move_only, int, std::hash<move_only> > map;

  using init_type = decltype(map)::init_type;
  static_assert(
    std::is_same<decltype(std::make_pair(move_only(1), v)), init_type>::value,
    "");

  map.insert(std::make_pair(move_only(1), v));
  map.insert({move_only(2), v});

  BOOST_TEST_EQ(map.size(), 2);

  map.rehash(1024);
  BOOST_TEST_GE(map.bucket_count(), 1024);
}

static void test_insert_tracking()
{
  raii_tracker::reset_counts();

  BOOST_TEST_EQ(raii_tracker::copy_constructs, 0);
  BOOST_TEST_EQ(raii_tracker::move_constructs, 0);

  boost::unordered_flat_map<raii_tracker, raii_tracker,
    std::hash<raii_tracker> >
    map;

  {
    std::pair<raii_tracker, raii_tracker> value{1, 2};

    map.insert(value);

    BOOST_TEST_EQ(raii_tracker::copy_constructs, 2);
    BOOST_TEST_EQ(raii_tracker::move_constructs, 0);
  }

  {
    std::pair<raii_tracker, raii_tracker> value{2, 3};

    map.insert(std::move(value));

    BOOST_TEST_EQ(raii_tracker::copy_constructs, 2);
    BOOST_TEST_EQ(raii_tracker::move_constructs, 2);
  }

  {
    std::pair<raii_tracker const, raii_tracker> value{3, 4};

    map.insert(value);

    BOOST_TEST_EQ(raii_tracker::copy_constructs, 4);
    BOOST_TEST_EQ(raii_tracker::move_constructs, 2);
  }

  {
    std::pair<raii_tracker const, raii_tracker> value{4, 5};

    map.insert(std::move(value));

    BOOST_TEST_EQ(raii_tracker::copy_constructs, 5);
    BOOST_TEST_EQ(raii_tracker::move_constructs, 3);
  }

  {
    map.insert(std::make_pair(5, 6));
    BOOST_TEST_EQ(raii_tracker::copy_constructs, 5);
    BOOST_TEST_EQ(raii_tracker::move_constructs, 5);
  }

  {
    map.insert({6, 7});
    BOOST_TEST_EQ(raii_tracker::copy_constructs, 5);
    BOOST_TEST_EQ(raii_tracker::move_constructs, 7);
  }

  BOOST_TEST_EQ(map.size(), 6);

  map.rehash(1024);
  BOOST_TEST_EQ(raii_tracker::copy_constructs, 5);
  BOOST_TEST_EQ(raii_tracker::move_constructs, 7 + 2 * map.size());
}

int main()
{
  test_move_only();
  test_insert_tracking();
  return boost::report_errors();
}
