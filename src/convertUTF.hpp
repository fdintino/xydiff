#ifndef _CONVERTUTF_H_
#define _CONVERTUTF_H_

#if defined(_WIN32) || defined(_WIN64)
#define snprintf _snprintf
//#ifndef vsnprintf
//#define vsnprintf _vsnprintf
//#endif
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#include <string>

/** the conversions methods below don't handle BOM (there must be no BOM).
    return false on conversion error (e.g. invalid utf8 sequence).
    Latin9 means ISO-8859-1.
*/

bool UTF8ToLatin9(const char *source, unsigned sourceLen,
                  std::string *outResult, 
		  bool useXmlEntities = true,
		  bool useEscapeSequenceHack = false);

inline bool UTF8ToLatin9(const std::string &source, 
			 std::string *outResult, 
			 bool useXmlEntities = true,
			 bool useEscapeSequenceHack = false) {
    return UTF8ToLatin9(source.c_str(), source.size(),
                        outResult, useXmlEntities, useEscapeSequenceHack);
}

bool UTF32ToUTF8(const wchar_t *source, std::string *outResult);
inline bool UTF32ToUTF8(const std::wstring &source, std::string *outResult) {
    return UTF32ToUTF8(source.c_str(), outResult);
}

#endif
