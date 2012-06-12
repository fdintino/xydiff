#ifndef DIFF_UNIQUEIDHANDLER_HXX__
#define DIFF_UNIQUEIDHANDLER_HXX__

#include <set>
#include <string>
#include <map>

#include "xercesc/util/XMLString.hpp"

class UniqueIdHandler {
	public:
		static std::string UniqueKey_from_TagAttr(const XMLCh* const tagName, const XMLCh* const attrName);
		bool isIdAttr(const std::string &key);
		std::map<unsigned long, int> v0nodeByIdAttrHash ;
	private:
		std::set<std::string> attrIdList ;
	} ;
#endif
