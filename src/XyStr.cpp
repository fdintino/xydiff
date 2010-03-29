#include "include/XyStr.hpp"

#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/TransService.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include "infra/general/Log.hpp"
#include "infra/general/Logf.hpp"


#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <vector>

XERCES_CPP_NAMESPACE_USE 

std::map<std::string,XMLTranscoder*> XyStr::staticTranscoder ; 

/*
 * XyStr functions (identical to StrX in xerces samples)
 */

XyStr::XyStr(const XMLCh* const toTranscode, int size, const int fastOp) : fLocalForm(NULL), fWideForm(NULL), fLocalFormSize(-1), fWideFormSize(size), theFastOptions(fastOp)
{
	if (toTranscode!=NULL) {
		if (fWideFormSize<0) fWideFormSize = XMLString::stringLen(toTranscode);
		if ((theFastOptions & XyStr::NO_SOURCE_COPY)==0) {
			fWideForm = newCopyOf(toTranscode, size);
		}
	}
}

XyStr::XyStr(const char* const toTranscode, int size, const int fastOp) : fLocalForm(NULL), fWideForm(NULL), fLocalFormSize(size), fWideFormSize(-1), theFastOptions(fastOp)
{
	if (toTranscode!=NULL) {
		if (fLocalFormSize<0) fLocalFormSize = strlen(toTranscode);
		if ((theFastOptions & XyStr::NO_SOURCE_COPY)==0) {
			fLocalForm = newCopyOf(toTranscode, size);
		}
	}
}

XyStr::~XyStr() {
	if (fLocalForm) XMLString::release( & fLocalForm);
	if (fWideForm ) XMLString::release( & fWideForm );
	fLocalFormSize = -1;
	fWideFormSize  = -1;
}

unsigned int XyStr::localFormSize() {
	if (fLocalForm==NULL) (void)this->localForm();
	return fLocalFormSize;
}

unsigned int XyStr::wideFormSize() {
	if (fWideForm==NULL) (void)this->wideForm();
	return fWideFormSize;
}

char * XyStr::getLocalFormOwnership() {
	(void)this->localForm();
	char *r = fLocalForm;
	fLocalForm = NULL;
	return r;
}
XMLCh* XyStr::getWideFormOwnership() {
	(void)this->wideForm();
	XMLCh *r = fWideForm;
	fWideForm = NULL;
	return r;
}

XyStr::operator const XMLCh*() {
	return this->wideForm();
}

XyStr::operator const char*() {
	return this->localForm();
}

const char *ErrorMsgCouldNotTranscode =   "??lTranscode Error??"; // length is 20, +1 for null char
//XMLCh LErrorMsgCouldNotTranscode[21];
//XMLCh testStr = XMLString::transcode("??wTranscode Error??", LErrorMsgCouldNotTranscode, 20);
//const XMLCh *LErrorMsgCouldNotTranscode = XMLString::transcode("Transcode Error"); // length is 20, +1 for null char
const unsigned int MinLengthForErrorMessage = 25;

XMLCh* XyStr::newCopyOf(const XMLCh* source, int size) {
	if (size<0) size=XMLString::stringLen(source);
	XMLCh *r = new XMLCh[size+1];
	memcpy(r, source, size*sizeof(XMLCh));
	r[size]=chNull;
	return r;
}

char*  XyStr::newCopyOf(const char* source, int size) {
	if (size<0) size=strlen(source);
	char *r = new char[size+1];
	strncpy(r, source, size*sizeof(char));
	r[size]='\0';
	return r;
}



unsigned int XyStr::transcodeToUTF32(const char* const source, const unsigned int length, const char *outputEncoding, XMLCh** result, int *resultLength) {
	const XMLCh* tto32errorMsg = XMLString::transcode("\n??? Transcode To UTF-32 Error ???\n");

	*result = NULL;
	if (resultLength) *resultLength=-1;
	if (outputEncoding==NULL) {
		ERROR("Encoding is NULL");
		*result = XyStr::newCopyOf(XMLString::transcode("\n\nINTERNAL ERROR === 'outputEncoding' is NULL\n\n"));
		if (resultLength) *resultLength=(int)XMLString::stringLen(*result);
		return 1;
	}

	XMLTranscoder *transcoder = getTranscoder(outputEncoding);
	if(transcoder==NULL){
		ERROR("Can not find transcoder for: "<<outputEncoding);
		*result = XyStr::newCopyOf(XMLString::transcode("??? Transcode Error ???"));
		if (resultLength) *resultLength=(int)XMLString::stringLen(*result);
		fprintf(stderr, "??? Transcode Error ???");
		return 1;
	}

	unsigned int sourceSize = length;
	unsigned int tmpBufferSize = length + MinLengthForErrorMessage ;
	// STACK CAN NOT HANDLE VERY LARGE DATA (>> 10MB) : XMLCh tmpResult[tmpBufferSize];
	XMLCh *tmpResult = new XMLCh[tmpBufferSize];
	
	unsigned int bytesEaten = 0;
	unsigned int numberInBuf = 0;
	bool error = false;
	while (bytesEaten<sourceSize) {
		try{
			char sourceBlock[XyStr::BlockSize+1];
			//bzero(sourceBlock, sizeof(char)*(XyStr::BlockSize+1));
			sourceBlock[XyStr::BlockSize]='\0';

			unsigned int maxSourceChars = sourceSize-bytesEaten;
			if (maxSourceChars>XyStr::BlockSize) maxSourceChars=XyStr::BlockSize;
			memcpy(sourceBlock, (void*)(source+bytesEaten), maxSourceChars*sizeof(char));

			unsigned int eatenThisTime = 0;
			unsigned int maxChars = tmpBufferSize - numberInBuf;
			if (maxChars > XyStr::BlockSize) maxChars = XyStr::BlockSize;
			if (maxChars < 1) {
				ERROR("Unexpected problem: destination buffer is full !");
			} else {			
				std::vector<unsigned char> charSizes(maxChars+1);
				unsigned int addedThisTime = transcoder->transcodeFrom((XMLByte*)sourceBlock, (XMLSize_t)maxSourceChars, tmpResult+numberInBuf, (XMLSize_t)maxChars, (XMLSize_t&)eatenThisTime, &charSizes[0]);
				bytesEaten  += eatenThisTime ;
				numberInBuf += addedThisTime ;
			}
					
			if (eatenThisTime==0) {
				ERROR("Error, transcoder does not eat any byte. Next byte is: "<<(unsigned int)sourceBlock[0]);
				NOTE("followed by: "<<(unsigned int)sourceBlock[1]<<(unsigned int)sourceBlock[2]<<(unsigned int)sourceBlock[3]);
				fprintf(stderr, "Error, transcoder does not eat any byte\n");
				XMLCh LErrorMsgCouldNotTranscode[21];
				XMLString::transcode("??wTranscode Error??", LErrorMsgCouldNotTranscode, 20);
				*result = XyStr::newCopyOf(LErrorMsgCouldNotTranscode);
				if (resultLength) *resultLength=XMLString::stringLen(*result);

				if (!(theFastOptions & XyStr::USE_STATIC_TRANSCODER)) {
					TRACE("delete transcoder");
					delete transcoder;
				}
				TRACE("delete [] tmpResult");
				delete [] tmpResult;
				return 1;
			}
			
		}catch(...){
			error = true;
			ERROR("transcode exception catched... leaving...");
			if (tmpBufferSize-numberInBuf-1>XMLString::stringLen(tto32errorMsg)) {
				memcpy(tmpResult+numberInBuf, tto32errorMsg, sizeof(XMLCh)*(1+XMLString::stringLen(tto32errorMsg)));
			}
			bytesEaten = sourceSize;
		}
	}

	*result = new XMLCh[numberInBuf+1];
	memcpy((void*)*result, (void*)tmpResult, numberInBuf*sizeof(XMLCh));
	(*result)[numberInBuf] = chNull;
	if (resultLength) *resultLength=(int)numberInBuf;

	if (!(theFastOptions & XyStr::USE_STATIC_TRANSCODER)) {
		TRACE("delete transcoder");
		delete transcoder;
	}

	TRACE("delete [] tmpResult");
	delete [] tmpResult;
	if (error) return 1;
	else return 0;
}

class XMLTranscoder * XyStr::getTranscoder(const char *encoding) {
	XMLTranscoder * transcoder = NULL;
	if (theFastOptions & XyStr::USE_STATIC_TRANSCODER) {
		std::string name = encoding;
		if (staticTranscoder.find(name)!=staticTranscoder.end()) {
			transcoder = staticTranscoder[name];
		} else {
			XMLTransService::Codes failReason;
			transcoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor(encoding, failReason, XyStr::BlockSize);
			staticTranscoder[name] = transcoder ;
		}
	} else {
		XMLTransService::Codes failReason;
		transcoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor(encoding, failReason, XyStr::BlockSize);
	}
	if(transcoder==NULL){
		ERROR("Can Not Create Transcoder: " << encoding);
	}
	return transcoder;
}

unsigned int XyStr::internal_transcodeFromUTF32(const XMLCh* const source, const unsigned int length, const char *outputEncoding, char* result, const unsigned int resultBufferSize, int *resultLength, bool escapeSequenceXyHack) {
	TRACEF(("source=%p, length=%u, outputEncoding=%s", source, length, outputEncoding));
	
	*resultLength=0;
	if (outputEncoding==NULL) {
		ERROR("Encoding is NULL");
		sprintf(result, "??? Transcode Error ???");
		*resultLength=(int)strlen(result);
		return 1;
	}
	
	XMLTranscoder *transcoder = getTranscoder(outputEncoding);
	if(transcoder==NULL){
		ERROR("Can not find transcoder for: "<<outputEncoding);
		sprintf(result, "??? Transcode Error ???");
		*resultLength=(int)strlen(result);
		fprintf(stderr, "%s", result);
		return 1;
	}

	unsigned int errors      = 0;
	unsigned int sourceSize  = length;
	XMLSize_t charsEaten  = 0;
	XMLSize_t numberInBuf = 0;
	
	bool error               = false;
	while (charsEaten<sourceSize) {
		XMLSize_t maxChars = resultBufferSize - numberInBuf;
		if (maxChars > XyStr::BlockSize) maxChars = XyStr::BlockSize;

		if (maxChars < 15) {
			TRACE("Destination buffer is full !");
			if (!(theFastOptions & XyStr::USE_STATIC_TRANSCODER)) {
				TRACE("delete transcoder");
				delete transcoder;
			}
			*resultLength=0;
			return INT_MAX;
		}
		
		XMLCh sourceBlock[XyStr::BlockSize+1];
		//bzero(sourceBlock, sizeof(XMLCh)*(XyStr::BlockSize+1));
		sourceBlock[XyStr::BlockSize]=chNull;

		unsigned int maxSourceChars = sourceSize-charsEaten;
		if (maxSourceChars>XyStr::BlockSize) maxSourceChars=XyStr::BlockSize;
		memcpy(sourceBlock, (void*)(source+charsEaten), maxSourceChars*sizeof(XMLCh));
		sourceBlock[maxSourceChars]=chNull;
		
		//int resultLength = strlen(result);
		
		XMLSize_t eatenThisTime = 0;
		
		try{
			TRACEF(("transcode(%p, %d, %p, %d, (out), throw)", sourceBlock, maxSourceChars, (XMLByte*)result+numberInBuf, maxChars));
			XMLSize_t addedThisTime = transcoder->transcodeTo((XMLCh*)sourceBlock, (XMLSize_t)maxSourceChars, (XMLByte*)result+numberInBuf, (XMLSize_t)maxChars, eatenThisTime, XMLTranscoder::UnRep_Throw);
			charsEaten  += eatenThisTime ;
			numberInBuf += addedThisTime ;
		}catch(...){
			bool UseCharacterEntity = ((theFastOptions & XyStr::NO_CHARACTER_ENTITY)==0);
			// NO LOSS OF CHARACTERS
			// If we do not want to loose exotic characters, we can encode them using characters entity in XML
			// This will slow down the process since we now have to transcode them one by one
			if (UseCharacterEntity) {
				maxSourceChars=1;
				sourceBlock[1]=chNull;
			}
			error = true;
			// The document is not valid, but we can accept it by replacing non transcodable chars by "?".
			TRACEF(("transcode(%p, %d, %p, %d, (out), repl)", sourceBlock, maxSourceChars, (XMLByte*)result+numberInBuf, maxChars));
			XMLSize_t addedThisTime = transcoder->transcodeTo((XMLCh*)sourceBlock, (XMLSize_t)maxSourceChars, (XMLByte*)result+numberInBuf, (XMLSize_t)maxChars, (XMLSize_t&)eatenThisTime, XMLTranscoder::UnRep_RepChar);
			//Unfortunately xerces does not allow to define the replacement char. It is hard-coded to be 0x1A !
			//We cannot store the document with this character, since then the document could not be parsed back again.
			// That's why we must traverse again the coded chars and replace 0x1A by '?'
			char* p = (char*) result+numberInBuf;
			bool characterEntityOK = false;
			if (UseCharacterEntity) {
				if (eatenThisTime!=1) {
					WARNINGF(("eatenThisTime is %d and not 1, can not use character entity", eatenThisTime));
				} else {
					if (p[0]==0x1A) {
						unsigned int x = (unsigned int)sourceBlock[0];
						if (transcoder->canTranscodeTo(x)) {
							WARNING("surprising... this was really the 0x1A character");
						} else {
							TRACEF(("replacing by character entity"));
static bool disabled=false;
							if (escapeSequenceXyHack && !disabled) sprintf(p, "&#xy%X;", x);
							else sprintf(p, "&#x%X;", x);
							addedThisTime = strlen(p);
						}
					}
				}
			}
			if (!characterEntityOK) {
				TRACEF(("replace 0x1A chars by '?' (p=%p, addedThisTime=%u)", p, addedThisTime));
				for(unsigned int i=0; i<addedThisTime; i++) {
					if (p[i]==0x1A) {
						p[i] = '?';
						++errors;
					}
				}
			}
			charsEaten  += eatenThisTime ;
			numberInBuf += addedThisTime ;
		}
		if (eatenThisTime==0) {
			ERROR("Error, transcoder does not eat any byte, next XMLCh="<<(int)sourceBlock[0]);
			fprintf(stderr, "Error, transcoder does not eat any XMLCh\n");
			sprintf(result, "??? Transcode Error ???");
			*resultLength=strlen(result);
			if (!(theFastOptions & XyStr::USE_STATIC_TRANSCODER)) {
				TRACE("delete transcoder");
				delete transcoder;
			}
			return 1;
		}
	}
	
	if (!(theFastOptions & XyStr::USE_STATIC_TRANSCODER)) {
		TRACE("delete transcoder");
		delete transcoder;
	}
	
	*resultLength = numberInBuf;
	
	return errors;
}

unsigned int XyStr::transcodeFromUTF32(const XMLCh* const source, const unsigned int length, const char *outputEncoding, char** result, int *resultLength, bool escapeSequenceXyHack) {
	int errors = transcodeFromUTF32_and_AddPrefix(source, length, "", 0, outputEncoding, result, resultLength, escapeSequenceXyHack);
	return errors;
}

unsigned int XyStr::transcodeFromUTF32_andReplaceXmlHeader(const XMLCh* const source, const unsigned int length, const char *outputEncoding, char** result, int *resultLength) {
	static const XMLCh *refHeader = XMLString::transcode("<?xml version=\"1.0\" encoding=\"UTF-32\" ?>");
	static unsigned long refHeaderSize = XMLString::stringLen(refHeader);
	
	if (XMLString::startsWith(source, refHeader)) {
		const XMLCh *source2 = source + refHeaderSize;
		unsigned int length2 = length - refHeaderSize;
		
		char prefix[256];
		sprintf(prefix, "<?xml version=\"1.0\" encoding=\"%s\" ?>", outputEncoding);
		unsigned int prefixLength = strlen(prefix);
		
		int errors = transcodeFromUTF32_and_AddPrefix(source2, length2, prefix, prefixLength, outputEncoding, result, resultLength, false);
		return errors;
	} else {
		WARNING("Header not found");
		int errors = transcodeFromUTF32_and_AddPrefix(source, length, "", 0, outputEncoding, result, resultLength, false);
		return errors;
	}
}

unsigned int XyStr::transcodeFromUTF32_and_AddPrefix(const XMLCh* const source, const unsigned int sourceLength, const char *prefixToAdd, const unsigned int prefixLength, const char *outputEncoding, char **result, int *resultLength, bool escapeSequenceXyHack) {

	// should be ok for 99% of european applications
	unsigned int tmpBufferSize = MinLengthForErrorMessage +prefixLength +2*sourceLength;
	int numberInBuf = 0;
	
	unsigned int errors = 0;

	char *tmpResult = new char[tmpBufferSize];
	if (prefixLength) memcpy(tmpResult, prefixToAdd, prefixLength*sizeof(char));	
	errors = internal_transcodeFromUTF32(source, sourceLength, outputEncoding, tmpResult+prefixLength, tmpBufferSize-prefixLength, &numberInBuf, escapeSequenceXyHack);

	if (errors==INT_MAX) {
		TRACE("Buffer not sufficient. We need a larger one.");
		delete [] tmpResult; tmpResult=NULL;
		tmpBufferSize = 12*(sourceLength+MinLengthForErrorMessage)+prefixLength+4; // worst case: each XMLCh becomes &#x87654321, so it's 11 bytes... let's say 12 to be safe...
		tmpResult = new char[tmpBufferSize];
		if (prefixLength) memcpy(tmpResult, prefixToAdd, prefixLength*sizeof(char));	
		errors = internal_transcodeFromUTF32(source, sourceLength, outputEncoding, tmpResult+prefixLength, tmpBufferSize-prefixLength, &numberInBuf, escapeSequenceXyHack);
	}

	if (errors==INT_MAX) {
		FATAL("Transcode OutputBuffer is FULL even for Second Pass");
		FATAL("SourceLength="<<sourceLength<<", BufferSize="<<tmpBufferSize);
		FATAL("Please fix program");
#ifdef DEBUG
		exit(-1);
#else
		delete [] tmpResult; tmpResult=NULL;
		*result = XyStr::newCopyOf("!!! XyTranscode Error !!!");
		*resultLength = strlen(*result)+1;
		return INT_MAX;
#endif
	} else {
		if (errors>0) {
			ERROR("There were "<<errors<<" transcoding errors");
		}

		numberInBuf += prefixLength;
		
		TRACE("copy result of size " << numberInBuf);
		if ((theFastOptions & XyStr::NO_BUFFER_ADJUST)||((unsigned int)numberInBuf>(10+(9*tmpBufferSize)/10))) {
			*result = tmpResult;
		} else {
			*result = XyStr::newCopyOf(tmpResult, numberInBuf);
			delete [] tmpResult; tmpResult=NULL;
		}
	}

	if (resultLength) *resultLength=(int)numberInBuf;

	return errors;
}
