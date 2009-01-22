#ifndef XYSTR_HXX__
#define XYSTR_HXX__

/* Class that handles transcode value of XMLString */

#include <stdio.h>
#include <iostream>
#include <map>
#include <string>

#include "xercesc/util/TransService.hpp"

#ifndef XERCES_STRINGS_HPP_INCLUDED
#define XERCES_STRINGS_HPP_INCLUDED

#include <boost/scoped_array.hpp>
#include <xercesc/util/XMLString.hpp>

typedef std::basic_string<XMLCh> XercesString;

// Converts from a narrow-character string to a wide-character string.
inline XercesString fromNative(const char* str)
{
    boost::scoped_array<XMLCh> ptr(xercesc::XMLString::transcode(str));
    return XercesString(ptr.get( ));
}

// Converts from a narrow-character string to a wide-charactr string.
inline XercesString fromNative(const std::string& str)
{
    return fromNative(str.c_str( ));
}

// Converts from a wide-character string to a narrow-character string.
inline std::string toNative(const XMLCh* str)
{
    boost::scoped_array<char> ptr(xercesc::XMLString::transcode(str));
    return std::string(ptr.get( ));
}

// Converts from a wide-character string to a narrow-character string.
inline std::string toNative(const XercesString& str)
{
    return toNative(str.c_str( ));
}

#endif // #ifndef XERCES_STRINGS_HPP_INCLUDED

class XyStr {
	public:
		enum FAST_OPTIONS { NO_FAST_OPTION        = 0x0,
		                    NO_SOURCE_COPY        = 0x1,
		                    NO_BUFFER_ADJUST      = 0x2,
		                    USE_STATIC_TRANSCODER = 0x4,
				    NO_CHARACTER_ENTITY   = 0x8
		};
		
		XyStr(const XMLCh* const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION) ;
		XyStr(const char * const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION) ;

		virtual ~XyStr(void) ;

		virtual const char*  localForm() = 0;
		virtual const XMLCh* wideForm()  = 0;

		char * getLocalFormOwnership() ;
		XMLCh* getWideFormOwnership()  ;

		unsigned int localFormSize();
		unsigned int wideFormSize();

		operator const char*();
		operator const XMLCh*();
		
		unsigned int transcodeToUTF32(const char* const source, const unsigned int length, const char *outputEncoding, XMLCh** result, int *resultLength=NULL);
		unsigned int transcodeFromUTF32(const XMLCh* const source, const unsigned int length, const char *outputEncoding, char** result, int *resultLength=NULL, bool escapeSequenceXyHack=false);

		unsigned int transcodeFromUTF32_andReplaceXmlHeader(const XMLCh* const source, const unsigned int length, const char *outputEncoding, char** result, int *resultLength=NULL);

		static const unsigned int BlockSize = 8*1024;
	
		static XMLCh* newCopyOf(const XMLCh* source, int size=-1);
		static char*  newCopyOf(const char* source, int size=-1);

		
	protected :
		char*   fLocalForm;
		XMLCh*  fWideForm;
		int	fLocalFormSize;
		int	fWideFormSize;

		// The difference with public API is that the result buffer is here given as an input
		unsigned int internal_transcodeFromUTF32(const XMLCh* const source, const unsigned int length, const char *outputEncoding, char* result, const unsigned int resultBufferSize, int *resultLength, bool escapeSequenceXyHack);
		unsigned int transcodeFromUTF32_and_AddPrefix(const XMLCh* const source, const unsigned int sourceLength, const char *prefixToAdd, const unsigned int prefixLength, const char *outputEncoding, char **result, int *resultLength, bool escapeSequenceXyHack);

		int theFastOptions ;
		
		class xercesc_3_0::XMLTranscoder * getTranscoder(const char *encoding);
		
		static std::map<std::string,xercesc_3_0::XMLTranscoder*> staticTranscoder ; 
	} ;

#endif
