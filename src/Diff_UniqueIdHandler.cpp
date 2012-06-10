#include "Diff_UniqueIdHandler.hpp"

#include "xercesc/util/NameIdPool.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/framework/XMLValidator.hpp"
#include "xercesc/parsers/SAXParser.hpp"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xydiff/XyLatinStr.hpp"
#include "xydiff/XyUTF8Str.hpp"
#include "Tools.hpp"

XERCES_CPP_NAMESPACE_USE

void UniqueIdHandler::RegisterAttrId( DTDValidator *theValidator ) {
  /*  NameIdPoolEnumerator<DTDElementDecl> elemEnum = theValidator->getElemEnumerator();
    while(elemEnum.hasMoreElements())
    {
        const DTDElementDecl& curElem = elemEnum.nextElement();
        // Get an enumerator for this guy's attributes if any
        if (curElem.hasAttDefs())
        {
            XMLAttDefList& attList = curElem.getAttDefList();
            while (attList.hasMoreElements())
            {
                const XMLAttDef& curAttDef = attList.nextElement();
                if (curAttDef.getType()==XMLAttDef::ID) {
                    string key = UniqueIdHandler::UniqueKey_from_TagAttr(curElem.getFullName(),curAttDef.getFullName());
										printf("key %s found\n", key.c_str());
										if (attrIdList.find(key)==attrIdList.end()) attrIdList.insert(key);
										else throw VersionManagerException("Runtime Error", __FUNCTION__, string("key already present: ")+key );
                }
            }
        }
    }
  */
}

bool UniqueIdHandler::isIdAttr(const std::string &key) {
	return ( attrIdList.find(key)!=attrIdList.end() );
	}

// Note: by XML spec, this key identies perfectly a couple because a tag can not contain '<' characters
std::string UniqueIdHandler::UniqueKey_from_TagAttr(const XMLCh* const tagName, const XMLCh* const attrName) {
	XyUTF8Str _tagName(tagName);
	XyUTF8Str _attrName(attrName);
	return std::string("<")+std::string(_tagName.localForm())+std::string(">")+std::string(_attrName.localForm());
	}
	
