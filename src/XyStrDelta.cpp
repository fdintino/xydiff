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
	if ( XMLString::equals(node->getNodeName(),
						   XMLString::transcode("xy:d")) ) {
		if ( XMLString::equals(node->getParentNode()->getNodeName(),
							   XMLString::transcode("xy:r")) ) {
			return XYDIFF_TXT_REPL_DEL;
		} else {
			return XYDIFF_TXT_DEL;
		}
	}
	else if ( XMLString::equals(node->getNodeName(),
								XMLString::transcode("xy:i")) ) {
		if ( XMLString::equals(node->getParentNode()->getNodeName(),
							   XMLString::transcode("xy:r")) ) {
			return XYDIFF_TXT_REPL_INS;
		} else {
			return XYDIFF_TXT_INS;
		}
	}
	else if ( XMLString::equals(node->getNodeName(),
								XMLString::transcode("xy:r")) ) {
		return XYDIFF_TXT_REPL;
	}
	else {
		return XYDIFF_TXT_NOOP;
	}
}


DOMNodeFilter::FilterAction FilterIfParentIsDelete::acceptNode(const DOMNode *node) const {
	if ( XMLString::equals(
						   node->getParentNode()->getNodeName(),
						   XMLString::transcode("xy:d")) ) {
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
	currentValue = XMLString::transcode(upNode->getNodeValue());
	applyAnnotations = false;
	cid = changeId;
}


void XyStrDeltaApply::remove(int startpos, int len)
{
	DOMTreeWalker *walker;
	DOMNode *currentNode;
	DOMElement *parentNode, *delNode, *replNode;
	
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



		int startIndex = max(0, startpos - curpos);
		int endIndex   = min( currNodeValueStrLen,  endpos - curpos ) ;	

		// If operation type is XYDIFF_TXT_NOOP, it means that the parent node is the original
		// text node, so we simply insert a regular <xy:d> tag around the removed text
		if ( getOperationType(parentNode) == XYDIFF_TXT_NOOP ) {
			removeFromNode((DOMText*)currentNode, startIndex, endIndex - startIndex);
			curpos += currNodeValueStrLen;
			continue;
		}

		// If the entire element needs to be removed
		if (startIndex == 0 && endIndex == currNodeValueStrLen) {
			// This keeps the DOMTreeWalker from going haywire after our changes to the structure
			walker->previousNode();
			
			switch( getOperationType(parentNode) ) {
				// If under a <xy:r> tag, we need to move the <xy:d> tag up one level and delete the <xy:r>
				case XYDIFF_TXT_REPL_INS:
					replNode = (DOMElement *) parentNode->getParentNode();
					// We move the <xy:d> element up one level, then delete the <xy:r> node
					delNode = (DOMElement *) replNode->removeChild( replNode->getFirstChild() );
					node->insertBefore(delNode, replNode);
					node->removeChild(replNode);
					doc->getXidMap().removeNode(replNode);
				break;
					
				// <xy:i> tag by itself, we can just remove it
				case XYDIFF_TXT_INS:
					node->removeChild(parentNode);
					doc->getXidMap().removeNode(parentNode);     // Remove <xy:i> from xidmap
				break;
			}
		
		} else {
			switch( getOperationType(parentNode) ) {
				// Only a substring of the <xy:i> text needs to be removed
				case XYDIFF_TXT_REPL_INS:
				case XYDIFF_TXT_INS:
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
				break;
			}

		}
		curpos += currNodeValueStrLen;
	} while (currentNode = walker->nextNode());
}

void XyStrDeltaApply::removeFromNode(DOMText *removeNode, int pos, int len)
{
	if (!applyAnnotations) {
		currentValue.erase(pos, len);
		return;
	}
	DOMText *deletedText;
	DOMText *endText;
	DOMNode *nextNode;
	
	endText = removeNode->splitText(pos+len);
	if (pos == 0) {
		nextNode = removeNode->getNextSibling();
		deletedText = (DOMText *)node->removeChild((DOMNode *)removeNode);
		doc->getXidMap().removeNode(removeNode);
	} else {
		deletedText = removeNode->splitText(pos);
		nextNode = removeNode->getNextSibling();
	}
	
	if (nextNode == NULL) {
		node->appendChild(endText);
	} else {
		node->insertBefore((DOMNode *)endText, nextNode);
	}
	
	doc->getXidMap().registerNode(endText, doc->getXidMap().allocateNewXID());
	
	DOMElement *delNode = doc->createElement(XMLString::transcode("xy:d"));
	delNode->setAttribute(XMLString::transcode("cid"),
						  XMLString::transcode( itoa(this->cid).c_str() ) );
	node->insertBefore(delNode, endText);
	
	doc->getXidMap().registerNode(delNode, doc->getXidMap().allocateNewXID());
	
	delNode->appendChild(deletedText);
	doc->getXidMap().registerNode(deletedText, doc->getXidMap().allocateNewXID());
}

void XyStrDeltaApply::insert(int startpos, const XMLCh *ins)
{
	int len = 0;
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
		printf("text=%s, currNodeValueStrLen=%d, curpos=%d, endpos=%d, startpos=%d, parent=%s\n",
			   XMLString::transcode(currentNodeValue),
			   currNodeValueStrLen, curpos, endpos, startpos,
			   XMLString::transcode(parentNode->getNodeName()));
		
		// Reached end of the loop
		if (curpos > endpos) {
			break;
		}
		// If we haven't hit the start of the change, keep going.
		if (curpos + currNodeValueStrLen <= startpos) {
			printf("curpos+currNodeValueStrLen <= startpos\n");
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
		
		int startIndex = max(0, startpos - curpos);
		switch( getOperationType(parentNode) ) {
			case XYDIFF_TXT_REPL_INS:
				// If under a <xy:r> tag, we need to move the <xy:d> tag up one level and delete the <xy:r>
				if (startIndex == 0) {
					insertIntoNode(parentNode->getParentNode(), startIndex, ins);
				} else {
					// Split into two insert nodes at insertion point
					DOMNode *newInsNode = parentNode->cloneNode(false);
					DOMNode *endCurrentNode = ((DOMText *)currentNode)->splitText(startIndex);
					parentNode->removeChild(endCurrentNode);
					newInsNode->appendChild(endCurrentNode);
				}
				// Only a substring of the <xy:i> text needs to be removed				
			break;

			case XYDIFF_TXT_INS:
				if (startIndex == 0) {
					insertIntoNode(parentNode, startIndex, ins);
				} else {
					std::cout << "ELSE\n" << std::endl;
				}				
			break;

			case XYDIFF_TXT_NOOP:
				printf("insertIntoNode(currentNode, %d, %d)\n", startIndex, ins);
				insertIntoNode(currentNode, startIndex, ins);
			break;
		}
		curpos += currNodeValueStrLen;
	} while (currentNode = walker->nextNode());
}

void XyStrDeltaApply::insertIntoNode(DOMNode *insertNode, int pos, const XMLCh *ins)
{
	if (!applyAnnotations) {
		std::string insString( XMLString::transcode(ins) );
		currentValue.insert(pos, insString);
		return;
	}
	DOMNode *insNode;
	DOMText *insText;
	DOMNode *endText;
	
	insNode = doc->createElement(XMLString::transcode("xy:i"));
	XMLCh *changeIdAttr = XMLString::transcode( itoa(this->cid).c_str() );
	((DOMElement *)insNode)->setAttribute( XMLString::transcode("cid"), changeIdAttr );
	
	if (pos == 0) {
		node->insertBefore(insNode, insertNode);
		printf("inserting before insertNode=%s with content=%s\n", XMLString::transcode(insertNode->getNodeName()), XMLString::transcode(ins));
		doc->getXidMap().registerNode(insNode, doc->getXidMap().allocateNewXID());
	} else {
		endText = ((DOMText *)insertNode)->splitText(pos);
		node->insertBefore(insNode, insertNode->getNextSibling());
		doc->getXidMap().registerNode(insNode, doc->getXidMap().allocateNewXID());
		node->insertBefore(endText, insNode->getNextSibling());
		doc->getXidMap().registerNode(endText, doc->getXidMap().allocateNewXID());
	}
	
	insText = doc->createTextNode(ins);
	insNode->appendChild(insText);
	doc->getXidMap().registerNode(insText, doc->getXidMap().allocateNewXID());
}

void XyStrDeltaApply::replace(int pos, int len, const XMLCh *repl)
{
	if (!applyAnnotations) {
		std::string replString( XMLString::transcode(repl) );
		currentValue.replace(pos, len, replString);
		return;
	}
	
	DOMText *replacedText;
	DOMText *endText;
	DOMNode *nextNode;

	vddprintf(("pos=%d, len=%d, repl=(%s)\n", pos, len, XMLString::transcode(repl)));
	endText = txt->splitText(pos+len);
	
	replacedText = txt->splitText(pos);
	nextNode = txt->getNextSibling();
	
	
	DOMElement *replNode = doc->createElement(XMLString::transcode("xy:r"));
	
	replNode->setAttribute(XMLString::transcode("cid"),
						   witoa( this->cid) );
	
	node->insertBefore((DOMNode *)endText, nextNode);
	doc->getXidMap().registerNode(endText, doc->getXidMap().allocateNewXID());
	//node->insertBefore(replNode, endText);
	
	doc->getXidMap().registerNode(replNode, doc->getXidMap().allocateNewXID());
	

	DOMElement *delNode = doc->createElement(XMLString::transcode("xy:d"));
	delNode->setAttribute(XMLString::transcode("repl"),
						  XMLString::transcode("yes") );
	//replNode->appendChild(delNode);
	node->insertBefore(delNode, endText);
	//
	delNode->setAttribute(XMLString::transcode("cid"), witoa( this->cid) );
	
	doc->getXidMap().registerNode(delNode, doc->getXidMap().allocateNewXID());
	
	
	DOMElement *insNode = doc->createElement(XMLString::transcode("xy:i"));
	
	//replNode->appendChild(insNode);
	node->insertBefore(insNode, delNode);
	//
	insNode->setAttribute(XMLString::transcode("cid"),
						  witoa( this->cid) );
	insNode->setAttribute(XMLString::transcode("repl"),
						  XMLString::transcode("yes") );
	doc->getXidMap().registerNode(insNode, doc->getXidMap().allocateNewXID());
	
	
	DOMText *replText = doc->createTextNode(repl);
	insNode->appendChild(replText);
	doc->getXidMap().registerNode(replText, doc->getXidMap().allocateNewXID());
	
	delNode->appendChild(replacedText);
	doc->getXidMap().registerNode(replacedText, doc->getXidMap().allocateNewXID());
	
	if (pos != 0) {
		txt = (DOMText *) node->getFirstChild();
	}
}

XyStrDeltaApply::~XyStrDeltaApply()
{
	
}

void XyStrDeltaApply::complete()
{
	if (!applyAnnotations) {
		txt->setNodeValue( XMLString::transcode(currentValue.c_str()) ) ;
		return;
	}
	// If a word in the previous document is replaced with another word that shares some characters,
	// we end up with a situation where there are multiple edits in a single word, which can look
	// confusing and isn't particularly helpful. Here we search for replace, insert, or delete operations
	// that surround a text node with no whitespace, and then merge the three into a single replace operation.
	DOMNodeList *childNodes = node->getChildNodes();
	
	for (int i = 0; i < childNodes->getLength(); i++) {
		DOMNode *node1 = childNodes->item(i);
		if (node1 == NULL) break;
		if (node1->getNodeType() == DOMNode::ELEMENT_NODE) {
			DOMNode *node2 = node1->getNextSibling();
			if (node2 == NULL) break;
			DOMNode *node3 = node2->getNextSibling();
			if (node3 == NULL) break;
			if (node3->getNodeType() != DOMNode::ELEMENT_NODE) {
				continue;
			}
			if (node2->getNodeType() != DOMNode::ELEMENT_NODE && !textNodeHasNoWhitespace((DOMText *)node2)) {
				continue;
			}
			if (mergeNodes(node1, node2, node3)) {
				// Move the increment back to retest our new node to see if it
				// can be merged in the same way with the nodes that follow it
				i--;
			}
		}
	}

	// Second go-around we see if adjacent operation nodes can be merged (ie ones that don't have
	// any text nodes between them, for instance two adjacent xy:i tags with matching changeIds
	for (int i = 0; i < childNodes->getLength(); i++) {
		DOMNode *node1 = childNodes->item(i);
		if (node1 == NULL) break;
		if (node1->getNodeType() == DOMNode::ELEMENT_NODE) {
			DOMNode *node2 = node1->getNextSibling();
			if (node2 == NULL) break;
			if (mergeTwoNodes((DOMElement *)node1, (DOMElement *)node2)) {
				// Move the increment back to retest our new node to see if it
				// can be merged in the same way with the nodes that follow it
				i--;
				continue;
			}
		}
	}
	
}

bool XyStrDeltaApply::mergeTwoNodes(DOMElement *node1, DOMElement *node2)
{
	if (node1 == NULL || node2 == NULL) return false;
	if (node1->getNodeType() != DOMNode::ELEMENT_NODE) return false;
	if (node2->getNodeType() != DOMNode::ELEMENT_NODE) return false;

	const XMLCh *cid1 = node1->getAttribute( XMLString::transcode("cid") );
	const XMLCh *cid2 = node2->getAttribute( XMLString::transcode("cid") );
	
	if (cid1 == NULL || cid2 == NULL) {
		return false;
	}
	if (!XMLString::equals(cid1, cid2)) {
		return false;
	}
	if (!XMLString::equals(node1->getNodeName(), node2->getNodeName())) {
		return false;
	}
	int textSize = XMLString::stringLen(node1->getFirstChild()->getNodeValue())
					+ XMLString::stringLen(node1->getFirstChild()->getNodeValue());
	std::vector<XMLCh> nodeValue_vec(textSize+1);
	XMLString::copyString(&nodeValue_vec[0], node1->getFirstChild()->getNodeValue());
	XMLCh *nodeValue = &nodeValue_vec[0];
	XMLString::catString(nodeValue,
						 node2->getFirstChild()->getNodeValue());
	node1->getFirstChild()->setNodeValue(&nodeValue[0]);
	node->removeChild(node2);
	doc->getXidMap().removeNode(node2);
	return true;
}

bool XyStrDeltaApply::textNodeHasNoWhitespace(DOMText *t)
{
	// Make sure we're dealing with a text node
	if (((DOMNode *)t)->getNodeType() != DOMNode::TEXT_NODE) {
		return false;
	}
	std::string nodeText = std::string ( XMLString::transcode(t->getNodeValue()) );
	return (nodeText.find("		\n") == std::string::npos);
}

bool XyStrDeltaApply::mergeNodes(DOMNode *node1, DOMNode *node2, DOMNode *node3)
{
	DOMNode *parent;
	DOMElement *delNode, *insNode;
	DOMText *insTextNode, *delTextNode;
	std::string instext, deltext;
	
	const XMLCh *cid1 = ((DOMElement *)node1)->getAttribute(XMLString::transcode("cid"));
	const XMLCh *cid3 = ((DOMElement *)node3)->getAttribute(XMLString::transcode("cid"));
	
	// If the nodes are not from the same edit, don't merge them
	if (!XMLString::equals(cid1, cid3)) {
		return false;
	}
	// If second node is an operation node, check that it has a matching changeId (cid)
	if (node2->getNodeType() == DOMNode::ELEMENT_NODE) {
		return false;
	}
	switch ( getOperationType(node1) ) {
		case XYDIFF_TXT_REPL:
			if (!node1->hasChildNodes()) return false;
			instext += XMLString::transcode( node1->getChildNodes()->item(0)->getFirstChild()->getNodeValue() );
			deltext += XMLString::transcode( node1->getChildNodes()->item(1)->getFirstChild()->getNodeValue() );
			break;
		case XYDIFF_TXT_DEL:
			deltext += XMLString::transcode( node1->getFirstChild()->getNodeValue() );
			break;
		case XYDIFF_TXT_INS:
			instext += XMLString::transcode( node1->getFirstChild()->getNodeValue() );
			break;
	}
	instext += XMLString::transcode( node2->getNodeValue() );
	deltext += XMLString::transcode( node2->getNodeValue() );
	switch ( getOperationType(node3) ) {
		case XYDIFF_TXT_REPL:
			if (!node3->hasChildNodes()) return false;
			instext += XMLString::transcode( node3->getChildNodes()->item(0)->getFirstChild()->getNodeValue() );
			deltext += XMLString::transcode( node3->getChildNodes()->item(1)->getFirstChild()->getNodeValue() );
			break;
		case XYDIFF_TXT_DEL:
			deltext += XMLString::transcode( node3->getFirstChild()->getNodeValue() );
			break;
		case XYDIFF_TXT_INS:
			instext += XMLString::transcode( node3->getFirstChild()->getNodeValue() );
			break;
	}

	parent = node1->getParentNode();
	
	insNode = doc->createElement(XMLString::transcode("xy:i"));
	parent->insertBefore(insNode, node1);
	insNode->setAttribute( XMLString::transcode("cid"), cid1 );

	delNode = doc->createElement(XMLString::transcode("xy:d"));
	parent->insertBefore(delNode, node1);
	delNode->setAttribute( XMLString::transcode("cid"), cid1 );	


	insTextNode = doc->createTextNode( XMLString::transcode(instext.c_str()) );
	insNode->appendChild(insTextNode);

	delTextNode = doc->createTextNode( XMLString::transcode(deltext.c_str()) );
	delNode->appendChild(delTextNode);

	doc->getXidMap().registerNode(insNode, doc->getXidMap().allocateNewXID());
	doc->getXidMap().registerNode(delNode, doc->getXidMap().allocateNewXID());
	doc->getXidMap().registerNode(insTextNode, doc->getXidMap().allocateNewXID());
	doc->getXidMap().registerNode(delTextNode, doc->getXidMap().allocateNewXID());
	doc->getXidMap().removeNode(node1);
	doc->getXidMap().removeNode(node2);
	doc->getXidMap().removeNode(node3);
	parent->removeChild(node1);
	parent->removeChild(node2);
	parent->removeChild(node3);
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
