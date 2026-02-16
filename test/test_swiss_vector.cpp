#include <jlib/generic_ostream.h>
#include <jlib/swiss_vector.h>
#include <jlib/test_framework.h>

using namespace std::literals;


TEST("swiss_vector constructors/CTAD") {
    auto v1 = swiss_vector { 1, 2, 3 };
    ASSERT(v1.collect() == std::vector { 1, 2, 3 });
    auto v4 = std::vector { v1.begin(), v1.end() };
    // ASSERT(v4 == std::vector { 1, 2, 3 });

    auto v2 = swiss_vector { 1 };
    ASSERT(v2.collect() == std::vector { 1 });

    auto v3 = swiss_vector<std::string>{};
    v3.emplace_back("hello");
    v3.emplace_back("world");
    ASSERT(v3.collect() == std::vector { "hello"s, "world"s });
}

TEST("swiss vector add-remove-add-remove-add") {
    auto v1 = swiss_vector<int> {};
    v1.reserve(10);

    v1.emplace_back(1);
    v1.emplace_back(2);
    v1.remove(1);
    v1.emplace_back(3);
    v1.remove(1);
    v1.emplace_back(4);

    ASSERT(v1.collect() == std::vector { 1, 4 });
};

TEST("swiss_vector emplace") {
    auto vec = swiss_vector { 1, 2, 3, 4, 5 };
    vec.emplace_back(1);
    vec.emplace_back(2);
    vec.emplace_back(3);

    ASSERT(vec.collect() == std::vector { 1, 2, 3, 4, 5, 1, 2, 3 });
}

TEST("swiss vector with some removes") {
    auto vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    vec.remove(3);
    vec.remove(6);
    vec.emplace_back(32);
    vec.remove(7);
    vec.emplace_back(64);

    ASSERT(vec.collect() == std::vector { 1, 2, 3, 5, 6, 32, 64, 9, 10 }); //
}

TEST("remove from middle, push two, size didn't change") {
    auto vec = swiss_vector { 1, 2, 3, 4, 5 };
    vec.remove(2);
    vec.remove(3);
    vec.emplace_back(6);
    vec.emplace_back(7);
    ASSERT(vec.collect() == std::vector { 1, 2, 7, 6, 5 });
}

// // subclass that makes the free slots publicly accessible
// template<typename T> class SV2 : public swiss_vector<T> {
// public:
//     using swiss_vector<T>::swiss_vector;

//     std::vector<size_t>& get_free() {
//         return this->free_slots;
//     }
// };
// template<typename F, typename...A> SV2(F,A...) -> SV2<F>;


// TEST("remove from end, free slots not used") {
//     auto vec = SV2 { 1, 2, 3, 4, 5, 6 };
//     vec.remove(5);
//     vec.remove(4);
//     ASSERT(vec.get_free().size() == 0);
// }

// TEST("swiss vector begin/end") {
//     auto vec = swiss_vector { 1, 2, 3, 4, 5, 6 };

//     ASSERT(vec.begin().index == 0);
//     ASSERT(vec.end().index == 6);

//     vec.remove(5);
//     ASSERT(vec.begin().index == 0);
//     ASSERT(vec.end().index == 5);

//     vec.remove(0);
//     ASSERT(vec.begin().index == 1);
//     ASSERT(vec.end().index == 5);

//     vec.remove(4);
//     ASSERT(vec.begin().index == 1);
//     ASSERT(vec.end().index == 4);

//     vec.remove(2);
//     ASSERT(vec.begin().index == 1);
//     ASSERT(vec.end().index == 4);

//     vec.remove(1);
//     auto b = vec.begin().index;
//     ASSERT(b == 3);
//     ASSERT(vec.end().index == 4);
// }

// TEST("swiss vector remove_if") {
//     auto vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7 };
//     vec.remove_if([](auto i) { return i & 1; });
//     ASSERT(vec.collect() == std::vector { 2, 4, 6 });
// }

// TEST("swiss_vector remove all") {
//     auto vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7 };
//     vec.remove_if([](int) { return true; });
//     ASSERT(vec.collect() == std::vector<int>{});
// }

// TEST("swiss_vector remove none") {
//     auto vec = swiss_vector { 1, 2, 3, 4, 5 };
//     vec.remove_if([] (int) { return false; });
//     ASSERT(vec.collect() == std::vector { 1, 2, 3, 4, 5 });
// }

// TEST("swiss_vector iteration") {
//     auto vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

//     // make some gaps
//     vec.remove(1);
//     vec.remove(3);
//     vec.remove(5);

//     auto total = 0;
//     for (auto i = vec.begin(); i != vec.end(); ++i) {
//         total += *i;
//     }
//     ASSERT(total == 43);

// }
// TEST("swiss vector range iteration") {
//     auto vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
//     vec.remove(1);
//     vec.remove(3);
//     vec.remove(5);

//     auto total = 0;
//     for (auto i: vec) {
//         total += i;
//     }
//     ASSERT(total == 43);
// }


// TEST("swiss vector fuzz") {
//     auto vec = std::vector { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
//     auto v = swiss_vector { vec.begin(), vec.end() };

//     ASSERT(v.collect() == vec);

//     srand(12345);
//     for (auto i = 0; i < 1000; i++) {
//         const auto op = rand() % 10;

//         switch (op) {
//         case 0: // add
//         case 1: // remove random
//         case 2: // reassign random (by iterating a random number of times)
//         case 3: // iterate and sum
//             break;
//         }
//     }

//     ASSERT(v.collect() == vec);
// }
