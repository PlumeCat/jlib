#include <vector>
#include <string_view>

#define JLIB_IMPLEMENTATION
#define JLIB_TEST_IMPLEMENTATION
#include <jlib/test_framework.h>
#include <jlib/log.h>
#include <jlib/binary_file.h>
#include <jlib/text_file.h>

int main(int argc, char* argv[]) {
    run_tests(std::vector<std::string_view> { argv+1, argv + argc });
    return 0;
}
