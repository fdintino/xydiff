#ifndef DELTAEXCEPTION_HXX
#define DELTAEXCEPTION_HXX

#include "infra/general/Log.hpp"

#include <string>

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

class DeltaException {
	public:
		DeltaException(std::string msg) {
			ERROR("msg="<<msg.c_str());
			printf("\n\n*** DELTA-EXCEPTION ***\n>> %s\n\n", msg.c_str());
			fflush(stdout);
			}
	};

class MessageEngine {
	public:
		MessageEngine(const char *filename, int line, const char *method);
		void add(const char *format, ...);
		char *getStr(void);
	private:
		char s[500];
	};

#define THROW_AWAY(why) { \
	MessageEngine e(__FILE__, __LINE__, __FUNCTION__); \
	e.add why ; \
	std::string s=e.getStr(); \
	throw DeltaException(s); \
	}
	
#endif
