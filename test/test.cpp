#define JLIB_IMPLEMENTATION
#define JLIB_TEST_IMPLEMENTATION
#include <jlib/log.h>
#include <jlib/jlib_all.h>

int main(int argc, char* argv[]) {
    run_tests(std::vector<std::string_view> { argv+1, argv + argc });
    return 0;
}
