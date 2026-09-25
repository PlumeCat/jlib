
// TEST("remove from middle, push two, size didn't change") {
//     auto vec = swiss_vector { 1, 2, 3, 4, 5 };
//     vec.remove(2);
//     vec.remove(3);
//     vec.emplace_back(6);
//     vec.emplace_back(7);
//     ASSERT(vec.collect() == std::vector { 1, 2, 7, 6, 5 });
// }

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
//     // remove odd
//     auto vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7 };
//     vec.remove_if([](auto i) { return i & 1; });
//     ASSERT(vec.collect() == std::vector { 2, 4, 6 });

//     // remove even
//     vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7 };
//     vec.remove_if([](auto i) { return (i+1) & 1; });
//     ASSERT(vec.collect() == std::vector { 1, 3, 5, 7 });

//     // remove from start
//     vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7 };
//     vec.remove_if([](auto i) { return i < 3; });
//     ASSERT(vec.collect() == std::vector { 3, 4, 5, 6, 7 });

//     // remove from end
//     vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7 };
//     vec.remove_if([](auto i) { return i > 5; });
//     log("size:", vec.size(), vec.storage_size());
//     for (auto i = 0; i < vec.size(); i++) {
//         log("i: ", i);
//         log(vec.at(i));
//     }
//     log("done", vec.size(), vec.storage_size());
//     ASSERT(vec.collect() == std::vector { 1, 2, 3, 4, 5 });

//     // // remove all
//     // vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7 };
//     // vec.remove_if([](int) { return true; });
//     // ASSERT(vec.collect() == std::vector<int>{});

//     // // remove none
//     // vec = swiss_vector { 1, 2, 3, 4, 5 };
//     // vec.remove_if([] (int) { return false; });
//     // ASSERT(vec.collect() == std::vector { 1, 2, 3, 4, 5 });
// }


// // TEST("swiss_vector iteration") {
// //     auto vec = swiss_vector { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

// //     // make some gaps
// //     vec.remove(1);
// //     vec.remove(3);
// //     vec.remove(5);

// //     auto total = 0;
// //     for (auto i = vec.begin(); i != vec.end(); ++i) {
// //         total += *i;
// //     }
// //     ASSERT(total == 43);

// // }
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
