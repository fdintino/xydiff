#ifndef __XYDELTADOM_HXX
#define __XYDELTADOM_HXX

#include <stdio.h>
#include "xercesc/dom/DOMDocument.hpp"

namespace XyDelta {
	// doc1xidmap can be NULL => the default XidMap (1-n|n+1) will be used
	xercesc_2_2::DOMDocument* XyDiff(xercesc_2_2::DOMDocument* doc1, const char *doc1name, xercesc_2_2::DOMDocument* doc2, const char *doc2name, const char *doc1xidmap=NULL);
	
	void SaveDomDocument(xercesc_2_2::DOMDocument* d, const char *filename);
	
	unsigned int estimateDocumentSize(xercesc_2_2::DOMDocument* doc);
	unsigned int estimateSubtreeSize(xercesc_2_2::DOMNode *node);

	xercesc_2_2::DOMNode* ReverseDelta(xercesc_2_2::DOMDocument *reversedDeltaDoc, xercesc_2_2::DOMNode *deltaElement);
	xercesc_2_2::DOMDocument* ApplyDelta(xercesc_2_2::DOMDocument *doc, xercesc_2_2::DOMNode *deltaElement, bool CreateNewDocumentResult=true);
	} ;

extern bool _XyDiff_DontSaveXidmapToFile ;


#endif
