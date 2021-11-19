// Copyright 2021 Christian Mazakas.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off
#include "../helpers/prefix.hpp"
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "../helpers/postfix.hpp"
// clang-format on

#include "../helpers/test.hpp"

#include <boost/container_hash/hash.hpp>

struct key {
    int x_;
    static int count_;

    explicit key(int x) : x_(x) {
        ++count_;
    }
};

int key::count_;

UNORDERED_AUTO_TEST(transparent_count) {
    key::count_ = 0;

    struct hasher {
        std::size_t operator()(key const& k) const {
            return boost::hash<int>()(k.x_);
        }

        std::size_t operator()(int const k) const {
            return boost::hash<int>()(k);
        }
    };

    struct key_equal {
        bool operator()(key const& k1, key const& k2) const {
            return k1.x_ == k2.x_;
        }

        bool operator()(key const& k1, int const x) const {
            return k1.x_ == x;
        }

        bool operator()(int const x, key const& k1 ) const {
            return k1.x_ == x;
        }
    };

    boost::unordered_map<key, int, hasher, key_equal> map;
    map.insert({key(0), 1337});
    BOOST_TEST(key::count_ == 1);

    std::size_t const count = map.count(0);

    BOOST_TEST(count == 1);
    BOOST_TEST(key::count_ == 1);
}

RUN_TESTS()
