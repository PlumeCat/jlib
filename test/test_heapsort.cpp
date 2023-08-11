// #include <jlib/heapsort.h>
// #include <jlib/test_framework.h>

// using namespace std;

// TEST("test static heapsort") {
//     constexpr auto N = 1000;
//     auto input = array<uint32_t, N>{};
//     auto output = array<uint32_t, N> { { 0 } };

//     // random inputs
//     srand(time(nullptr));
//     for (auto& i: input) {
//         i = rand() % 100;
//     }
    
//     heapsort(input, output);

//     // make sure output is in order
//     for (auto i = 1; i < N; i++) {
//         ASSERT(output[i-1] >= output[i]);
//     }
// }

// TEST("pop empty heap") {
//     auto h = dynamic_heap<int>{};
//     ASSERT_THROWS(h.pop());
// }

// TEST("test dynamic heapsort") {
//     constexpr auto N = 10000;
//     auto input = vector<uint32_t>();
//     auto output = vector<uint32_t>();
    
//     srand(time(nullptr));
//     for (auto i = 0; i < N; i++) {
//         input.emplace_back(rand() % N);
//     }

//     auto before = chrono::steady_clock::now();
//     heapsort(input, output);
//     auto after = chrono::steady_clock::now();
//     log("sorting took",
//         chrono::duration_cast<chrono::microseconds>(after - before).count() / 1000.f,
//         "ms"
//     );

//     for (auto i = 1; i < N; i++) {
//         ASSERT(output[i] <= output[i-1]);
//     }
// }
