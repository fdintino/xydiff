#include "include/XyLatinDocument.hpp"
#include "include/XyStr.hpp"

#include "xercesc/sax/SAXParseException.hpp"
#include "xercesc/sax/Locator.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/TransService.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"
#include "xercesc/validators/schema/SchemaSymbols.hpp"

#include <iostream>
#include <fstream>
#include <string.h>

XyLatinDocument::XyLatinDocument(const XMLCh* const toTranscode, int size, const int fastOp) : XyStr(toTranscode, size, fastOp) {
	if (theFastOptions & XyStr::NO_SOURCE_COPY) {
		fWideForm = const_cast<XMLCh*>(toTranscode);
		(void)this->localForm();
		fWideForm = NULL;
		fWideFormSize = 0;
	}
}
XyLatinDocument::XyLatinDocument(const char * const toTranscode, int size, const int fastOp) : XyStr(toTranscode, size, fastOp) {
	if (theFastOptions & XyStr::NO_SOURCE_COPY) {
		fLocalForm = const_cast<char*>(toTranscode);
		(void)this->wideForm();
		fLocalForm = NULL;
		fLocalFormSize = 0;
	}
}

const char* XyLatinDocument::localForm() {
	if ((fLocalForm==NULL)&&(fWideForm)) {
		XyStr::transcodeFromUTF32_andReplaceXmlHeader(fWideForm, fWideFormSize, "ISO-8859-1", &fLocalForm, &fLocalFormSize);
	}
	return fLocalForm;
}

const XMLCh* XyLatinDocument::wideForm() {
	if ((fWideForm==NULL)&&(fLocalForm)) {
		XyStr::transcodeToUTF32(fLocalForm, fLocalFormSize, "ISO-8859-1", &fWideForm, &fWideFormSize);
	}
	return fWideForm;
}
