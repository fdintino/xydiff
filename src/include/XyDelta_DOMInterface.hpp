#ifndef __XYDELTADOM_HXX
#define __XYDELTADOM_HXX

#include <stdio.h>
#include "xercesc/dom/DOMDocument.hpp"
#include "include/XID_DOMDocument.hpp"

namespace XyDelta {
	// doc1xidmap can be NULL => the default XidMap (1-n|n+1) will be used
	xercesc::DOMDocument* XyDiff(xercesc::DOMDocument* doc1, const char *doc1name, xercesc::DOMDocument* doc2, const char *doc2name, const char *doc1xidmap=NULL);
	
	void SaveDomDocument(xercesc::DOMDocument* d, const char *filename);
	
	unsigned int estimateDocumentSize(xercesc::DOMDocument* doc);
	unsigned int estimateSubtreeSize(xercesc::DOMNode *node);

	xercesc::DOMNode* ReverseDelta(xercesc::DOMDocument *reversedDeltaDoc, xercesc::DOMNode *deltaElement);
	//xercesc::DOMDocument* ApplyDelta(XID_DOMDocument *xdoc, xercesc::DOMNode *deltaElement, bool applyAnnotations=false);
	XID_DOMDocument* ApplyDelta(XID_DOMDocument* xdoc, xercesc::DOMNode* deltaElement, bool applyAnnotations=false);
	} ;

extern bool _XyDiff_DontSaveXidmapToFile ;

class XyDOMDelta {
public:
	XyDOMDelta(XID_DOMDocument* doc1p, XID_DOMDocument* doc2p, const char *doc1xidmapp=NULL);
	~XyDOMDelta();
	XID_DOMDocument* createDelta();
private:
	XID_DOMDocument* doc1;
	XID_DOMDocument* doc2;
	const char* doc1xidmap;
	};

#endif
