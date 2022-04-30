#pragma once

#include <folly/FBString.h>
#include <folly/small_vector.h>
#include <folly/sorted_vector_types.h>

namespace toc {
    using string = folly::fbstring;

    // template <typename Key, typename Mapped, typename Hasher, typename KeyEqual, typename Alloc>
    // using map = folly::F14FastMap<Key, Mapped, Hasher, KeyEqual, Alloc>;
    template<typename K, typename V> using flat_map = folly::sorted_vector_map<K, V>;

    template<typename K> using flat_set = folly::sorted_vector_set<K>;
    using folly::small_vector;
} // namespace toc
