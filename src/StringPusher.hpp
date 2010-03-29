#ifndef _STRINGPUSHER_H_
#define _STRINGPUSHER_H_

#include <string.h>

class StringPusher {
public:
    StringPusher();
    ~StringPusher();
    
    inline void push_back(char c) {
	if (firstFree == end) {
	    grow(1);
	}
	*firstFree = c;
	++firstFree;
    }

    inline void append(const char *s, int len) {
	if (firstFree + len > end) {
	    grow(len);
	}
	memcpy(firstFree, s, len);
	firstFree += len;
    }

    inline const char *c_str() {
	*firstFree = 0; //insure 0-ended
	return buf;
    }

private:
    //disallow copy
    StringPusher(const StringPusher &);
    void operator=(const StringPusher &);

    void grow(int reqSpace);

    static const int kIniSize;
    char * buf;
    char * firstFree;
    char * end;
};

#endif
