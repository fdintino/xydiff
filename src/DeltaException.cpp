#ifdef __SunOS
#include <sys/varargs.h>
#endif

#include "XyDiff/DeltaException.hpp"
#include <errno.h>
#include<time.h>
#include<sys/time.h>
#include <stdarg.h>

MessageEngine::MessageEngine(const char *filename, int line, const char *method) {
  struct tm tm_time;
  struct timeval tval;
  time_t sec_time;

  time(&sec_time);
  gettimeofday(&tval, 0);
  localtime_r(&sec_time, &tm_time);
  char err[32];
  *err= 0;
  if(errno){
    snprintf(err, sizeof(err), " (%s)", strerror(errno));
    errno= 0;
  }

  snprintf(s, 499, 
	  "%02d/%02d/%02d %02d:%02d:%02d.%03ld %s:%d %s() : ",
	  tm_time.tm_mday, tm_time.tm_mon+1, tm_time.tm_year%100, 
	  tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
	  tval.tv_usec/1000,
    filename, line, method
	  );
	};

void MessageEngine::add(const char *format, ...) {
	int len=strlen(s);
#ifdef __SunOS
  snprintf(s+len, 499-len, "Sorry, Exception Text disabled on SunOS/Solaris");
#else
	va_list var_arg ;
	va_start (var_arg, format);
	vsnprintf(s+len, 499-len, format, var_arg);
	va_end(var_arg);
#endif
	};
	
char* MessageEngine::getStr(void) {
	return s;
	};
