#include "infra/general/Log.hpp"

#include "xercesc/util/PlatformUtils.hpp"
//#include "xercesc/dom/deprecated/DOM_NamedNodeMap.hpp"
//#include "xercesc/dom/deprecated/DOMParser.hpp"


#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/dom/DOMImplementationLS.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMBuilder.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMError.hpp"
#include "xercesc/dom/DOMErrorHandler.hpp"
#include "xercesc/dom/DOMLocator.hpp"
#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/util/XMLUniDefs.hpp"
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/sax/SAXException.hpp"
#include "xercesc/sax/SAXParseException.hpp"

#include "XyDiff/include/XyLatinStr.hpp"
#include "XyDiff/include/XID_DOMDocument.hpp"
#include "XyDiff/include/XID_map.hpp"

#include "XyDiff/DeltaException.hpp"
#include "XyDiff/DOMPrint.hpp"
#include "XyDiff/Tools.hpp"
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

static const XMLCh gLS[] = { xercesc_2_2::chLatin_L, xercesc_2_2::chLatin_S, xercesc_2_2::chNull };

//
// Opens an XML file and if possible its associated XID-mapping file
// Apply all XIDs on the Xerces Nodes and create XID_map hash_map index to help 'getNodeWithXID()'
//

bool PrintWithXID = false ;
XID_map *PrintXidMap = NULL ;

void GlobalPrintContext_t::SetModeDebugXID( XID_map &xidmap ) { PrintXidMap = &xidmap; PrintWithXID=true; }
void GlobalPrintContext_t::ReleaseContext( void )             { PrintWithXID=false; } ;

class GlobalPrintContext_t globalPrintContext ;

// Create new XID_DOMDocument

XID_DOMDocument* XID_DOMDocument::createDocument(void) {
  xercesc_2_2::DOMImplementation* impl =  xercesc_2_2::DOMImplementationRegistry::getDOMImplementation(gLS);
  xercesc_2_2::DOMDocument* doc = impl->createDocument();
  return new XID_DOMDocument(doc, NULL, true);
}

xercesc_2_2::DOMDocument * XID_DOMDocument::getDOMDocumentOwnership() {
	xercesc_2_2::DOMDocument * doc = theDocument;
	theDocument = NULL;
	return doc;
}

/* Given an XML File, construct an XID_DOMDocument
 * There are 3 possibilities concerning XIDs for the Document:
 *
 * (1) The XID-file exists, it is used
 * (2) There is no XID-file, default XIDs (1-n|n+1) are assigned
 * (3) There is no XID-file, no XIDs are assigned for the moment
 *
 */

XID_map &XID_DOMDocument::getXidMap(void) {
	return *xidmap;
	}

void XID_DOMDocument::addXidMapFile(const char *xidmapFilename) {
	if (xidmapFilename==NULL) THROW_AWAY(("empty file name"));
	std::fstream XIDmap_file(xidmapFilename, std::fstream::in);
	if (!XIDmap_file) {
		printf("Warning: no .xidmap description for XML file <%s>, using defaults Xids (1-n|n+1)\n",xidmapFilename);
		char xidstring[100];
		int nodecount = addXidMap(NULL)-1;
		if (nodecount>1) sprintf(xidstring, "(1-%d|%d)", nodecount, nodecount+1 );
		else sprintf(xidstring, "(1|2)");
		std::string mapString = xidstring ;
		XIDmap_file.close();
		std::fstream writeXidmapFile(xidmapFilename, std::fstream::out);
		if (!writeXidmapFile) {
			fprintf(stderr, "Could not attach xid description to file - write access to %s denied\n",xidmapFilename);
			THROW_AWAY(("Can not write to Xidmap file"));
			}
		else {
			writeXidmapFile << mapString << endl ;
			printf("XIDmap attached to xml file (.xidmap file created)\n");
			}
		}
	else {
		std::string mapstr;
		XIDmap_file >> mapstr ;
		addXidMap(mapstr.c_str());
		}
	return;	
	}

int XID_DOMDocument::addXidMap(const char *theXidmap) {
	if (xidmap!=NULL) THROW_AWAY(("can't attach XidMap, slot not empty"));
	TRACE("getDocumentElement()");
	xercesc_2_2::DOMElement* docRoot = getDocumentElement();
	if (theXidmap==NULL) {
		int nodecount = getDocumentNodeCount() ;
		char xidstring[100];
		if (nodecount>1) sprintf(xidstring, "(1-%d|%d)", nodecount, nodecount+1 );
		else sprintf(xidstring, "(1|2)");
		TRACE("define default XidMap : " <<xidstring);
		xidmap = XID_map::addReference( new XID_map( xidstring, docRoot ) );
		return (nodecount+1);
		}
	xidmap = XID_map::addReference(new XID_map(theXidmap, docRoot));
	return -1;
	}

void XID_DOMDocument::initEmptyXidmapStartingAt(XID_t firstAvailableXid) {
	if (xidmap!=NULL) THROW_AWAY(("can't attach XidMap, slot not empty"));
	xidmap = XID_map::addReference( new XID_map() );
	if (firstAvailableXid>0) xidmap->initFirstAvailableXID(firstAvailableXid);
	}

// Construct XID_DOMDocument from a DOM_Document

XID_DOMDocument::XID_DOMDocument(xercesc_2_2::DOMDocument *doc, const char *xidmapStr, bool adoptDocument) : xidmap(NULL), theDocument(doc), theParser(NULL), doReleaseTheDocument(adoptDocument)
{
	if (xidmapStr) {
		(void)addXidMap(xidmapStr);
	}
}

XID_DOMDocument::XID_DOMDocument(const char* xmlfile, bool useXidMap, bool doValidation) : xidmap(NULL), theDocument(NULL), theParser(NULL), doReleaseTheDocument(false) {
	parseDOM_Document(xmlfile, doValidation);
	
	xidmap=NULL;
	if (xmlfile==NULL) THROW_AWAY(("bad argument for xmlfile"));
	std::string xidmap_filename = std::string( xmlfile )+std::string(".xidmap") ;
  
	if (!useXidMap) {
		xidmap = XID_map::addReference( new XID_map() );
		}
	else {
		addXidMapFile(xidmap_filename.c_str());
		}
		
	}

//
// Save the XID_DOMDocument by saving the document in one file, and the XIDString in another
//

void XID_DOMDocument::SaveAs(const char *xml_filename, bool saveXidMap) {
	
	// Saves XML file
	
	std::ofstream xmlfile(xml_filename);
	if (!xmlfile) {
		cerr << "Error: could not open <" << xmlfile << "> for output" << endl ;
		return ;
		}

	if (saveXidMap) {
		if (xidmap==NULL) throw VersionManagerException("Runtime Error", "XID_DOMDocument::SaveAs", "Cannot save XidMap as there is no XidMap");
		
		// Saves XID mapping
		std::string xidmap_filename = xml_filename ;
		xidmap_filename += ".xidmap" ;
	
		std::ofstream xidmapfile(xidmap_filename.c_str());
		if (!xidmapfile) {
			cerr << "Error: could not open <" << xidmap_filename << "> for output" << endl ;
			return ;
			}
		
		std::string xidString = xidmap->String() ;
		cout << "Version: SaveAs/xidmap->String()=" << xidString << endl ;	
		xidmapfile << xidString ;
		}
	xmlfile << *this ;

	}
	
//
// Create a copy of a document
//

XID_DOMDocument* XID_DOMDocument::copy(const XID_DOMDocument *doc, bool withXID) {
	std::cout << "XID_DOMDocument::copy" << std::endl;
	XID_DOMDocument* result = XID_DOMDocument::createDocument() ;
	std::cout << "create document ok" << std::endl;
	xercesc_2_2::DOMNode* resultRoot = result->importNode((xercesc_2_2::DOMNode*)doc->getDocumentElement(), true);
	std::cout << "node import ok" << std::endl;
	result->appendChild(resultRoot);
	std::cout << "append child ok" << std::endl;

	if (withXID) {
		if ( doc->xidmap==NULL ) throw VersionManagerException("Program Error", "XID_DOMDocument::copy()", "Option 'withXID' used but source has NULL xidmap");
		//result.xidmap = XID_map::addReference( new XID_map( resultRoot, doc.xidmap->getFirstAvailableXID() ) );
		result->xidmap = XID_map::addReference(new XID_map(doc->xidmap->String().c_str(), resultRoot));
		std::cout << "copy xidmap ok" << std::endl;
	}
	return (result) ;	
}

XID_DOMDocument::~XID_DOMDocument() {
	this->release();
}

//---------------------------------------------------------------------------
//  Initialisation of document from File
//---------------------------------------------------------------------------

/************************************************************************************************************************************
 ***                                                    ***
 ***                PARSING XML DOCUMENT                ***
 ***                                                    ***
 ************************************************************************************************************************************/

class xydeltaParseHandler : public xercesc_2_2::DOMErrorHandler {
public:
  void warning(const xercesc_2_2::SAXParseException& e);
  void error(const xercesc_2_2::SAXParseException& e);
  void fatalError(const xercesc_2_2::SAXParseException& e);
  void resetErrors() {};
  bool handleError(const xercesc_2_2::DOMError& domError);
} ;

bool xydeltaParseHandler::handleError(const xercesc_2_2::DOMError& domError)
{
  xercesc_2_2::DOMLocator* locator = domError.getLocation();
  cerr << "\n(GF) Error at (file " << XyLatinStr(locator->getURI()).localForm()
       << ", line " << locator->getLineNumber()
       << ", char " << locator->getColumnNumber()
       << "): " << XyLatinStr(domError.getMessage()).localForm() << endl;
  throw VersionManagerException("xydeltaParseHandler", "error", "-");
}

void xydeltaParseHandler::error(const xercesc_2_2::SAXParseException& e) {
	cerr << "\n(GF) Error at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << e.getLineNumber()
	     << ", char " << e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	throw VersionManagerException("xydeltaParseHandler", "error", "-");
	}
void xydeltaParseHandler::fatalError(const xercesc_2_2::SAXParseException& e) {
	cerr << "\n(GF) Fatal Error at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << e.getLineNumber()
	     << ", char " << e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	throw VersionManagerException("xydeltaParseHandler", "fatal error", "-");
	}
void xydeltaParseHandler::warning(const xercesc_2_2::SAXParseException& e) {
	cerr << "\n(GF) Warning at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << e.getLineNumber()
	     << ", char " << e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	}
  
void XID_DOMDocument::parseDOM_Document(const char* xmlfile, bool doValidation) {
	if (theDocument || theParser) {
		cerr << "current doc is not initialized, can not parse" << endl;
		throw VersionManagerException("XML Exception", "parseDOM_Document", "current doc is not initialized");
	}
	
	// Initialize the XML4C2 system
	try {
		xercesc_2_2::XMLPlatformUtils::Initialize();
	}
	catch(const xercesc_2_2::XMLException& e) {
		cerr << "Error during Xerces-c Initialization.\n"
		     << "  Exception message:" << XyLatinStr(e.getMessage()).localForm() << endl;
		ERROR("Xerces::Initialize() FAILED");
		throw VersionManagerException("XML Exception", "parseDOM_Document", "Xerces-C++ Initialization failed");
	}

	//  Create our validator, then attach an error handler to the parser.
	//  The parser will call back to methods of the ErrorHandler if it
	//  discovers errors during the course of parsing the XML document.
	//  Then parse the XML file, catching any XML exceptions that might propogate out of it.
   
        // The parser owns the Validator, so we don't have to free it
  	// The parser also owns the DOMDocument
	
	static xercesc_2_2::DOMImplementation *impl = xercesc_2_2::DOMImplementationRegistry::getDOMImplementation(gLS);
	theParser = ((xercesc_2_2::DOMImplementationLS*)impl)->createDOMBuilder(xercesc_2_2::DOMImplementationLS::MODE_SYNCHRONOUS, 0);
  
	bool errorsOccured = false;
  
	try {
		theParser->setFeature(xercesc_2_2::XMLUni::fgDOMValidation, doValidation);
		//theParser->setDoValidation(doValidation);
		xercesc_2_2::DOMErrorHandler * handler = new xydeltaParseHandler();
		theParser->setErrorHandler(handler);
                theDocument = theParser->parseURI(xmlfile);
        } catch (const xercesc_2_2::XMLException& e) {
		cerr << "XMLException: An error occured during parsing\n   Message: "
		     << XyLatinStr(e.getMessage()).localForm() << endl;
		errorsOccured = true;
		throw VersionManagerException("XML Exception", "parseDOM_Document", "See previous exception messages for more details");
	}
}
 
//---------------------------------------------------------------------------
// get Document(Subtree)NodeCount
//---------------------------------------------------------------------------

int XID_DOMDocument::getSubtreeNodeCount(xercesc_2_2::DOMNode *node) {
	int count = 0;
	if (node==NULL) {
		FATAL("unexpected NULL node");
		abort();
	}
        if(node->hasChildNodes()) {
		xercesc_2_2::DOMNode* child = node->getFirstChild();
		while(child!=NULL) {
			count += getSubtreeNodeCount(child);
			child=child->getNextSibling();
			}
		}
	count++;
	return count;
	}

int XID_DOMDocument::getDocumentNodeCount() {
	xercesc_2_2::DOMElement* docRoot = this->getDocumentElement() ;
	if (docRoot==NULL) {
		ERROR("document has no Root Element");
		return 0;
	}
	return getSubtreeNodeCount( docRoot );
}

bool XID_DOMDocument::isRealData(xercesc_2_2::DOMNode *node) {
	switch(node->getNodeType()) {
		
/*
 * this part needs to be optimized as it uses XyLatinStr (XML transcode)
 */ 

		case xercesc_2_2::DOMNode::TEXT_NODE: {
			XyLatinStr theText( node->getNodeValue() );
			unsigned int l = strlen(theText.localForm()) ;
			for(unsigned int i=0;i<l;i++) {
				if (theText.localForm()[i]>32) return true;
				}
			return false;
			}
			
		default:
			return true;
		}
	}


void Restricted::XidTagSubtree(XID_DOMDocument *doc, xercesc_2_2::DOMNode* node) {
	
	// Tag Node
	
	XID_t myXid = doc->getXidMap().getXIDbyNode(node);
	if (node->getNodeType()==xercesc_2_2::DOMNode::ELEMENT_NODE) {
		char xidStr[20];
		sprintf(xidStr, "%d", (int)myXid);
		xercesc_2_2::DOMNamedNodeMap* attr = node->getAttributes();
		if (attr==NULL) throw VersionManagerException("Internal Error", "XidTagSubtree()", "Element node getAttributes() returns NULL");
		xercesc_2_2::DOMAttr* attrNode = doc->createAttribute(L"XyXid");
		attrNode->setValue(XyLatinStr(xidStr).wideForm());
		attr->setNamedItem(attrNode);
		}
	
	
	// Apply Recursively
	
	if (node->hasChildNodes()) {
          xercesc_2_2::DOMNode* child = node->getFirstChild();
		while(child!=NULL) {
			XidTagSubtree(doc, child);
			child=child->getNextSibling();
			}
		}
	}

const XMLCh * XID_DOMDocument::getNodeName() const
{
  return theDocument->getNodeName();
}

const XMLCh * XID_DOMDocument::getNodeValue() const
{
  return theDocument->getNodeValue();
}

short int XID_DOMDocument::getNodeType() const
{
  return theDocument->getNodeType();
}

xercesc_2_2::DOMNode * XID_DOMDocument::getParentNode() const
{
  return theDocument->getParentNode();
}

xercesc_2_2::DOMNodeList * XID_DOMDocument::getChildNodes() const
{
  return theDocument->getChildNodes();
}

xercesc_2_2::DOMNode * XID_DOMDocument::getFirstChild() const
{
  return theDocument->getFirstChild();
}

xercesc_2_2::DOMNode * XID_DOMDocument::getLastChild() const
{
  return theDocument->getLastChild();
}

xercesc_2_2::DOMNode * XID_DOMDocument::getPreviousSibling() const
{
  return theDocument->getPreviousSibling();
}

xercesc_2_2::DOMNode * XID_DOMDocument::getNextSibling() const
{
  return theDocument->getNextSibling();
}

xercesc_2_2::DOMNamedNodeMap * XID_DOMDocument::getAttributes() const
{
  return theDocument->getAttributes();
}

xercesc_2_2::DOMDocument * XID_DOMDocument::getOwnerDocument() const
{
  return theDocument->getOwnerDocument();
}

xercesc_2_2::DOMNode * XID_DOMDocument::cloneNode(bool deep) const
{
  return theDocument->cloneNode(deep);
}

xercesc_2_2::DOMNode * XID_DOMDocument::insertBefore(xercesc_2_2::DOMNode * node1, xercesc_2_2::DOMNode * node2)
{
  return theDocument->insertBefore(node1, node2);
}

xercesc_2_2::DOMNode * XID_DOMDocument::replaceChild(xercesc_2_2::DOMNode * node1, xercesc_2_2::DOMNode *node2)
{
  return theDocument->replaceChild(node1, node2);
}

xercesc_2_2::DOMNode * XID_DOMDocument::removeChild(xercesc_2_2::DOMNode *node1)
{
  return theDocument->removeChild(node1);
}

xercesc_2_2::DOMNode * XID_DOMDocument::appendChild(xercesc_2_2::DOMNode *node1)
{
  return theDocument->appendChild(node1);
}

bool XID_DOMDocument::hasChildNodes() const
{
  return theDocument->hasChildNodes();
}

void XID_DOMDocument::setNodeValue(const XMLCh *nodeValue)
{
  return theDocument->setNodeValue(nodeValue);
}

void XID_DOMDocument::normalize()
{
  return theDocument->normalize();
}

bool XID_DOMDocument::isSupported(const XMLCh *feature, const XMLCh *version) const
{
  return theDocument->isSupported(feature, version);
}

const XMLCh * XID_DOMDocument::getNamespaceURI() const
{
  return theDocument->getNamespaceURI();
}

const XMLCh * XID_DOMDocument::getPrefix() const
{
  return theDocument->getPrefix();
}

const XMLCh * XID_DOMDocument::getLocalName() const
{
  return theDocument->getLocalName();
}

void XID_DOMDocument::setPrefix(const XMLCh *prefix)
{
  return theDocument->setPrefix(prefix);
}

bool XID_DOMDocument::hasAttributes() const
{
  return theDocument->hasAttributes();
}

bool XID_DOMDocument::isSameNode(const xercesc_2_2::DOMNode *node) const
{
  return theDocument->isSameNode(node);
}

bool XID_DOMDocument::isEqualNode(const xercesc_2_2::DOMNode *node) const
{
  return theDocument->isEqualNode(node);
}

void * XID_DOMDocument::setUserData(const XMLCh *key, void *data, xercesc_2_2::DOMUserDataHandler *handler)
{
  return theDocument->setUserData(key, data, handler);
}

void * XID_DOMDocument::getUserData(const XMLCh *key) const
{
  return theDocument->getUserData(key);
}

const XMLCh * XID_DOMDocument::getBaseURI() const
{
  return theDocument->getBaseURI();
}

short int XID_DOMDocument::compareTreePosition(const xercesc_2_2::DOMNode *node) const
{
  return theDocument->compareTreePosition(node);
}

const XMLCh * XID_DOMDocument::getTextContent() const
{
  return theDocument->getTextContent();
}

void XID_DOMDocument::setTextContent(const XMLCh *textContent)
{
  return theDocument->setTextContent(textContent);
}

const XMLCh * XID_DOMDocument::lookupNamespacePrefix(const XMLCh *namespaceURI, bool useDefault) const
{
  return theDocument->lookupNamespacePrefix(namespaceURI, useDefault);
}

bool XID_DOMDocument::isDefaultNamespace(const XMLCh *namespaceURI) const
{
  return theDocument->isDefaultNamespace(namespaceURI);
}

const XMLCh * XID_DOMDocument::lookupNamespaceURI(const XMLCh *prefix) const
{
  return theDocument->lookupNamespaceURI(prefix);
}

xercesc_2_2::DOMNode * XID_DOMDocument::getInterface(const XMLCh *feature)
{
  return theDocument->getInterface(feature);
}

void XID_DOMDocument::release() {
	if (theParser) {
		NOTE("delete parser owning the document");
		delete theParser;
		theParser=NULL;
		theDocument=NULL;
	} else if (doReleaseTheDocument) {
		if (theDocument) {
			NOTE("delete the document");
			delete theDocument;
			theDocument=NULL;
		}
	}
	theDocument=NULL;		
	if (xidmap) {
		NOTE("delete the xidmap");
		XID_map::removeReference(xidmap);
	}
}

xercesc_2_2::DOMRange * XID_DOMDocument::createRange()
{
  return theDocument->createRange();
}

xercesc_2_2::DOMElement * XID_DOMDocument::createElement(const XMLCh *tagName)
{
  return theDocument->createElement(tagName);
}

xercesc_2_2::DOMDocumentFragment * XID_DOMDocument::createDocumentFragment()
{
  return theDocument->createDocumentFragment();
}

xercesc_2_2::DOMText * XID_DOMDocument::createTextNode(const XMLCh *data)
{
  return theDocument->createTextNode(data);
}

xercesc_2_2::DOMComment * XID_DOMDocument::createComment(const XMLCh *data)
{
  return theDocument->createComment(data);
}

xercesc_2_2::DOMCDATASection * XID_DOMDocument::createCDATASection(const XMLCh *data)
{
  return theDocument->createCDATASection(data);
}

xercesc_2_2::DOMProcessingInstruction * XID_DOMDocument::createProcessingInstruction(const XMLCh *target, const XMLCh *data)
{
  return theDocument->createProcessingInstruction(target, data);
}

xercesc_2_2::DOMAttr * XID_DOMDocument::createAttribute(const XMLCh *name)
{
  return theDocument->createAttribute(name);
}

xercesc_2_2::DOMEntityReference * XID_DOMDocument::createEntityReference(const XMLCh *name)
{
  return theDocument->createEntityReference(name);
}

xercesc_2_2::DOMDocumentType * XID_DOMDocument::getDoctype() const
{
  return theDocument->getDoctype();
}

xercesc_2_2::DOMImplementation *XID_DOMDocument::getImplementation() const
{
  return theDocument->getImplementation();
}

xercesc_2_2::DOMElement * XID_DOMDocument::getDocumentElement() const
{
  return theDocument->getDocumentElement();
}

xercesc_2_2::DOMNodeList * XID_DOMDocument::getElementsByTagName(const XMLCh *tagName) const
{
  return theDocument->getElementsByTagName(tagName);
}

xercesc_2_2::DOMNode * XID_DOMDocument::importNode(xercesc_2_2::DOMNode *importNode, bool deep)
{
  return theDocument->importNode(importNode, deep);
}

xercesc_2_2::DOMElement * XID_DOMDocument::createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName)
{
  return theDocument->createElementNS(namespaceURI, qualifiedName);
}

xercesc_2_2::DOMAttr * XID_DOMDocument::createAttributeNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName)
{
  return theDocument->createAttributeNS(namespaceURI, qualifiedName);
}

xercesc_2_2::DOMNodeList * XID_DOMDocument::getElementsByTagNameNS(const XMLCh *namespaceURI, const XMLCh *localName) const
{
  return theDocument->getElementsByTagNameNS(namespaceURI, localName);
}

xercesc_2_2::DOMElement * XID_DOMDocument::getElementById(const XMLCh *elementId) const
{
  return theDocument->getElementById(elementId);
}

const XMLCh * XID_DOMDocument::getActualEncoding() const
{
  return theDocument->getActualEncoding();
}

void XID_DOMDocument::setActualEncoding(const XMLCh *actualEncoding)
{
  return theDocument->setActualEncoding(actualEncoding);
}

const XMLCh * XID_DOMDocument::getEncoding() const
{
  return theDocument->getEncoding();
}

void XID_DOMDocument::setEncoding(const XMLCh *encoding)
{
  return theDocument->setEncoding(encoding);
}

bool XID_DOMDocument::getStandalone() const
{
  return theDocument->getStandalone();
}

void XID_DOMDocument::setStandalone(bool standalone)
{
  return theDocument->setStandalone(standalone);
}

const XMLCh * XID_DOMDocument::getVersion() const
{
  return theDocument->getVersion();
}

void XID_DOMDocument::setVersion(const XMLCh *version)
{
  return theDocument->setVersion(version);
}

const XMLCh * XID_DOMDocument::getDocumentURI() const
{
  return theDocument->getDocumentURI();
}

void XID_DOMDocument::setDocumentURI(const XMLCh *documentURI)
{
  return theDocument->setDocumentURI(documentURI);
}





bool XID_DOMDocument::getStrictErrorChecking() const
{
  return theDocument->getStrictErrorChecking();
}

void XID_DOMDocument::setStrictErrorChecking(bool strictErrorChecking)
{
  return theDocument->setStrictErrorChecking(strictErrorChecking);
}


xercesc_2_2::DOMErrorHandler * XID_DOMDocument::getErrorHandler() const
{
  return theDocument->getErrorHandler();
}


void XID_DOMDocument::setErrorHandler(xercesc_2_2::DOMErrorHandler *handler)
{
  return theDocument->setErrorHandler(handler);
}


xercesc_2_2::DOMNode * XID_DOMDocument::renameNode(xercesc_2_2::DOMNode *n, const XMLCh *namespaceURI, const XMLCh *name)
{
  return theDocument->renameNode(n, namespaceURI, name);
}


xercesc_2_2::DOMNode * XID_DOMDocument::adoptNode(xercesc_2_2::DOMNode *node)
{
  return theDocument->adoptNode(node);
}


void XID_DOMDocument::normalizeDocument()
{
  return theDocument->normalizeDocument();
}


bool XID_DOMDocument::canSetNormalizationFeature(const XMLCh *name, bool state) const
{
  return theDocument->canSetNormalizationFeature(name, state);
}


void XID_DOMDocument::setNormalizationFeature(const XMLCh *name, bool state)
{
  return theDocument->setNormalizationFeature(name, state);
}


bool XID_DOMDocument::getNormalizationFeature(const XMLCh *name) const
{
  return theDocument->getNormalizationFeature(name);
}


xercesc_2_2::DOMEntity * XID_DOMDocument::createEntity(const XMLCh *name)
{
  return theDocument->createEntity(name);
}


xercesc_2_2::DOMDocumentType * XID_DOMDocument::createDocumentType(const XMLCh *name)
{
  return theDocument->createDocumentType(name);
}


xercesc_2_2::DOMNotation * XID_DOMDocument::createNotation(const XMLCh *name)
{
  return theDocument->createNotation(name);
}


xercesc_2_2::DOMElement * XID_DOMDocument::createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName, const XMLSSize_t lineNum, const XMLSSize_t columnNum)
{

  return theDocument->createElementNS(namespaceURI, qualifiedName, lineNum, columnNum);
}

xercesc_2_2::DOMTreeWalker * XID_DOMDocument::createTreeWalker(xercesc_2_2::DOMNode *root, long unsigned int whatToShow, xercesc_2_2::DOMNodeFilter *filter, bool entityReferenceExpansion){
  return theDocument->createTreeWalker(root, whatToShow, filter, entityReferenceExpansion);
}


xercesc_2_2::DOMNodeIterator * XID_DOMDocument::createNodeIterator(xercesc_2_2::DOMNode *root, long unsigned int whatToShow, xercesc_2_2::DOMNodeFilter *filter, bool entityReferenceExpansion)
{
  return theDocument->createNodeIterator(root, whatToShow, filter, entityReferenceExpansion);
}
