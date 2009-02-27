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

#if defined(_WIN32) || defined(_WIN64)
#include<time.h>
#else
#include<sys/time.h>
#endif

class DeltaException {
public:
	DeltaException(std::string msg, char *e = NULL);
	char *message;
	char *error;
	~DeltaException();
};

class MessageEngine {
	public:
		MessageEngine(const char *filename, int line, const char *method);
		void add(const char *format, ...);
		char *getStr(void);
		char *getWhy(void);
	private:
		char s[500];
		char why[500];
	};

#define THROW_AWAY(why) { \
	MessageEngine e(__FILE__, __LINE__, __FUNCTION__); \
	e.add why ; \
	std::string s=e.getStr(); \
	throw DeltaException(s, e.getWhy()); \
	}
	
#endif
