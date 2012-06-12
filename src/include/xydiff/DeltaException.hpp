#ifndef DELTAEXCEPTION_HXX
#define DELTAEXCEPTION_HXX

#include <string>

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)

#ifndef snprintf
#define snprintf _snprintf
#endif

#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif

#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

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


#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

#ifndef XYDIFF_WIN_GETTIMEOFDAY
#define XYDIFF_WIN_GETTIMEOFDAY
struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }

  return 0;
}
#endif
#endif