/*
 *  StringDiff.cpp
 *  xydiff
 *
 *  Created by Frankie Dintino on 2/3/09.
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

#include "xercesc/dom/DOMNodeList.hpp"

#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/util/XMLUniDefs.hpp"
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/sax/SAXException.hpp"
#include "xercesc/sax/SAXParseException.hpp"
#include "include/XID_DOMDocument.hpp"

// For output



#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "include/XyStrDiff.hpp"

/*
 * XyStrDiff functions (character-by-character string diffs)
 */

XERCES_CPP_NAMESPACE_USE 



static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

XyStrDiff::XyStrDiff(DOMDocument *myDoc, DOMElement *elem, const char* strX, const char *strY, int sizeXStr, int sizeYStr)
{	
	doc = myDoc;
	root = elem;
	sizex = sizeXStr;
	sizey = sizeYStr;
	
	if ((strX==NULL)||(sizex==0)) return;
	if (sizex<0) sizex = strlen(strX);
	x = new char[sizex+1];
	memcpy(x, strX, sizex*sizeof(char));
	x[sizex]='\0';
	
	if ((strY==NULL)||(sizey==0)) return;
	if (sizey<0) sizey = strlen(strY);
	y = new char[sizey+1];
	memcpy(y, strY, sizey*sizeof(char));
	y[sizey]='\0';
	
	n = sizex;
	m = sizey;

	int malloclen = (sizeof(int))*(sizex+1)*(sizey+1);
	// c = LCS Length matrix
	c = (int*) malloc(malloclen);
	// d = Levenshtein Distance matrix
	d = (int*) malloc(malloclen);
	t = (int*) malloc(malloclen);
	
	currop = -1;

}

/*
 * Destructor
 */

XyStrDiff::~XyStrDiff(void)
{
	free(t);
	free(c);
	free(d);
	delete [] x;
	delete [] y;
}

void XyStrDiff::LevenshteinDistance()
{
	// Step 1
	int k, i, j, cost, distance;
	n = strlen(x);
	m = strlen(y);

	if (n != 0 && m != 0) {
		m++;
		n++;
		// Step 2
		for(k = 0; k < n; k++) {
			c[k] = 0;
			d[k] = k;
		}
		for(k = 0; k < m; k++) {
			c[k*n] = 0;
			d[k*n] = k;
		}
		
		// Step 3 and 4	
		for(i = 1; i < n; i++) {
			for(j = 1; j < m; j++) {
				// Step 5
				if (x[i-1] == y[j-1]) {
					cost = 0;
					c[j*n+i] = c[(j-1)*n + i-1] + 1;
				} else {
					cost = 1;
					c[j*n+i] = max(c[(j-1)*n + i], c[j*n + i-1]);
				}
				// Step 6
				int del = d[j*n+i-1] + 1;
				int ins = d[(j-1)*n+i] + 1;
				int sub = d[(j-1)*n+i-1] + cost;
				if (sub <= del && sub <= ins) {
					d[j*n+i] = sub;
					t[j*n+i] = STRDIFF_SUB;
				} else if (del <= ins) {
					d[j*n+i] = del;
					t[j*n+i] = STRDIFF_DEL;
				} else {
					d[j*n+i] = ins;
					t[j*n+i] = STRDIFF_INS;
				}
			}
		}
		distance = d[n*m-1];
		this->calculatePath();
		this->flushBuffers();
		
		vddprintf(("debugstr=%s\n", debugstr.c_str()));
	}
	//return root;
}

void XyStrDiff::calculatePath(int i, int j)
{
	if (i == -1) i = sizex;
	if (j == -1) j = sizey;
	
	if (i > 0 && j > 0 && (x[i-1] == y[j-1])) {
		this->calculatePath(i-1, j-1);
		this->registerBuffer(i, j, STRDIFF_NOOP, x[i-1]);
	} else {
		if (j > 0 && (i == 0 || c[(j-1)*n+i] >= c[j*n+i-1])) {
			this->calculatePath(i, j-1);
			this->registerBuffer(i, j, STRDIFF_INS, y[j-1]);
		} else if (i > 0 && (j == 0 || c[(j-1)*n+i] < c[j*n+i-1])) {
			this->calculatePath(i-1, j);
			this->registerBuffer(i, j, STRDIFF_DEL, x[i-1]);
		}
	}
}

void XyStrDiff::registerBuffer(int i, int j, int optype, char chr)
{
	if (wordbuf.empty()) {
		wordbuf = chr;
	}
	xpos = i;
	ypos = j;
	if (currop == -1) {
		currop = optype;

	} 
	if (currop == STRDIFF_SUB) {
		if (optype == STRDIFF_DEL) {
			delbuf += chr;
		} else if (optype == STRDIFF_INS) {
			insbuf += chr;
		} else {
			this->flushBuffers();
			currop = optype;
		}
	}
	else if (optype == STRDIFF_DEL) {
		currop = optype;
		delbuf += chr;
	}
	else if (optype == STRDIFF_INS) {
		currop = (currop == STRDIFF_DEL) ? STRDIFF_SUB : STRDIFF_INS;
		insbuf += chr;
	}
	else if (optype == STRDIFF_NOOP) {
		this->flushBuffers();
		currop = optype;
	}
}

void XyStrDiff::flushBuffers()
{
	int startpos, len;
	if (currop == STRDIFF_NOOP) {
		return;
	} else if (currop == STRDIFF_SUB) {
		startpos = xpos - delbuf.length() - 1;
		len = delbuf.length();
		debugstr.append("<tr pos=\"" + itoa(startpos) + "\" len=\"" + itoa(len) + "\">" + insbuf + "</r>\n");
		
		try {
			DOMElement *r = doc->createElement(XMLString::transcode("tr"));
			r->setAttribute(XMLString::transcode("pos"), XMLString::transcode((itoa(startpos)).c_str()));
			r->setAttribute(XMLString::transcode("len"), XMLString::transcode((itoa(len)).c_str()));		
			DOMText *textNode = doc->createTextNode(XMLString::transcode(insbuf.c_str()));
			r->appendChild((DOMNode *)textNode);
			root->appendChild((DOMNode *)r);
		}
		catch (const XMLException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (const DOMException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (...) {
			std::cout << "Unexpected Exception" << std::endl;
		}
		
		delbuf = "";
		insbuf = "";
	} else if (currop == STRDIFF_INS) {
		startpos = xpos - 1;
		debugstr.append("<ti pos=\"" + itoa(startpos) + "\">"+insbuf+"</i>\n");
		
		try {
			DOMElement *r = doc->createElement(XMLString::transcode("ti"));
			r->setAttribute(XMLString::transcode("pos"), XMLString::transcode((itoa(startpos)).c_str()));
			DOMText *textNode = doc->createTextNode(XMLString::transcode(insbuf.c_str()));
			r->appendChild((DOMNode *)textNode);
			root->appendChild((DOMNode *)r);
		}
		catch (const XMLException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (const DOMException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (...) {
			std::cout << "Unexpected Exception" << std::endl;
		}
		
		
		insbuf = "";
	} else if (currop == STRDIFF_DEL) {
		startpos = xpos - delbuf.length() - 1;
		len = delbuf.length();
		debugstr.append("<td pos=\"" + itoa(startpos) + "\" len=\"" + itoa(len) + "\" />\n");
		try {
			DOMElement *r = doc->createElement(XMLString::transcode("td"));
			r->setAttribute(XMLString::transcode("pos"), XMLString::transcode((itoa(startpos)).c_str()));
			r->setAttribute(XMLString::transcode("len"), XMLString::transcode((itoa(len)).c_str()));		
			root->appendChild((DOMNode *)r);
		}
		catch (const XMLException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (const DOMException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (...) {
			std::cout << "Unexpected Exception" << std::endl;
		}
		delbuf = "";
	}
}


XyDOMText::XyDOMText(XID_DOMDocument *pDoc, DOMNode *upNode, bool applyAnnotations)
{
	doc = pDoc;
	node = upNode->getParentNode();

	for (int i = node->getChildNodes()->getLength() - 1; i >= 0; i--) {
		DOMNode *tmpNode = node->getChildNodes()->item(i);
		char *nodeName = XMLString::transcode(tmpNode->getNodeName());
		XMLString::release(&nodeName);
	}
	if (node->getChildNodes()->getLength() != 1) {
		THROW_AWAY(("XyDOMText must be passed elements with only one DOMText node"));
	}
	DOMNode *txtNode = node->getFirstChild();
	
	if (txtNode->getNodeType() != DOMNode::TEXT_NODE) {
		THROW_AWAY(("XyDOMText must be passed elements with only one DOMText node"));
	}
	txt = (DOMText *)txtNode;
	currentValue = XMLString::transcode(upNode->getNodeValue());
}

void XyDOMText::remove(int pos, int len)
{
	if (!applyAnnotations) {
		currentValue.erase(pos, len);
		return;
	}
	DOMText *deletedText;
	DOMText *endText;
	DOMNode *nextNode;

	endText = txt->splitText(pos+len);
	if (pos == 0) {
		deletedText = (DOMText *)node->removeChild((DOMNode *)txt);
		doc->getXidMap().removeNode(txt);
		nextNode = node->getFirstChild();
	} else {
		deletedText = txt->splitText(pos);
		nextNode = txt->getNextSibling();
	}

	node->insertBefore((DOMNode *)endText, nextNode);
	doc->getXidMap().registerNode(endText, doc->getXidMap().allocateNewXID());
	
	DOMNode *delNode = doc->createElement(XMLString::transcode("del"));
	node->insertBefore(delNode, endText);
	doc->getXidMap().registerNode(delNode, doc->getXidMap().allocateNewXID());
	
	delNode->appendChild(deletedText);
	doc->getXidMap().registerNode(deletedText, doc->getXidMap().allocateNewXID());
}

void XyDOMText::insert(int pos, const XMLCh *ins)
{
	if (!applyAnnotations) {
		std::string insString( XMLString::transcode(ins) );
		currentValue.insert(pos, insString);
		return;
	}
	DOMNode *insNode;
	DOMText *insText;
	DOMNode *endText;
	
	insNode = doc->createElement(XMLString::transcode("ins"));
	
	
	if (pos == 0) {
		node->insertBefore(insNode, txt);
		doc->getXidMap().registerNode(insNode, doc->getXidMap().allocateNewXID());
	} else {
		endText = txt->splitText(pos);
		node->insertBefore(insNode, txt->getNextSibling());
		doc->getXidMap().registerNode(insNode, doc->getXidMap().allocateNewXID());
		node->insertBefore(endText, insNode->getNextSibling());
		doc->getXidMap().registerNode(endText, doc->getXidMap().allocateNewXID());
	}
	
	insText = doc->createTextNode(ins);
	insNode->appendChild(insText);
	doc->getXidMap().registerNode(insText, doc->getXidMap().allocateNewXID());
}

void XyDOMText::replace(int pos, int len, const XMLCh *repl)
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
	if (pos == 0) {
		replacedText = (DOMText *)node->removeChild((DOMNode *)txt);
		doc->getXidMap().removeNode(txt);
		nextNode = node->getFirstChild();
	} else {
		replacedText = txt->splitText(pos);
		nextNode = txt->getNextSibling();
	}
	

	DOMNode *replNode = doc->createElement(XMLString::transcode("repl"));

	node->insertBefore((DOMNode *)endText, nextNode);
	doc->getXidMap().registerNode(endText, doc->getXidMap().allocateNewXID());
	node->insertBefore(replNode, endText);

	doc->getXidMap().registerNode(replNode, doc->getXidMap().allocateNewXID());

	DOMNode *insNode = doc->createElement(XMLString::transcode("ins"));
	replNode->appendChild(insNode);
	doc->getXidMap().registerNode(insNode, doc->getXidMap().allocateNewXID());

	DOMNode *delNode = doc->createElement(XMLString::transcode("del"));
	replNode->appendChild(delNode);
	doc->getXidMap().registerNode(delNode, doc->getXidMap().allocateNewXID());
	
	DOMText *replText = doc->createTextNode(repl);
	insNode->appendChild(replText);
	doc->getXidMap().registerNode(replText, doc->getXidMap().allocateNewXID());

	delNode->appendChild(replacedText);
	doc->getXidMap().registerNode(replacedText, doc->getXidMap().allocateNewXID());
	
	if (pos != 0) {
		txt = (DOMText *) node->getFirstChild();
	}
}

XyDOMText::~XyDOMText()
{
	
}

void XyDOMText::complete()
{
	if (!applyAnnotations) {
		std::cout << currentValue << std::endl;
		txt->setNodeValue( XMLString::transcode(currentValue.c_str()) ) ;
	}
	// If a word in the previous document is replaced with another word that shares some characters,
	// we end up with a situation where there are multiple edits in a single word, which can look
	// confusing and isn't particularly helpful. Here we search for replace, insert, or delete operations
	// that surround a text node with no whitespace, and then merge the three into a single replace operation.
	DOMNodeList *childNodes = node->getChildNodes();
	for (int i = 0; i < childNodes->getLength(); i++) {
		DOMNode *node1 = childNodes->item(i);
		if (node1 == NULL) return;
		if (node1->getNodeType() == DOMNode::ELEMENT_NODE) {
			DOMNode *node2 = node1->getNextSibling();
			if (node2 == NULL) return;
			DOMNode *node3 = node2->getNextSibling();
			if (node3 == NULL) return;
			if (textNodeHasNoWhitespace((DOMText *)node2)) {
				if (node3->getNodeType() == DOMNode::ELEMENT_NODE) {
					if (mergeNodes(node1, node2, node3)) {
						// Move the increment back to retest our new node to see if it
						// can be merged in the same way with the nodes that follow it
						i--;
					}
				}
			} 
		}
	}
}

bool XyDOMText::textNodeHasNoWhitespace(DOMText *t)
{
	// Make sure we're dealing with a text node
	if (((DOMNode *)t)->getNodeType() != DOMNode::TEXT_NODE) {
		return 0;
	}
	std::string nodeText = std::string ( XMLString::transcode(t->getNodeValue()) );
	std::cout << nodeText << std::endl;
	return (nodeText.find("		\n") == std::string::npos);
}

bool XyDOMText::mergeNodes(DOMNode *node1, DOMNode *node2, DOMNode *node3)
{
	DOMNode *replNode, *insNode, *delNode, *parent;
	DOMText *insTextNode, *delTextNode;
	std::string instext, deltext;

	if ( XMLString::compareString(node1->getNodeName(), XMLString::transcode("repl")) == 0) {
		// @todo: Add check that these child nodes exist and are the proper tag names
		if (!node1->hasChildNodes()) return 0;
		instext += XMLString::transcode( node1->getChildNodes()->item(0)->getFirstChild()->getNodeValue() );
		deltext += XMLString::transcode( node1->getChildNodes()->item(1)->getFirstChild()->getNodeValue() );
	}
	else if ( XMLString::compareString(node1->getNodeName(), XMLString::transcode("del")) == 0) {
		deltext += XMLString::transcode( node1->getFirstChild()->getNodeValue() );
	}
	else if ( XMLString::compareString(node1->getNodeName(), XMLString::transcode("ins")) == 0) {
		instext += XMLString::transcode( node1->getFirstChild()->getNodeValue() );
	}
	instext += XMLString::transcode( node2->getNodeValue() );
	deltext += XMLString::transcode( node2->getNodeValue() );
	
	if ( XMLString::compareString(node3->getNodeName(), XMLString::transcode("repl")) == 0) {
		// @todo: Add check that these child nodes exist and are the proper tag names
		if (!node1->hasChildNodes()) return 0;
		instext += XMLString::transcode( node3->getChildNodes()->item(0)->getFirstChild()->getNodeValue() );
		deltext += XMLString::transcode( node3->getChildNodes()->item(1)->getFirstChild()->getNodeValue() );
	}
	else if ( XMLString::compareString(node3->getNodeName(), XMLString::transcode("del")) == 0) {
		deltext += XMLString::transcode( node3->getFirstChild()->getNodeValue() );
	}
	else if ( XMLString::compareString(node3->getNodeName(), XMLString::transcode("ins")) == 0) {
		instext += XMLString::transcode( node3->getFirstChild()->getNodeValue() );
	}


	replNode = doc->createElement(XMLString::transcode("repl"));
	parent = node1->getParentNode();
	parent->insertBefore(replNode, node1);
	doc->getXidMap().registerNode(replNode, doc->getXidMap().allocateNewXID());
	
	insNode = doc->createElement(XMLString::transcode("ins"));
	replNode->appendChild(insNode);
	doc->getXidMap().registerNode(insNode, doc->getXidMap().allocateNewXID());
	
	delNode = doc->createElement(XMLString::transcode("del"));
	replNode->appendChild(delNode);
	doc->getXidMap().registerNode(delNode, doc->getXidMap().allocateNewXID());
	
	insTextNode = doc->createTextNode( XMLString::transcode(instext.c_str()) );
	insNode->appendChild(insTextNode);
	doc->getXidMap().registerNode(insTextNode, doc->getXidMap().allocateNewXID());

	delTextNode = doc->createTextNode( XMLString::transcode(deltext.c_str()) );
	delNode->appendChild(delTextNode);
	doc->getXidMap().registerNode(delTextNode, doc->getXidMap().allocateNewXID());

	doc->getXidMap().removeNode(node1);
	doc->getXidMap().removeNode(node2);
	doc->getXidMap().removeNode(node3);
	parent->removeChild(node1);
	parent->removeChild(node2);
	parent->removeChild(node3);
	return 1;
}

std::string itoa (int n)
{	
	char * s = new char[17];
	std::string u;
	
	if (n < 0) {         //turns n positive
		n = (-1 * n); 
		u = "-";         //adds '-' on result string
	}
	
	int i = 0;  // s counter
	
	do {
		s[i++] = n % 10 + '0'; //conversion of each digit of n to char
		n -= n % 10;           //update n value
	} while ((n /= 10) > 0);
	
	for (int j = i-1; j >= 0; j--) { 
		u += s[j];    //building our string number
	}
	
	delete[] s;       //free-up the memory!
	return u;
}