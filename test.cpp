#include <gtest/gtest.h>
#include <cds/init.h>
#include <cds/gc/hp.h>

#include <cds/container/iterable_list_hp.h>
#include <cds/container/impl/feldman_hashmap.h>
#include <cds/container/impl/feldman_hashset.h>
#include <cds/container/lazy_list_hp.h>
#include <cds/container/michael_list_hp.h>
#include <cds/threading/model.h>

#include "./HashSet.h"


namespace cc = cds::container;

struct int_with_bad_hash {
 public:
  int_with_bad_hash(int x = 0) : val(x) {}

  bool operator<(const int_with_bad_hash& other) const { return val < other.val; }
  operator int() const { return val; }

  int val;
};

struct get_hash {
  int operator()(const int_with_bad_hash& other) { return other.val % 2; }
};

template <class List>
struct my_map_traits : public cc::feldman_hashmap::traits {
  typedef List list_type;
  typedef CDS_DEFAULT_ALLOCATOR list_allocator;
  typedef cds::details::Allocator<list_type, list_allocator> cxx_list_allocator;
};

template <class List>
struct my_map_traits_int_with_bad_hash : public my_map_traits<List> {
  typedef get_hash hash;
};

struct func {
    int* operator()( const int_with_bad_hash& item, const int_with_bad_hash& key ){
      static int sum = 0;
      sum += item.val;
      return &sum;
    }
    int* operator()( const int_with_bad_hash& item ){
      static int sum = 0;
      sum += item.val;
      return &sum;
    }
};

typedef cds::gc::HP gc_type;

class HashSet_HP : public testing::Test {
 protected:
  void SetUp() {
    typedef cc::FeldmanHashMap<gc_type, int, int> map_type;

    // +1 - for guarded_ptr
    cds::gc::hp::GarbageCollector::Construct(map_type::c_nHazardPtrCount + 20,
                                             1, 16);
    cds::threading::Manager::attachThread();
  }

  void TearDown() {
    cds::threading::Manager::detachThread();
    cds::gc::hp::GarbageCollector::Destruct(true);
  }
};

TEST_F(HashSet_HP, simple_test) {
  HashSet<cds::gc::HP, int, my_map_traits<cc::LazyList<cds::gc::HP, int> > >
      map_with_lazylist;
  HashSet<cds::gc::HP, int, my_map_traits<cc::MichaelList<cds::gc::HP, int> > >
      map_with_michaellist;
  HashSet<cds::gc::HP, int, my_map_traits<cc::IterableList<cds::gc::HP, int> > >
      map_with_iterablelist;
  ASSERT_EQ(true, map_with_lazylist.insert(1) ==
                      map_with_michaellist.insert(1) ==
                      map_with_iterablelist.insert(1) == true);
  ASSERT_EQ(true, map_with_lazylist.insert(2) ==
                      map_with_michaellist.insert(2) ==
                      map_with_iterablelist.insert(2) == true);
  ASSERT_EQ(true, map_with_lazylist.insert(3) ==
                      map_with_michaellist.insert(3) ==
                      map_with_iterablelist.insert(3) == true);
}

TEST_F(HashSet_HP, insert_and_find) {
  HashSet<cds::gc::HP, int, my_map_traits<cc::LazyList<cds::gc::HP, int> > >
      map_with_lazylist;
  HashSet<cds::gc::HP, int, my_map_traits<cc::MichaelList<cds::gc::HP, int> > >
      map_with_michaellist;
  HashSet<cds::gc::HP, int, my_map_traits<cc::IterableList<cds::gc::HP, int> > >
      map_with_iterablelist;

  map_with_lazylist.insert(3);
  map_with_michaellist.insert(5);
  map_with_iterablelist.insert(10);

  ASSERT_EQ(true, map_with_lazylist.find(3));
  ASSERT_EQ(true, map_with_michaellist.find(5));
  ASSERT_EQ(true, map_with_iterablelist.find(10));

  ASSERT_EQ(false, map_with_lazylist.find(4));
  ASSERT_EQ(false, map_with_michaellist.find(2));
  ASSERT_EQ(false, map_with_iterablelist.find(6));

  ASSERT_EQ(false, map_with_lazylist.find(8));
  ASSERT_EQ(false, map_with_michaellist.find(9));
  ASSERT_EQ(false, map_with_iterablelist.find(7));
}

TEST_F(HashSet_HP, insert_and_get) {
  HashSet<cds::gc::HP, int, my_map_traits<cc::LazyList<cds::gc::HP, int> > >
      map_with_lazylist;
  HashSet<cds::gc::HP, int, my_map_traits<cc::MichaelList<cds::gc::HP, int> > >
      map_with_michaellist;
  HashSet<cds::gc::HP, int, my_map_traits<cc::IterableList<cds::gc::HP, int> > >
      map_with_iterablelist;

  map_with_lazylist.insert(3);
  map_with_michaellist.insert(5);
  map_with_iterablelist.insert(10);

  auto gp_lazylist = map_with_lazylist.get(3);
  auto gp_michaellist = map_with_michaellist.get(5);
  auto gp_iterablelist = map_with_iterablelist.get(10);

  ASSERT_EQ(3, *gp_lazylist);
  ASSERT_EQ(5, *gp_michaellist);
  ASSERT_EQ(10, *gp_iterablelist);

  auto gp_iterablelist_null = map_with_iterablelist.get(20);
  ASSERT_EQ(false , static_cast<bool>(gp_iterablelist_null));

}

TEST_F(HashSet_HP, insert_with_same_hash) {
  HashSet<cds::gc::HP, int_with_bad_hash,
          my_map_traits_int_with_bad_hash<cc::LazyList<cds::gc::HP, int_with_bad_hash> > >
      map_with_lazylist;

  map_with_lazylist.insert(int_with_bad_hash(5));
  map_with_lazylist.insert(int_with_bad_hash(6));
  map_with_lazylist.insert(int_with_bad_hash(11));
  map_with_lazylist.insert(int_with_bad_hash(13));
  map_with_lazylist.insert(int_with_bad_hash(8));
  map_with_lazylist.insert(int_with_bad_hash(14));

  ASSERT_EQ(true, map_with_lazylist.find(5));
  ASSERT_EQ(true, map_with_lazylist.find(6));
  ASSERT_EQ(true, map_with_lazylist.find(11));
  ASSERT_EQ(true, map_with_lazylist.find(13));
  ASSERT_EQ(true, map_with_lazylist.find(8));
  ASSERT_EQ(true, map_with_lazylist.find(14));

  ASSERT_EQ(false, map_with_lazylist.find(1));
  ASSERT_EQ(false, map_with_lazylist.find(2));
  ASSERT_EQ(false, map_with_lazylist.find(3));

  ASSERT_EQ(false, map_with_lazylist.insert(5));
  ASSERT_EQ(false, map_with_lazylist.insert(6));
  ASSERT_EQ(false, map_with_lazylist.insert(11));

  ASSERT_EQ(6, map_with_lazylist.size());
}

TEST_F(HashSet_HP, insert_and_erase) {
  HashSet<cds::gc::HP, int_with_bad_hash,
          my_map_traits_int_with_bad_hash<cc::IterableList<cds::gc::HP, int_with_bad_hash> > >
      map_with_iterablelist;
  map_with_iterablelist.insert(1);
  map_with_iterablelist.insert(5);
  map_with_iterablelist.insert(7);
  map_with_iterablelist.insert(2);

  ASSERT_EQ(4, map_with_iterablelist.size());

  ASSERT_EQ(true, map_with_iterablelist.erase(1));
  ASSERT_EQ(true, map_with_iterablelist.erase(5));
  ASSERT_EQ(true, map_with_iterablelist.erase(2));

  ASSERT_EQ(false, map_with_iterablelist.erase(2));

  ASSERT_EQ(1, map_with_iterablelist.size());
}


TEST_F(HashSet_HP, test_with_function) {
  HashSet<cds::gc::HP, int_with_bad_hash,
          my_map_traits_int_with_bad_hash<cc::MichaelList<cds::gc::HP, int_with_bad_hash> > >
      map_with_michaelist;
  map_with_michaelist.insert(6);
  map_with_michaelist.insert(11);
  map_with_michaelist.insert(7);
  map_with_michaelist.insert(2);

  func int_sum;

  int* result = int_sum(0,0);

  ASSERT_EQ(true, map_with_michaelist.find(7, int_sum));
  ASSERT_EQ(true, map_with_michaelist.find(6, int_sum));
  ASSERT_EQ(true, map_with_michaelist.find(11, int_sum));
  ASSERT_EQ(true, map_with_michaelist.find(2, int_sum));

  ASSERT_EQ(26, *result);

  result = int_sum(0);

  ASSERT_EQ(true, map_with_michaelist.erase(7, int_sum));
  ASSERT_EQ(true, map_with_michaelist.erase(6, int_sum));
  ASSERT_EQ(true, map_with_michaelist.erase(11, int_sum));
  ASSERT_EQ(true, map_with_michaelist.erase(2, int_sum));

  ASSERT_EQ(26, *result);

  ASSERT_EQ(true, map_with_michaelist.empty());
}

int main(int argc, char** argv) {
  int result;
  cds::Initialize();
  {
    ::testing::InitGoogleTest(&argc, argv);
    result = RUN_ALL_TESTS();
  }
  cds::Terminate();
  return result;
}
