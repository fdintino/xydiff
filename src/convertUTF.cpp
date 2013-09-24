#include "convertUTF.hpp"

#include <cstdlib>
#include <string.h>
#include <cstdio>

#include "StringPusher.hpp"
#include "infra/general/Log.hpp"


/** reference for latin9 encoding:
    http://www.cs.tut.fi/~jkorpela/latin9.html
    the table below will remap some unicode chars to latin9
    positions. 
    These are the differences between latin9 and latin1.
*/

unsigned latin9remap[8][2] = {
    {0x20ac, 0xa4},
    {0x160, 0xa6},
    {0x161, 0xa8},
    {0x17d, 0xb4},
    {0x17e, 0xb8},
    {0x152, 0xbc},
    {0x153, 0xbd},
    {0x178, 0xbe}
};

inline static unsigned remapLatin9(unsigned ch, bool *representable) {
    *representable = true;
    for (int i = 0; i < 8; ++i) {
        if (ch == latin9remap[i][0]) {
            return latin9remap[i][1];
        }
        if (ch == latin9remap[i][1]) {
            *representable = false;
            return ch;
        }
    }
    return ch;
}

bool UTF32ToUTF8(const wchar_t *source, std::string *outResult) {
    StringPusher res;
    if (!source) {
        FATAL("null input string");
        abort();
    }
    bool ok = true;
    for(const wchar_t *pos = source; *pos; ++pos) {
        unsigned ch = *pos;
        if (ch < 0x80) {
            res.push_back((char)ch);
        } else if (ch < 0x800) {
            res.push_back((ch >> 6) | 0xc0);
            res.push_back((ch & 0x3f) | 0x80);
        } else if (ch < 0x10000) {
            res.push_back((ch >> 12) | 0xe0);
            res.push_back(((ch >> 6) & 0x3f) | 0x80);
            res.push_back((ch &  0x3f) | 0x80);
        } else {
            res.push_back('?');
            ok = false;
        }
    }
    *outResult = res.c_str();
    return ok;
}

bool UTF8ToLatin9(const char *inSource, unsigned sourceLen,
                  std::string *outResult,
                  bool useXmlEntities,
		  bool useEscapeSequenceHack) {
    static const char kReplacementChar = '?';
    const unsigned char *source = (const unsigned char *)inSource;
    const unsigned char *sourceEnd = source + sourceLen;
    unsigned ch;
    int len;
    char buf[32];
    StringPusher res;
    const char *escapeFormat = useEscapeSequenceHack ? 
	"&#xy%X;" : "&#x%X;";

    bool ok = true;
    while(source < sourceEnd) {
        if (*source < 0x80) {
            const unsigned char *p = source;
            do {
                ++p;
            } while (p < sourceEnd && *p < 0x80);
            res.append((const char *)source, (p - source));
            source = p;
        } else {
            ch = *source++;
            //0 extraLen means error
            int extraLen = 
                (ch < 0xc0) ? 0 : 
                (ch < 0xe0) ? 1 : 
                (ch < 0xf0) ? 2 : 0;
            if (source + extraLen > sourceEnd) {
                res.push_back('?');
                ok = false;
                break;
            }
 	    
            switch(extraLen) {
            case 2: 
                ch <<= 6;
                ch += *source++;
                ch <<= 6;
                ch += *source++;
                ch -= 0x0e2080;
                break;
            case 1: 
                ch <<= 6;
                ch += *source++;
                ch -= 0x3080;
                break;
            default:
                ch = '?';
                ok = false;
            }
            
            bool representable = true;
            if ((ch >= 0x152 && ch <= 0x20ac) ||
                (ch >= 0xa4 && ch <= 0xbe)) {
                ch = remapLatin9(ch, &representable);
            }
            if (ch <= 0xff && representable) {
                res.push_back((char)ch);
            } else {
                if (ch > 0xffff) {
                    res.push_back('?');
                    ok = false;
                } else if (useXmlEntities) {
                    len = snprintf(buf, sizeof(buf), escapeFormat, ch);
                    res.append(buf, len);
                } else {
                    res.push_back(kReplacementChar);
                }
            }
        }
    }
    *outResult = res.c_str();
    return ok;
}
