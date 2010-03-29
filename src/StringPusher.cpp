#include "StringPusher.hpp"
#include <stdlib.h>
#include <assert.h>

const int StringPusher::kIniSize = 32;

StringPusher::StringPusher() :
    buf((char*)malloc(kIniSize)),
    firstFree(buf),
    end(buf + kIniSize - 1)
{}

StringPusher::~StringPusher() {
    free(buf);
    buf = firstFree = end = 0;
}

void StringPusher::grow(int reqSpace) {
    int size = end - buf + 1;
    int space;
    do {
        size <<= 1;
        space = size - (firstFree - buf) - 1;
    } while (space < reqSpace);

    int firstFreeLen = firstFree - buf;
    buf = (char*)realloc(buf, size);
    assert(buf);
    firstFree = buf + firstFreeLen;
    end = buf + size - 1;
}
