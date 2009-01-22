#ifndef __XYDELTADOM_HXX
#define __XYDELTADOM_HXX

#include <stdio.h>
#include "xercesc/dom/DOMDocument.hpp"

namespace XyDelta {
	// doc1xidmap can be NULL => the default XidMap (1-n|n+1) will be used
	xercesc_3_0::DOMDocument* XyDiff(xercesc_3_0::DOMDocument* doc1, const char *doc1name, xercesc_3_0::DOMDocument* doc2, const char *doc2name, const char *doc1xidmap=NULL);
	
	void SaveDomDocument(xercesc_3_0::DOMDocument* d, const char *filename);
	
	unsigned int estimateDocumentSize(xercesc_3_0::DOMDocument* doc);
	unsigned int estimateSubtreeSize(xercesc_3_0::DOMNode *node);

	xercesc_3_0::DOMNode* ReverseDelta(xercesc_3_0::DOMDocument *reversedDeltaDoc, xercesc_3_0::DOMNode *deltaElement);
	xercesc_3_0::DOMDocument* ApplyDelta(xercesc_3_0::DOMDocument *doc, xercesc_3_0::DOMNode *deltaElement, bool CreateNewDocumentResult=true);
	} ;

extern bool _XyDiff_DontSaveXidmapToFile ;


#endif
