#include <cds/container/impl/feldman_hashmap.h>
#include <atomic>

template <typename GC, typename K, class Traits>
class HashSet {
 public:
  HashSet();
  ~HashSet();

  size_t size() const;
  bool empty() const;


  bool insert(const K& key);

  bool find(const K& key);
  template <typename Func>
  bool find(const K& key, Func f);
  bool contains(const K& key) const;

  bool erase(const K& key);
  template <typename Func>
  bool erase(const K& key, Func f);

  typename Traits::list_type::guarded_ptr get(const K& key);

  size_t head_size() const;
  size_t array_node_size() const;
  void get_level_statistics(
      std::vector<cds::container::feldman_hashmap::level_statistics>& stat)
      const;

 private:
  cds::container::FeldmanHashMap<cds::gc::HP, K, typename Traits::list_type*,
                                 Traits> map;
  std::atomic_int size_;
};

template <typename GC, typename K, class Traits>
HashSet<GC, K, Traits>::HashSet()
    : size_(0) {}

template <typename GC, typename K, class Traits>
size_t HashSet<GC, K, Traits>::size() const {
  return size_;
}

template <typename GC, typename K, class Traits>
bool HashSet<GC, K, Traits>::empty() const {
  return size_ == 0;
}

template <typename GC, typename K, class Traits>
bool HashSet<GC, K, Traits>::insert(const K& key) {
  typename Traits::list_type* l = typename Traits::cxx_list_allocator().New();
  l->insert(key);
  bool InsertInList = false;
  if (!map.insert(key, l)) {
    typename cds::container::FeldmanHashMap<
        cds::gc::HP, K, typename Traits::list_type*>::guarded_ptr
        gp(map.get(key));
    if (gp && (*gp).second->insert(key)) {
      InsertInList = true;
    } else {
      typename Traits::cxx_list_allocator().Delete(l);
      return false;
    }
  }
  size_++;
  if (InsertInList) typename Traits::cxx_list_allocator().Delete(l);
  return true;
}

template <typename GC, typename K, class Traits>
bool HashSet<GC, K, Traits>::find(const K& key) {
  typename cds::container::FeldmanHashMap<
      cds::gc::HP, K, typename Traits::list_type*>::guarded_ptr
      gp(map.get(key));
  if (gp && (*gp).second->find(key, [](const K& node, const K& key) {}))
    return true;
  return false;
}

template <typename GC, typename K, class Traits>
bool HashSet<GC, K, Traits>::erase(const K& key) {
  typename cds::container::FeldmanHashMap<
      cds::gc::HP, K, typename Traits::list_type*>::guarded_ptr
      gp(map.get(key));
  if (gp && (*gp).second->erase(key)) {
    size_--;
    return true;
  }
  return false;
}

template <typename GC, typename K, class Traits>
HashSet<GC, K, Traits>::~HashSet() {
  for (auto i = map.begin(); i != map.end(); ++i) {
    typename Traits::cxx_list_allocator().Delete((*i).second);
  }
}

template <typename GC, typename K, class Traits>
bool HashSet<GC, K, Traits>::contains(const K& key) const {
  typename cds::container::FeldmanHashMap<cds::gc::HP, K,
                                          typename Traits::list_type*,
                                          Traits>::guarded_ptr gp(map.get(key));
  if (!gp)
    return false;
  else
    return (*gp).second->empty();
}

template <typename GC, typename K, class Traits>
typename Traits::list_type::guarded_ptr HashSet<GC, K, Traits>::get(
    const K& key) {
  typename cds::container::FeldmanHashMap<cds::gc::HP, K,
                                          typename Traits::list_type*,
                                          Traits>::guarded_ptr gp(map.get(key));
  if (gp) {
    return (*gp).second->get(key);
  } else {
    typename Traits::list_type::guarded_ptr gp(nullptr);
    return gp;
  }
}

template <typename GC, typename K, class Traits>
template <typename Func>
bool HashSet<GC, K, Traits>::erase(const K& key, Func f) {
  typename cds::container::FeldmanHashMap<
      cds::gc::HP, K, typename Traits::list_type*>::guarded_ptr
      gp(map.get(key));
  if (gp && (*gp).second->erase(key, f)) {
    size_--;
    return true;
  }
  return false;
}

template <typename GC, typename K, class Traits>
template <typename Func>
bool HashSet<GC, K, Traits>::find(const K& key, Func f) {
  typename cds::container::FeldmanHashMap<
      cds::gc::HP, K, typename Traits::list_type*>::guarded_ptr
      gp(map.get(key));
  if (gp) return (*gp).second->find(key, f);
  return false;
}

template <typename GC, typename K, class Traits>
size_t HashSet<GC, K, Traits>::head_size() const {
  return map.head_size();
}

template <typename GC, typename K, class Traits>
size_t HashSet<GC, K, Traits>::array_node_size() const {
  return map.array_node_size();
}

template <typename GC, typename K, class Traits>
void HashSet<GC, K, Traits>::get_level_statistics(
    std::vector<cds::container::feldman_hashmap::level_statistics>& stat)
    const {
  map.get_level_statistics(stat);
}
