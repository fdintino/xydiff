/*
 *  XyStrDeltaApply.hpp
 *  xydiff
 *
 *  Created by Frankie Dintino on 2/13/09.
 *
 */

#ifndef XyStrDeltaApply_HXX__
#define XyStrDeltaApply_HXX__

/* Class to do diffs on strings character-by-character */

#include "include/XyStr.hpp"
#include "include/XID_DOMDocument.hpp"

// #include "xercesc/dom/deprecated/DOMString.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"

#include <map>

namespace XyStrDelta {

	enum XyStrOperationType {
		XYDIFF_TXT_NOOP     = 0,
		XYDIFF_TXT_INS      = 1,
		XYDIFF_TXT_DEL      = 2,
		XYDIFF_TXT_REPL     = 3,
		XYDIFF_TXT_REPL_INS = 4,
		XYDIFF_TXT_REPL_DEL = 5
	};
};

typedef std::vector<xercesc::DOMNode *> domnode_vec_t;

class FilterIfParentIsDelete : public xercesc::DOMNodeFilter {
public:
	FilterIfParentIsDelete(bool reject=false) : xercesc::DOMNodeFilter(), fReject(reject) {};
	xercesc::DOMNodeFilter::FilterAction acceptNode(const xercesc::DOMNode *node) const;
private:
	bool fReject;
};

class XyStrDeltaApply {
public:
	XyStrDeltaApply(XID_DOMDocument *pDoc, xercesc::DOMNode *upNode, int changeId=0);
	~XyStrDeltaApply();
	void removeFromNode(xercesc::DOMText *removeNode, XMLSize_t pos, XMLSize_t len, bool isReplaceOperation=false);
	void remove(XMLSize_t startpos, XMLSize_t len, bool isReplaceOperation=false);
	void insert(XMLSize_t startpos, const XMLCh *ins, bool isReplaceOperation=false);
	void insertIntoNode(xercesc::DOMNode *insertNode, XMLSize_t pos, const XMLCh *ins, bool isReplaceOperation=false);
	void replaceFromNode(xercesc::DOMText *replacedNode, XMLSize_t pos, XMLSize_t len, const XMLCh *repl);
	void replace(XMLSize_t pos, XMLSize_t len, const XMLCh *repl);
	void complete();
	void setApplyAnnotations(bool paramApplyAnnotations);
	bool getApplyAnnotations();
	XyStrDelta::XyStrOperationType getOperationType(xercesc::DOMNode *node);

private :
	XID_DOMDocument *doc;
	xercesc::DOMNode *node;
	xercesc::DOMText *txt;
	std::basic_string<XMLCh> currentValue;
	bool applyAnnotations;
	bool textNodeHasWhitespace(xercesc::DOMNode *t);
	bool mergeNodes(xercesc::DOMElement *node1, xercesc::DOMElement *node2, xercesc::DOMElement *node3);
	bool mergeTwoNodes(xercesc::DOMElement *node1, xercesc::DOMElement *node2);
	int cid;
	domnode_vec_t removedNodeVector;
	domnode_vec_t addedNodeVector;
};

#endif