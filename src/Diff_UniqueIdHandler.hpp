#ifndef DIFF_UNIQUEIDHANDLER_HXX__
#define DIFF_UNIQUEIDHANDLER_HXX__

#include <set>
#include <string>
#include <map>


#include "xercesc/util/XercesDefs.hpp"


#include "xercesc/dom/deprecated/DOMString.hpp"
#include "xercesc/validators/DTD/DTDValidator.hpp"

class UniqueIdHandler {
	public:
		void RegisterAttrId(xercesc_2_2::DTDValidator *theDTDValidator);
		static std::string UniqueKey_from_TagAttr(const XMLCh* const tagName, const XMLCh* const attrName);
		bool isIdAttr(const std::string &key);
		std::map<unsigned long, int> v0nodeByIdAttrHash ;
	private:
		std::set<std::string> attrIdList ;
	} ;
#endif
