#include <jlib/generic_ostream.h>
#include <jlib/swiss_vector.h>
#include <jlib/test_framework.h>

TEST("swiss_vector basics") {
    auto vec = swiss_vector<int> { 1, 2, 3, 4, 5 };
    vec.add(1);
    vec.add(2);
    vec.add(3);

    ASSERT(vec.collect() == std::vector<int>({ 1, 2, 3, 4, 5, 1, 2, 3 }));
}

TEST("swiss vector with some removes") {
    auto vec = swiss_vector<int> { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    vec.remove(3);
    vec.remove(6);
    vec.add(32);
    vec.remove(7);
    vec.add(64);

    ASSERT(vec.collect() == std::vector<int>({ 1, 2, 3, 5, 6, 32, 64, 9, 10 })); //
}

TEST("remove from middle, push two, size didn't change") {
    auto vec = swiss_vector<int> { 1, 2, 3, 4, 5 };
    vec.remove(2);
    vec.remove(3);
    vec.add(6);
    vec.add(7);
    ASSERT(vec.collect() == std::vector<int>({ 1, 2, 7, 6, 5 }));
}

// subclass that makes the free slots publicly accessible
template<typename T> class sv2 : public swiss_vector<T> {
public:
    template<typename... R>
    sv2(R... r):
        swiss_vector<T>(r...) {}
    std::vector<size_t>& get_free() {
        return this->free_slots;
    }
};
TEST("remove from end, free slots not used") {
    auto vec = sv2<int> { 1, 2, 3, 4, 5, 6 };
    vec.remove(5);
    vec.remove(4);
    ASSERT(vec.get_free().size() == 0);
}

TEST("swiss vector begin/end") {
    auto vec = swiss_vector<int> { 1, 2, 3, 4, 5, 6 };

    ASSERT(vec.begin().index == 0);
    ASSERT(vec.end().index == 6);

    vec.remove(5);
    ASSERT(vec.begin().index == 0);
    ASSERT(vec.end().index == 5);

    vec.remove(0);
    ASSERT(vec.begin().index == 1);
    ASSERT(vec.end().index == 5);

    vec.remove(4);
    ASSERT(vec.begin().index == 1);
    ASSERT(vec.end().index == 4);

    vec.remove(2);
    ASSERT(vec.begin().index == 1);
    ASSERT(vec.end().index == 4);

    vec.remove(1);
    auto b = vec.begin().index;
    ASSERT(b == 3);
    ASSERT(vec.end().index == 4);
}

TEST("swiss_vector iteration") {
    auto vec = swiss_vector<int> { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // make some gaps
    vec.remove(1);
    vec.remove(3);
    vec.remove(5);

    log(vec.collect());

    auto total = 0;
    // for (auto& i: vec) {
    for (auto i = vec.begin(); i != vec.end(); ++i) {
        std::cout << *i << std::endl;
        total += *i;
    }
    log("total: ", total);
    // ASSERT(total == 43);
}