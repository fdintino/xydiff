#ifndef HASH32_FAST__
#define HASH32_FAST__

#include "xercesc/util/XMLUniDefs.hpp"

/*
 *
 * 32 bits Hash
 * See file <lookup2.c> for details
 * From: http://burtleburtle.net/bob/hash/evahash.html
 *
 */

class hash32 {
	public:
		hash32 ( unsigned char *buffer, unsigned long length, const hash32 &startingValue ) ;
		hash32 ( unsigned char *buffer, unsigned long length ) ;
		hash32 ( const XMLCh *str);
		hash32 ( const char *str);
		hash32 ( const XMLCh *str, const hash32 &startingValue);
		hash32 ( const char *str, const hash32 &startingValue);
		hash32 ( void ) ;
		unsigned long value ;
} ;
	
#endif
