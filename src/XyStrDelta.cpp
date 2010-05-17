/*
 *  XyStrDeltaApply.cpp
 *  xydiff
 *
 *  Created by Frankie Dintino on 2/13/09.
 *
 */

#include "xercesc/util/PlatformUtils.hpp"


#include "Tools.hpp"
#include "DeltaException.hpp"
#include "include/XyLatinStr.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/dom/DOMImplementationLS.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMText.hpp"
#include "xercesc/dom/DOMTreeWalker.hpp"

#include "xercesc/dom/DOMNodeList.hpp"

#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/util/XMLUniDefs.hpp"
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/sax/SAXException.hpp"
#include "xercesc/sax/SAXParseException.hpp"
#include "include/XID_DOMDocument.hpp"


#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "include/XyStrDelta.hpp"


XERCES_CPP_NAMESPACE_USE 

using namespace XyStrDelta;

XyStrOperationType XyStrDeltaApply::getOperationType(DOMNode *node)
{
	XMLCh xyd_ch[6];
	XMLCh xyi_ch[6];
	XMLString::transcode("xy:d", xyd_ch, 5);
	XMLString::transcode("xy:i", xyi_ch, 5);
	if ( XMLString::equals(node->getNodeName(), xyd_ch) ) {
		return XYDIFF_TXT_DEL;
	}
	else if ( XMLString::equals(node->getNodeName(), xyi_ch) ) {
		return XYDIFF_TXT_INS;
	}
	else {
		return XYDIFF_TXT_NOOP;
	}
}

DOMNodeFilter::FilterAction FilterIfParentIsDelete::acceptNode(const DOMNode *node) const {
	if ( node->getNodeType() == DOMNode::TEXT_NODE ) {
		if ( XMLString::stringLen(node->getNodeValue()) == 0 ) {
			return DOMNodeFilter::FILTER_SKIP;
		}
	}
	XMLCh xyd_ch[6];
	XMLString::transcode("xy:d", xyd_ch, 5);
	if ( XMLString::equals(node->getParentNode()->getNodeName(), xyd_ch) ) {
		return DOMNodeFilter::FILTER_SKIP;
	} else {
		return DOMNodeFilter::FILTER_ACCEPT;
	}
}

XyStrDeltaApply::XyStrDeltaApply(XID_DOMDocument *pDoc, DOMNode *upNode, int changeId)
{
	doc = pDoc;
	node = upNode->getParentNode();

	for (int i = node->getChildNodes()->getLength() - 1; i >= 0; i--) {
		DOMNode *tmpNode = node->getChildNodes()->item(i);
		char *nodeName = XMLString::transcode(tmpNode->getNodeName());
		XMLString::release(&nodeName);
	}
	
	DOMNode *txtNode = node->getFirstChild();

	txt = (DOMText *)txtNode;
  currentValue = upNode->getNodeValue();
	applyAnnotations = false;
	cid = changeId;
}


void XyStrDeltaApply::remove(int startpos, int len, bool isReplaceOperation)
{
	DOMTreeWalker *walker;
	DOMNode *currentNode;
	DOMElement *parentNode;
	
	// Create a DOMTreeWalker out of all text nodes under the parent that
	// aren't children of <xy:d> elements
	DOMNodeFilter *filter = new FilterIfParentIsDelete();
	walker = doc->createTreeWalker(node, DOMNodeFilter::SHOW_TEXT, filter, true);
	
	int curpos = 0;
	int endpos = startpos + len;

	currentNode = walker->firstChild();
	if (currentNode == NULL) return;
	do { // while (currentNode = walker->nextNode())
		const XMLCh *currentNodeValue = currentNode->getNodeValue();
		int currNodeValueStrLen = XMLString::stringLen(currentNodeValue);

		parentNode = (DOMElement *)currentNode->getParentNode();

		// Reached end of the loop
		if (curpos > endpos) {
			break;
		}

		// If we haven't hit the start of the change, keep going.
		if (curpos + currNodeValueStrLen <= startpos) {
			curpos += currNodeValueStrLen;
			continue;
		}

		// Since we're not normalizing the document at every step, empty text
		// nodes will show up. We just remove them and move on.
		if (currNodeValueStrLen == 0) {
			walker->previousNode();
			parentNode->removeChild(currentNode);
			continue;
		}

		int startIndex = intmax(0, startpos - curpos);
		int endIndex   = intmin( currNodeValueStrLen,  endpos - curpos ) ;	
		
		switch( getOperationType(parentNode) ) {
			// If operation type is XYDIFF_TXT_NOOP, it means that the parent node is the original
			// text node, so we simply insert a regular <xy:d> tag around the removed text
			case XYDIFF_TXT_NOOP:
				removeFromNode((DOMText*)currentNode, startIndex, endIndex - startIndex, isReplaceOperation);
			break;

			
			case XYDIFF_TXT_INS:
				// If the entire element needs to be removed
				if (startIndex == 0 && endIndex == currNodeValueStrLen) {
					// This keeps the DOMTreeWalker from going haywire after our changes to the structure
					walker->previousNode();
					node->removeChild(parentNode);
					doc->getXidMap().removeNode(parentNode);     // Remove <xy:i> from xidmap
				// Only a substring of the <xy:i> text needs to be removed
				} else {
					// Anything that had been inserted and then subsequently deleted in another
					// change can just be removed, since we're only interested in annotating
					// text that was changed from the first diffed revision
					XMLCh *replaceString = new XMLCh [startIndex + 1 + currNodeValueStrLen - endIndex];
					XMLCh *endString     = new XMLCh [currNodeValueStrLen - endIndex + 1];
					XMLString::subString(replaceString, currentNodeValue, 0, startIndex);
					XMLString::subString(endString, currentNodeValue, endIndex, currNodeValueStrLen);
					XMLString::catString(replaceString, endString);
					currentNode->setNodeValue( replaceString );
					
					// Free up memory
					XMLString::release(&endString);
					XMLString::release(&replaceString);
				}
			break;
		}
		curpos += currNodeValueStrLen;
	} while (currentNode = walker->nextNode());
	delete filter;
}

void XyStrDeltaApply::removeFromNode(DOMText *removeNode, int pos, int len, bool isReplaceOperation)
{
	if (!applyAnnotations) {
		currentValue.erase(pos, len);
		return;
	}
	DOMText *deletedText;
	DOMText *endText;
	DOMElement *delNode;
	
	endText     = removeNode->splitText(pos+len);
	deletedText = removeNode->splitText(pos);

	node->insertBefore(endText, removeNode->getNextSibling());

	XMLCh tempStrA[6];
	XMLCh tempStrB[6];
	XMLCh xyDeltaNS_ch[36];
	XMLString::transcode("urn:schemas-xydiff:xydelta", xyDeltaNS_ch, 35);

	XMLString::transcode("xy:d", tempStrA, 5);
	delNode = doc->createElementNS(xyDeltaNS_ch, tempStrA);
	XMLString::transcode("cid", tempStrA, 5);
	delNode->setAttribute( tempStrA, witoa(cid) );

	delNode->appendChild(deletedText);
	if (isReplaceOperation) {
		XMLString::transcode("repl", tempStrA, 5);
		XMLString::transcode("yes", tempStrB, 5);
		delNode->setAttribute( tempStrA, tempStrB );
	}
	node->insertBefore(delNode, endText);

	doc->getXidMap().registerNewNode(endText);
	doc->getXidMap().registerNewNode(delNode);
	doc->getXidMap().registerNewNode(deletedText);
}

void XyStrDeltaApply::insert(int startpos, const XMLCh *ins, bool isReplaceOperation)
{
	DOMTreeWalker *walker;
	DOMNode *currentNode;
	DOMElement *parentNode;
	
	// Create a DOMTreeWalker out of all text nodes under the parent that
	// aren't children of <xy:d> elements
	DOMNodeFilter *filter = new FilterIfParentIsDelete();
	walker = doc->createTreeWalker(node, DOMNodeFilter::SHOW_TEXT, filter, true);
	
	int curpos = 0;

	currentNode = walker->firstChild();
	if (currentNode == NULL) return;
	do { // while (currentNode = walker->nextNode())
		const XMLCh *currentNodeValue = currentNode->getNodeValue();
		int currNodeValueStrLen = XMLString::stringLen(currentNodeValue);

		parentNode = (DOMElement *)currentNode->getParentNode();
		
		// Reached end of the loop
		if (curpos > startpos) {
			break;
		}

		// If we haven't hit the start of the insert point, keep going.
		if (curpos + currNodeValueStrLen <= startpos) {
			if (!(startpos == currNodeValueStrLen && curpos == 0)) {
				curpos += currNodeValueStrLen;
				continue;
			}
		}

		// Since we're not normalizing the parent node at every step, empty text
		// nodes will show up. We just ignore them and move on.
		if (currNodeValueStrLen == 0) {
			continue;
		}
		
		int startIndex = intmax(0, startpos - curpos);
		switch( getOperationType(parentNode) ) {
			case XYDIFF_TXT_INS:
				if (startIndex == 0) {
					insertIntoNode(parentNode, startIndex, ins);
				} else {
					// Split into two insert nodes at insertion point
					DOMNode *newInsNode = parentNode->cloneNode(false);
					DOMNode *endCurrentNode = ((DOMText *)currentNode)->splitText(startIndex);
					newInsNode->appendChild( endCurrentNode );
					node->insertBefore( newInsNode, parentNode->getNextSibling() );
					doc->getXidMap().registerNewNode(newInsNode);
					doc->getXidMap().registerNewNode(endCurrentNode);
					insertIntoNode(newInsNode, 0, ins, isReplaceOperation);
				}				
			break;

			case XYDIFF_TXT_NOOP:
				insertIntoNode(currentNode, startIndex, ins, isReplaceOperation);
			break;
		}
		delete filter;
		return;
	} while (currentNode = walker->nextNode());
	
}

void XyStrDeltaApply::insertIntoNode(DOMNode *insertNode, int pos, const XMLCh *ins, bool isReplaceOperation)
{
	if (!applyAnnotations) {
		currentValue.insert(pos, ins);
		return;
	}
	DOMElement *insNode;
	DOMText *insText;
	DOMNode *endText;
	DOMNode *parentNode;
	XMLCh tempStrA[6];
	XMLCh tempStrB[6];
	XMLCh xyDeltaNS_ch[36];
	XMLString::transcode("urn:schemas-xydiff:xydelta", xyDeltaNS_ch, 35);

	parentNode = insertNode->getParentNode();
	XMLString::transcode("xy:i", tempStrA, 5);
	insNode = doc->createElementNS(xyDeltaNS_ch, tempStrA);
	XMLString::transcode("cid", tempStrA, 5);
	insNode->setAttribute( tempStrA, witoa(cid) );

	if (pos == 0) {
		parentNode->insertBefore(insNode, insertNode);
		doc->getXidMap().registerNewNode(insNode);
	} else {
		endText = ((DOMText *)insertNode)->splitText(pos);
		node->insertBefore(insNode, insertNode->getNextSibling());
		node->insertBefore(endText, insNode->getNextSibling());
		doc->getXidMap().registerNewNode(insNode);
		doc->getXidMap().registerNewNode(endText);
	}
	
	insText = doc->createTextNode(ins);
	insNode->appendChild(insText);

	if (isReplaceOperation) {
		XMLString::transcode("repl", tempStrA, 5);
		XMLString::transcode("yes", tempStrB, 5);
		insNode->setAttribute( tempStrA, tempStrB );
	}
	doc->getXidMap().registerNewNode(insText);
}

void XyStrDeltaApply::replace(int pos, int len, const XMLCh *repl)
{
	bool isReplaceOperation = true;
	this->remove(pos, len,  isReplaceOperation);
	this->insert(pos, repl, isReplaceOperation);
}

XyStrDeltaApply::~XyStrDeltaApply()
{
	
}

void XyStrDeltaApply::complete()
{
	if (!applyAnnotations) {
		txt->setNodeValue( currentValue.c_str() ) ;
		return;
	}

	DOMElement *node1, *node2, *node3;
	DOMNodeList *childNodes;
	node->normalize();

	// If a word in the previous document is replaced with another word that shares some characters,
	// we end up with a situation where there are multiple edits in a single word, which can look
	// confusing and isn't particularly helpful. Here we search for replace, insert, or delete operations
	// that surround a text node with no whitespace, and then merge the three into a single replace operation.
	childNodes = node->getChildNodes();
	int i;
	for (i = 0; i < childNodes->getLength(); i++) {
		node1 = (DOMElement *) childNodes->item(i);
		if (node1 == NULL) {
			break;
		}
		node2 = (DOMElement *) node1->getNextSibling();
		if (node2 == NULL) {
			break;
		}
		node3 = (DOMElement *) node2->getNextSibling();
		if (node3 == NULL) {
			break;
		}
		if (textNodeHasWhitespace(node2)) {
			continue;
		}
		if (mergeNodes(node1, node2, node3)) {
			// Move the increment back to retest our new node to see if it
			// can be merged in the same way with the nodes that follow it
			i--;
		}
	}
	
	// Second go-around we see if adjacent operation nodes can be merged (ie ones that don't have
	// any text nodes between them, for instance two adjacent xy:i tags with matching changeIds
	for (i = 0; i < childNodes->getLength(); i++) {
		node1 = (DOMElement *) childNodes->item(i);
		if (node1 == NULL) {
			break;
		}
		node2 = (DOMElement *) node1->getNextSibling();
		if (node2 == NULL) break;
		if (mergeTwoNodes(node1, node2)) {
			// Move the increment back to retest our new node to see if it
			// can be merged in the same way with the nodes that follow it
			i--;
			continue;
		}
	}
}

bool XyStrDeltaApply::mergeTwoNodes(DOMElement *node1, DOMElement *node2)
{
	const XMLCh *cid1, *cid2, *value1, *value2;
	XMLCh *newNodeValue;

	if (node1 == NULL || node2 == NULL) return false;
	if (node1->getNodeType() != DOMNode::ELEMENT_NODE) return false;
	if (node2->getNodeType() != DOMNode::ELEMENT_NODE) return false;

	XMLCh cid_attr_name_xmlch[5];
	XMLString::transcode("cid", cid_attr_name_xmlch, 4);
	cid1 = node1->getAttribute( cid_attr_name_xmlch );
	cid2 = node2->getAttribute( cid_attr_name_xmlch );
	
	if (cid1 == NULL || cid2 == NULL) {
		return false;
	}
	if (!XMLString::equals(cid1, cid2)) {
		return false;
	}
	if (!XMLString::equals(node1->getNodeName(), node2->getNodeName())) {
		return false;
	}

	value1 = node1->getFirstChild()->getNodeValue();
	value2 = node2->getFirstChild()->getNodeValue();

	int textSize = XMLString::stringLen(value1) + XMLString::stringLen(value2);

	std::vector<XMLCh> nodeValue_vec(textSize+1);
	XMLString::copyString(&nodeValue_vec[0], value1);
	newNodeValue = &nodeValue_vec[0];
	XMLString::catString(newNodeValue, value2);

	node1->getFirstChild()->setNodeValue(newNodeValue);
	node->removeChild(node2);

	// Remove nodes from xidmap
	doc->getXidMap().removeNode(node2);
	// Free up allocated memory
//	XMLString::release(&newNodeValue);
	return true;
}

bool XyStrDeltaApply::textNodeHasWhitespace(DOMNode *t)
{
  std::basic_string<XMLCh> nodeText;
  std::basic_string<XMLCh> findText;
  XMLCh* findText_ch;
  bool ret;

	// Make sure we're dealing with a text node
	if (t->getNodeType() != DOMNode::TEXT_NODE) {
		return false;
	}

  nodeText = std::basic_string<XMLCh>(t->getNodeValue());
  findText_ch = XMLString::transcode("		\n");
  findText = std::basic_string<XMLCh>(findText_ch);
	ret = (nodeText.find(findText) != std::string::npos);
  XMLString::release(&findText_ch);
  return ret;
}

bool XyStrDeltaApply::mergeNodes(DOMElement *node1, DOMElement *node2, DOMElement *node3)
{
	DOMNode *parent;
	DOMElement *delNode, *insNode;
	DOMText *insTextNode, *delTextNode;
	std::string instext, deltext;
	const XMLCh *cid1, *cid3;

	if (node1->getNodeType() != DOMNode::ELEMENT_NODE || node3->getNodeType() != DOMNode::ELEMENT_NODE) {
		return false;
	}

	XMLCh cid_attr_name_xmlch[5];
	XMLString::transcode("cid", cid_attr_name_xmlch, 4);
	cid1 = node1->getAttribute( cid_attr_name_xmlch );
	cid3 = node3->getAttribute( cid_attr_name_xmlch );
	
	// If the nodes are not from the same edit, don't merge them
	if (!XMLString::equals(cid1, cid3)) {
		return false;
	}
	char *node1_value = XMLString::transcode( node1->getFirstChild()->getNodeValue() );
	char *node2_value = XMLString::transcode( node2->getNodeValue() );
	char *node3_value = XMLString::transcode( node3->getFirstChild()->getNodeValue() );

	switch ( getOperationType(node1) ) {
		case XYDIFF_TXT_DEL:
			deltext += node1_value;
			break;
		case XYDIFF_TXT_INS:
			instext += node1_value;
			break;
	}

	instext += node2_value;
	deltext += node2_value;
	
	
	switch ( getOperationType(node3) ) {
		case XYDIFF_TXT_DEL:
			deltext += node3_value;
			break;
		case XYDIFF_TXT_INS:
			instext += node3_value;
			break;
	}

	XMLString::release(&node1_value);
	XMLString::release(&node2_value);
	XMLString::release(&node3_value);

	parent = node1->getParentNode();

	bool isReplaceOperation;

	XMLCh tempStrA[6];
	XMLCh tempStrB[6];
	XMLCh xyDeltaNS_ch[36];
	XMLString::transcode("urn:schemas-xydiff:xydelta", xyDeltaNS_ch, 35);

	XMLString::transcode("repl", tempStrA, 5);
	const XMLCh *repl1 = node1->getAttribute( tempStrA );
	const XMLCh *repl3 = node3->getAttribute( tempStrA );
	isReplaceOperation = (XMLString::stringLen(repl1) > 0 || XMLString::stringLen(repl3) > 0);

	XMLString::transcode("xy:d", tempStrA, 5);
	
	delNode = doc->createElementNS(xyDeltaNS_ch, tempStrA);
	parent->insertBefore(delNode, node1);
	XMLString::transcode("cid", tempStrA, 5);
	delNode->setAttribute( tempStrA, cid1 );	

	XMLString::transcode("xy:i", tempStrA, 5);
	insNode = doc->createElementNS(xyDeltaNS_ch, tempStrA);
	parent->insertBefore(insNode, node1);

	XMLString::transcode("cid", tempStrA, 5);
	insNode->setAttribute( tempStrA, cid1 );

	XMLCh *instext_xmlch = XMLString::transcode(instext.c_str());
	XMLCh *deltext_xmlch = XMLString::transcode(deltext.c_str());
	insTextNode = doc->createTextNode( instext_xmlch );
	insNode->appendChild(insTextNode);
	delTextNode = doc->createTextNode( deltext_xmlch );
	delNode->appendChild(delTextNode);
	XMLString::release(&instext_xmlch);
	XMLString::release(&deltext_xmlch);

	if (isReplaceOperation) {
		XMLString::transcode("repl", tempStrA, 5);
		XMLString::transcode("yes", tempStrB, 5);
		insNode->setAttribute(tempStrA, tempStrB);
		delNode->setAttribute(tempStrA, tempStrB);
	}

	parent->removeChild(node1);
	parent->removeChild(node2);
	parent->removeChild(node3);

	// Update nodes in xidmap
	doc->getXidMap().registerNewNode(insNode);
	doc->getXidMap().registerNewNode(delNode);
	doc->getXidMap().registerNewNode(insTextNode);
	doc->getXidMap().registerNewNode(delTextNode);
	doc->getXidMap().removeNode(node1);
	doc->getXidMap().removeNode(node2);
	doc->getXidMap().removeNode(node3);
	return true;
}

void XyStrDeltaApply::setApplyAnnotations(bool paramApplyAnnotations)
{
	applyAnnotations = paramApplyAnnotations;
}

bool XyStrDeltaApply::getApplyAnnotations()
{
	return applyAnnotations;
}
