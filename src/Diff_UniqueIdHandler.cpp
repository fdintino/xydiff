#include "Diff_UniqueIdHandler.hpp"

#include "xercesc/framework/XMLValidator.hpp"

#include "xydiff/XyUTF8Str.hpp"

XERCES_CPP_NAMESPACE_USE

bool UniqueIdHandler::isIdAttr(const std::string &key) {
	return ( attrIdList.find(key)!=attrIdList.end() );
	}

// Note: by XML spec, this key identies perfectly a couple because a tag can not contain '<' characters
std::string UniqueIdHandler::UniqueKey_from_TagAttr(const XMLCh* const tagName, const XMLCh* const attrName) {
	XyUTF8Str _tagName(tagName);
	XyUTF8Str _attrName(attrName);
	return std::string("<")+std::string(_tagName.localForm())+std::string(">")+std::string(_attrName.localForm());
	}
	
