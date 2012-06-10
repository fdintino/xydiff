#include "xydiff/XyUTF8Document.hpp"
#include "xydiff/XyStr.hpp"

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

/*
 * XyUTF8Str functions (identical to StrX in xerces samples)
 */

XyUTF8Document::XyUTF8Document(const XMLCh* const toTranscode, int size, const int fastOp) : XyStr(toTranscode, size, fastOp) {
	if (theFastOptions & XyStr::NO_SOURCE_COPY) {
		fWideForm = const_cast<XMLCh*>(toTranscode);
		(void)this->localForm();
		fWideForm = NULL;
		fWideFormSize = 0;
	}
}
XyUTF8Document::XyUTF8Document(const char * const toTranscode, int size, const int fastOp) : XyStr(toTranscode, size, fastOp) {
	if (theFastOptions & XyStr::NO_SOURCE_COPY) {
		fLocalForm = const_cast<char*>(toTranscode);
		(void)this->wideForm();
		fLocalForm = NULL;
		fLocalFormSize = 0;
	}
}

const char* XyUTF8Document::localForm() {
	if ((fLocalForm==NULL)&&(fWideForm)) {
		XyStr::transcodeFromUTF32_andReplaceXmlHeader(fWideForm, fWideFormSize, "UTF-8", &fLocalForm, &fLocalFormSize);
	}
	return fLocalForm;
}

const XMLCh* XyUTF8Document::wideForm() {
	if ((fWideForm==NULL)&&(fLocalForm)) {
		XyStr::transcodeToUTF32(fLocalForm, fLocalFormSize, "UTF-8", &fWideForm, &fWideFormSize);
	}
	return fWideForm;
}
