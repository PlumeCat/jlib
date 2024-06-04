#define JLIB_IMPLEMENTATION
#define TESTS_IMPLEMENTATION
#include <jlib/jlib_all.h>

IMPLEMENT_TESTS()
int main(int argc, char* argv[]) {
    (void)argc, (void)argv;
    RUN_TESTS()
    return 0;
}
