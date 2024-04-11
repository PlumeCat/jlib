#include <jlib/defer.h>
#include <jlib/test_framework.h>

TEST("defer") {
    int x = 1;
    {
        defer {
            x = 2;
        };
    }

    ASSERT(x == 2);
}

TEST("defer cancel") {
    int x = 1;
    {
        auto d = defer_cancelable {
            x = 2;
        };
        d.cancel();
    }

    ASSERT(x == 1);
}