#include "xydiff/XyLatinStr.hpp"

#include <string.h>

#include "xercesc/util/XMLString.hpp"

#include "xydiff/XyStr.hpp"
#include "infra/general/Logf.hpp"

#include "convertUTF.hpp"

XyLatinStr::XyLatinStr(const XMLCh* const toTranscode, int size, const int fastOp, bool escapeSequenceXyHack) : XyStr(toTranscode, size, fastOp), theEscapeSequenceXyHack(escapeSequenceXyHack) {
	if (theFastOptions & XyStr::NO_SOURCE_COPY) {
		fWideForm = const_cast<XMLCh*>(toTranscode);
		(void)this->localForm();
		fWideForm = NULL;
		fWideFormSize = 0;
	}
}
XyLatinStr::XyLatinStr(const char * const toTranscode, int size, const int fastOp) : XyStr(toTranscode, size, fastOp) {
	if (theFastOptions & XyStr::NO_SOURCE_COPY) {
		fLocalForm = const_cast<char*>(toTranscode);
		(void)this->wideForm();
		fLocalForm = NULL;
		fLocalFormSize = 0;
	}
}

char * XyLatinStr::CreateFromUTF8(const char * toTranscode, int size, bool escapeSequenceXyHack) {
	if (toTranscode==NULL) return NULL;
	if (size<0) size = strlen(toTranscode);
    std::string latin;
    UTF8ToLatin9(toTranscode, &latin, true, escapeSequenceXyHack);
    char *ret = new char[latin.size() + 1];
    strcpy(ret, latin.c_str());
    return ret;
}

void XyLatinStr::ConvertFromUTF8(std::string & inoutStr, bool escapeSequenceXyHack) {
    UTF8ToLatin9(inoutStr, &inoutStr, true, escapeSequenceXyHack);
}

const char* XyLatinStr::localForm() {
	if ((fLocalForm==NULL)&&(fWideForm)) {
		XyStr::transcodeFromUTF32(fWideForm, fWideFormSize, "UTF-8", &fLocalForm, &fLocalFormSize, theEscapeSequenceXyHack);
	}
	return fLocalForm;
}

const XMLCh* XyLatinStr::wideForm() {
	if ((fWideForm==NULL)&&(fLocalForm)) {
		XyStr::transcodeToUTF32(fLocalForm, fLocalFormSize, "UTF-8", &fWideForm, &fWideFormSize);
	}
	return fWideForm;
}

std::ostream& operator<<(std::ostream& target, XyLatinStr& toDump)
{
    target << toDump.localForm();
    return target;
}
